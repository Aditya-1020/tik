// Parse Data from Other streams
#include "parser.h"


Market_data MessageRouter::parseMarketData(const std::string &message){
    
    if (message.find("8=FIX") != std::string::npos) {
        return FIXParser::parseMarketData(message);
    } // else add SBE

    return Market_data{};
}

Market_data FIXParser::parseMarketData(const std::string& message) {
    Market_data data;
    std::string current_md_entry_type;
    
    // Split by SOH character (\x01)
    // Parse each tag=value pair
    // Call processField for each pair

    
    
    return data;
}