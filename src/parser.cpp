// Parse Data from Other streams
#include "parser.h"
#include <iostream>

Market_data MessageParser::parseMarketData(const std::string &message){
    // Convert message into Market_data struct
    // Also detect what type of format and route to the right parser

}

Market_data FIXParser::parseMarketData(const std::string &message){
    // FIX parser for market data

}

Market_data FileParser::parseMarketData(const std::string &message) {
    // Reuse dummy logic for now
    return FIXParser::parseMarketData(message);
}

Market_data SBEParser::parseMarketData(const std::vector<uint8_t> &buffer) {
    // TODO: Binary parsing
    return Market_data{};
}