// Parse Data from Other streams
#include "parser.h"


Market_data MessageRouter::parseMarketData(const std::string &message){
    
    if (message.compare(0,5, "8=FIX") == 0) {
        return FIXParser::parseMarketData(message);
    } // else add SBE

    return Market_data{};
}

Market_data FIXParser::parseMarketData(const std::string &message){

    State current_state = State::INITIAL_STATE;
    std::string current_tag = "";
    std::string current_value = "";
    Market_data result;
    MDContext md_context;

    for (char c : message) {
        current_state = processChar(c, current_state, current_tag, current_value, md_context, result);
    
        if (current_state == State::ERROR_STATE) {
            result.is_valid = false;
            return result;
        }
    }

    if (current_state == State::COMPLETE) {
        result.is_valid = true;
        return result;
    }

    return Market_data{};
}

// Process each char based on state
FIXParser::State FIXParser::processChar(char c, State current_state, std::string &current_tag, std::string current_value, MDContext &md_context, Market_data &result) {

    switch (current_state) {
        case State::INITIAL_STATE:
            if (std::isdigit(c)) {
                current_tag += c;
                return State::READING_TAG;
            }
            return State::ERROR_STATE;

        case State::READING_TAG:
            if (std::isdigit(c)) {
                current_tag += c;
                return State::READING_TAG;
            } else if (c == '=') {
                return State::READING_VALUE;
            }
            return State::ERROR_STATE;
        
        case State::READING_VALUE:
            if (c = '\x01') {
                processField(current_tag, current_value, md_context, result);
                State next_state = determineNextState(current_tag, md_context);
                
                // Reset for next state
                current_tag.clear();
                current_value.clear();

                return next_state;
            } else {
                current_value += c;
                return State::READING_VALUE;
            }
        
        case State::MD_ENTRY_TYPE_FOUND:
            if (std::isdigit(c)) {
                current_tag += c;
                return State::READING_TAG;
            }
            return State::ERROR_STATE;
        
        case State::EXPECTING_QUANTITY:
            if (std::isdigit(c)) {
                current_tag += c;
                return State::READING_TAG;
            }
            return State::ERROR_STATE;
            
        case State::ERROR_STATE: return State::ERROR_STATE;
        
        case State::COMPLETE: return State::COMPLETE;
        
        default: return State::ERROR_STATE;
    }
}


void FIXParser::processField(const std::string &tag, const std::string &value, MDContext &md_context, Market_data &result) {

    int tag_int = std::stoi(tag);

    switch (tag_int) {
        case 35: result.message_type = value; break;
        case 55: result.symbol = value; break;
        case 52: result.timestamp = value; break;
        case 269:
            md_context.entry_type = value;
            md_context.has_price = false;
            md_context.has_quantity = false;
            break;
        case 270:
            md_context.price = std::stod(value);
            md_context.has_price = true;
            if (md_context.has_quantity){ commitMDEntry(md_context, result); }
            break;
        case 271:
            md_context.quantity = std::stoi(value);
            md_context.has_quantity = true;
            if (md_context.has_price) { commitMDEntry(md_context, result); }
            break;
    }

    std::string tag = std::to_string(tag_int);
}

void FIXParser::commitMDEntry(MDContext &md_context, Market_data &result) {
    if (md_context.entry_type == "0") {
        result.bid_price = md_context.price;
        result.bid_quantity = md_context.quantity;
    } else if (md_context.entry_type == "1") {
        result.ask_price = md_context.price;
        result.ask_quantity = md_context.quantity;
    }

    md_context.reset();
}

FIXParser::State FIXParser::determineNextState(const std::string &tag, const MDContext &md_context){
    int tag_int = std::stoi(tag);
    
    switch (tag_int){
        case 269: return State::MD_ENTRY_TYPE_FOUND; break;
        case 270: return State::EXPECTING_QUANTITY; break;
        case 271: return State::READING_TAG; break;
        case 10: return State::COMPLETE; break;
    }

    return State::READING_TAG;

    std::string tag = std::to_string(tag_int);
}

Market_data SBEParser::parseMarketData(const std::vector<uint8_t> &buffer){
    // TODU: IMPLEMENT SBE PARSING
    return;
}

