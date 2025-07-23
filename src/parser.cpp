#include "parser.h"
#include "orders.h"
#include "ordermanager.h"
#include "receive.h"
#include <iostream>
#include <chrono>
#include <random>
#include <sstream>
#include <iomanip>
#include <charconv>
#include <fast_float/fast_float.h>

constexpr char SOH = '\x01';
constexpr char EQUALS = '=';

inline bool isFIXMessage(const std::string_view &message){
    return message.size() >= 5 && std::memcmp(message.data(), "8=FIX", 5) == 0;
}

void MessageRouter::parseMarketData(const std::string_view &message, OrderBook &book){
    if (isFIXMessage(message)) {
        FIXParser::parseMarketData(message, book);
    }
}

void FIXParser::parseMarketData(const std::string_view &message, OrderBook &book){
    // Optimized single-pass parser
    MDContext md_context;
    
    const char* start = message.data();
    const char* end = message.data() + message.size();
    const char* current = start;
    
    // skip FIX header
    while (current < end && *current != SOH) ++current;
    if (current < end) ++current; // Skip first SOH
    
    while (current < end) {
        const char* tag_start = current; // Fast tag parsing
        while (current < end && *current != EQUALS) ++current;
        if (current >= end) break;
        
        int tag = 0;
        std::from_chars(tag_start, current, tag);
        ++current;
        
        const char* value_start = current;
        while (current < end && *current != SOH) ++current;

        std::string_view value(value_start, current - value_start);
        
        // Process only relevant tags
        switch (tag) {
            case 269: // MD Entry Type
                md_context.entry_type = value;
                md_context.has_price = false;
                md_context.has_quantity = false;
                break;
            case 270: { // Price
                fast_float::from_chars(value.data(), value.data() + value.size(), md_context.price);
                md_context.has_price = true;
                if (md_context.has_quantity) {
                    commitMDEntry(md_context, book);
                }
                break;
            }
            case 271: { // Quantity
                std::from_chars(value.data(), value.data() + value.size(), md_context.quantity);
                md_context.has_quantity = true;
                if (md_context.has_price) {
                    commitMDEntry(md_context, book);
                }
                break;
            }
        }
        if (current < end) ++current; // Skip SOH
    }
}

void FIXParser::commitMDEntry(MDContext &md_context, OrderBook &book) {
    if (md_context.entry_type[0] == '0') {  // Bid - compare first char only
        book.updateBid(md_context.price, md_context.quantity);
    } else if (md_context.entry_type[0] == '1') { // Ask
        book.updateAsk(md_context.price, md_context.quantity);
    }
    md_context.reset();
}

FIXParser::State FIXParser::processChar(char c, State current_state, std::string &current_tag, std::string &current_value, MDContext &md_context, OrderBook& book, std::string& symbol) {
    switch (current_state) {
        case State::READING_VALUE:
            if (c == SOH) {
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
            else if (c == EQUALS) { return State::READING_VALUE; }
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
    int tag_int{0};
    auto [tag_ptr, tag_ec] = std::from_chars(tag.data(), tag.data() + tag.size(), tag_int);
    if (tag_ec != std::errc()) return;
    
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
            fast_float::from_chars(value.data(), value.data() + value.size(), md_context.price);
            md_context.has_price = true;
            if (md_context.has_quantity) { commitMDEntry(md_context, book); }
            break;
        case 271: // Quantity
            std::from_chars(value.data(), value.data() + value.size(), md_context.quantity);
            md_context.has_quantity = true;
            if (md_context.has_price) { commitMDEntry(md_context, book); }
            break;
    }
}

FIXParser::State FIXParser::determineNextState(const std::string_view &tag, const MDContext &md_context){
    (void)md_context;
    int tag_int{0};
    std::from_chars(tag.data(), tag.data() + tag.size(), tag_int);
    
    switch (tag_int){
        case 269: return State::MD_ENTRY_TYPE_FOUND;
        case 270: return State::EXPECTING_QUANTITY;
        case 271: return State::READING_TAG;
        case 10: return State::COMPLETE;
        default: return State::READING_TAG;
    }
}

// Market_data SBEParser::parseMarketData(const std::vector<uint8_t> &buffer){
//     (void)buffer;
//     return Market_data{};
// }

std::string FIXParser::serializeOrder(const TradeOrder &order, Data_receiver &receiver, std::string_view sender_id, std::string_view target_id) {
    (void)sender_id;
    (void)target_id;
    
    std::string fix_message;
    fix_message.reserve(EST_FIX_MESSAGE_SIZE);

    std::string timestamp = getTimestamp();
    std::string order_id  = generateOrderID();


    // message body first
    std::string body = "35=D\x01" "49=TRADER\x01" "56=EXCHANGE\x01" "52=" + timestamp + 
                  "\x01" "11=" + order_id + "\x01" "55=" + order.symbol + 
                  "\x01" "54=" + (order.side == TradeOrder::Side::BUY ? "1" : "2") + 
                  "\x01" "38=" + std::to_string(order.quantity) + 
                  "\x01" "40=2\x01" "44=" + formatPrice(order.price) + "\x01" "59=0\x01";

    fix_message += "9=" + std::to_string(body.length()) + "\x01";
    fix_message += body;

    int checksum = calculateChecksum(fix_message);
    fix_message += "10=";
    if (checksum < 100) fix_message += "0";
    if (checksum < 10) fix_message += "0";
    fix_message += std::to_string(checksum) + "\x01";

    receiver.sendMarketData(fix_message);

    return fix_message;
}

std::string FIXParser::formatPrice(double price) {
    char buffer[32];
    int len = snprintf(buffer, sizeof(buffer), "%.5f", price);
    return std::string(buffer, len);
}

std::string FIXParser::generateOrderID() {
    static std::atomic<uint64_t> counter{0};
    static thread_local std::random_device rd;
    static thread_local std::mt19937 gen(rd());
    static thread_local std::uniform_int_distribution<int> dis(1000, 9999);
    
    return "ORD" + std::to_string(counter.fetch_add(1)) + std::to_string(dis(gen));
}

int FIXParser::calculateChecksum(const std::string_view &message) {
    int sum = 0;
    for (unsigned char c : message) {
        sum += c;
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