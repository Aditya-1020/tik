#include "parser.h"
#include "orders.h"
#include "ordermanager.h"
#include "receive.h"
#include <iostream>
#include <chrono>
#include <random>
#include <sstream>
#include <iomanip>

void MessageRouter::parseMarketData(const std::string_view &message, OrderBook &book){
    if (message.size() >= 5 && 
        message[0] == '8' &&
        message[1] == '=' &&
        message[2] == 'F' &&
        message[3] == 'I' &&
        message[4] == 'X') {
            FIXParser::parseMarketData(message, book);
    }
}

void FIXParser::parseMarketData(const std::string_view &message, OrderBook &book){
    State current_state = State::INITIAL_STATE;
    std::string current_tag = "";
    std::string current_value = "";
    std::string symbol = ""; // We need to extract the symbol to pass to the book
    MDContext md_context;

    for (char c : message) {
        current_state = processChar(c, current_state, current_tag, current_value, md_context, book, symbol);
    
        if (current_state == State::ERROR_STATE) {
            return;
        }
    }
}

FIXParser::State FIXParser::processChar(char c, State current_state, std::string &current_tag, std::string &current_value, MDContext &md_context, OrderBook& book, std::string& symbol) {
    switch (current_state) {
        case State::READING_VALUE:
            if (c == '\x01') {
                processField(current_tag, current_value, md_context, book, symbol);
                State next_state = determineNextState(current_tag, md_context);
                current_tag.clear();
                current_value.clear();
                return next_state;
            } else {
                current_value += c;
                return State::READING_VALUE;
            }
        case State::INITIAL_STATE:
            if (std::isdigit(c)) { current_tag += c; return State::READING_TAG; }
            return State::ERROR_STATE;
        case State::READING_TAG:
            if (std::isdigit(c)) { current_tag += c; return State::READING_TAG; } 
            else if (c == '=') { return State::READING_VALUE; }
            return State::ERROR_STATE;
        case State::MD_ENTRY_TYPE_FOUND:
            if (std::isdigit(c)) { current_tag += c; return State::READING_TAG; }
            return State::ERROR_STATE;
        case State::EXPECTING_QUANTITY:
            if (std::isdigit(c)) { current_tag += c; return State::READING_TAG; }
            return State::ERROR_STATE;
        case State::ERROR_STATE: return State::ERROR_STATE;
        case State::COMPLETE: return State::COMPLETE;
        default: return State::ERROR_STATE;
    }
}

void FIXParser::processField(std::string_view tag, std::string_view value, MDContext &md_context, OrderBook &book, std::string &symbol) {
    int tag_int = std::stoi(std::string(tag));
    switch (tag_int) {
        case 55: // Symbol
            symbol = value;
            break;
        case 269: // MD Entry Type
            md_context.entry_type = value;
            md_context.has_price = false;
            md_context.has_quantity = false;
            break;
        case 270: // Price
            md_context.price = std::stod(std::string(value));
            md_context.has_price = true;
            if (md_context.has_quantity) { commitMDEntry(md_context, book, symbol); }
            break;
        case 271: // Quantity
            md_context.quantity = std::stoi(std::string(value));
            md_context.has_quantity = true;
            if (md_context.has_price) { commitMDEntry(md_context, book, symbol); }
            break;
        // TODO: Other tags 35, 52
    }
}

void FIXParser::commitMDEntry(MDContext &md_context, OrderBook &book, const std::string_view &symbol) {
    if (symbol.empty()) {
        return;
    }

    if (md_context.entry_type == "0") {
        book.updateBid(md_context.price, md_context.quantity);
    } else if (md_context.entry_type == "1") { // Ask
        book.updateAsk(md_context.price, md_context.quantity);
    }

    // Reset context for next entry snapshot
    md_context.reset();
}

FIXParser::State FIXParser::determineNextState(const std::string_view &tag, const MDContext &md_context){
    (void)md_context;
    int tag_int = std::stoi(std::string(tag));
    switch (tag_int){
        case 269: return State::MD_ENTRY_TYPE_FOUND;
        case 270: return State::EXPECTING_QUANTITY;
        case 271: return State::READING_TAG;
        case 10: return State::COMPLETE;
        default: return State::READING_TAG;
    }
}

Market_data SBEParser::parseMarketData(const std::vector<uint8_t> &buffer){
    (void)buffer;
    return Market_data{};
}

// 8=FIX.4.2\x || 019=123\x01 || 35=W\x01 || 55=EUR/USD\x01 || 268=2\x01269=0\x01 || 270=1.1234\x01 || 271=100000\x01 || 269=1\x01 || 
// 270=1.1236\x01 || 271=120000\x01 || 10=168\x01

std::string FIXParser::serializeOrder(const TradeOrder &order, Data_receiver &receiver, std::string_view sender_id, std::string_view target_id) {
    (void)sender_id;
    (void)target_id;
    
    std::ostringstream fix_message;

    // message body first
    std::ostringstream body;
    body << "35=D\x01";  // New Order Single
    body << "49=TRADER\x01";
    body << "56=EXCHANGE\x01";
    body << "52=" << getTimestamp() << "\x01";
    body << "11=" << generateOrderID() << "\x01";
    body << "55=" << order.symbol << "\x01";
    body << "54=" << ((order.side == TradeOrder::Side::BUY) ? "1" : "2") << "\x01";
    body << "38=" << order.quantity << "\x01";
    body << "40=2\x01";  // Limit order
    body << "44=" << std::fixed << std::setprecision(5) << order.price << "\x01";
    body << "59=0\x01";  // Day order

    std::string body_str = body.str();
    
    // complete message
    fix_message << "8=FIX.4.2\x01";
    fix_message << "9=" << body_str.length() << "\x01";
    fix_message << body_str;

    std::string message_without_checksum = fix_message.str();
    int checksum = calculateChecksum(message_without_checksum);
    
    fix_message << "10=" << std::setfill('0') << std::setw(3) << checksum << "\x01";

    std::string send_order_message = fix_message.str();
    receiver.sendMarketData(send_order_message);

    return send_order_message;
}

std::string FIXParser::generateOrderID() {
    auto now = std::chrono::system_clock::now();
    auto timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();

    static std::random_device rd;
    static std::mt19937 gen(rd()); // random number
    static std::uniform_int_distribution<int> dis(1000, 9999);
    
    return "ORD" + std::to_string(timestamp) + std::to_string(dis(gen));
}

int FIXParser::calculateChecksum(const std::string_view &message) {
    int sum = 0;
    for (char c : message) {
        sum += static_cast<unsigned char>(c);
    }
    return sum % CHECKSUM_MODULO;
}

std::string FIXParser::getTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;
    
    std::ostringstream oss;
    oss << std::put_time(std::gmtime(&time_t), "%Y%m%d-%H:%M:%S");
    oss << "." << std::setfill('0') << std::setw(3) << ms.count();
    
    return oss.str();
}