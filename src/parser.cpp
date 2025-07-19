#include "parser.h"
#include "orders.h"
#include "ordermanager.h"
#include <iostream>
#include <chrono>
#include <random>
#include <sstream>
#include <iomanip>

void MessageRouter::parseMarketData(const std::string &message, OrderBook &book){
    if (message.size() >= 5 && 
        message[0] == '8' &&
        message[1] == '=' &&
        message[2] == 'F' &&
        message[3] == 'I' &&
        message[4] == 'X') {
            FIXParser::parseMarketData(message, book);
    }
}

void FIXParser::parseMarketData(const std::string &message, OrderBook &book){
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

void FIXParser::processField(const std::string &tag, const std::string &value, MDContext &md_context, OrderBook& book, std::string& symbol) {
    int tag_int = std::stoi(tag);
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
            md_context.price = std::stod(value);
            md_context.has_price = true;
            if (md_context.has_quantity) { commitMDEntry(md_context, book, symbol); }
            break;
        case 271: // Quantity
            md_context.quantity = std::stoi(value);
            md_context.has_quantity = true;
            if (md_context.has_price) { commitMDEntry(md_context, book, symbol); }
            break;
        // TODU: Other tags 35, 52
    }
}

void FIXParser::commitMDEntry(MDContext &md_context, OrderBook& book, const std::string& symbol) {
    // We can't update a book without a symbol
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

FIXParser::State FIXParser::determineNextState(const std::string &tag, const MDContext &md_context){
    (void)md_context; // unused parameter
    int tag_int = std::stoi(tag);
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

std::string FIXParser::serializeOrder(const TradeOrder &order, const std::string &sender_id, const std::string &target_id) {
    std::ostringstream fix_message;

    fix_message << "8=FIX.4.2\x01"; // header


}