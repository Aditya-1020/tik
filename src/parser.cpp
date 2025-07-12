// Parse Data from Other streams
#include "parser.h"
#include <iostream>

Market_data MessageRouter::parseMarketData(const std::string &message){
    
    if (message.find("8=FIX") != std::string::npos) {
        return FIXParser::parseMarketData(message);
    } else {
        std::ifstream file(message);
        if (file.good()) {
            file.close();
            return FileParser::parseMarketData(message);
        } else if (message.find(',') != std::string::npos) {
            return CSVParser::parseMarketData(message);
        }
    }

    return Market_data{};
}

Market_data FIXParser::parseMarketData(const std::string &message){
    // TODO: FIX parser implementation
    // https://www.fixtrading.org/standards/ 
    // https://www.nasdaqtrader.com/content/technicalsupport/specifications/dataproducts/NQTVITCHspecification.pdf
    return Market_data{};
}

Market_data CSVParser::parseMarketData(const std::string &message) {
    // Ex: AAPL,150.25,150.30,100,200,1234567890
    
    std::vector<std::string> parts;
    std::stringstream ss(message);
    std::string part;

    while (std::getline(ss, part, ',')) {
        parts.push_back(part);
    }

    if (parts.size() != 6) {
        return Market_data{}; // wrong format
    }

    Market_data data;
    data.symbol = parts[0];
    data.bid_price = std::stod(parts[1]);
    data.ask_price = std::stod(parts[2]);
    data.bid_quantity = std::stoi(parts[3]);
    data.ask_quantity = std::stoi(parts[4]);
    data.timestamp = std::stol(parts[5]);

    return data;
}


Market_data FileParser::parseMarketData(const std::string &message) {

    try {
        std::ifstream file(message, std::ios::binary | std::ios::ate);
        if (!file) {
            throw std::runtime_error("Could not open file");
        }

        std::streamsize size = file.tellg();
        file.seekg(0, std::ios::beg);

        std::vector<char> buffer(size);
        if (!file.read(buffer.data(), size)) {
            throw std::runtime_error("Error reading file");
        }
        std::string fileContent(buffer.begin(), buffer.end());
        return CSVParser::parseMarketData(fileContent);
       
    } catch(const std::exception& e) {
        std::cerr << "File parsing error: " << e.what() << std::endl;
        return Market_data{};
    }
}

Market_data SBEParser::parseMarketData(const std::vector<uint8_t> &buffer) {
    // TODO: Binary parsing implementation
    return Market_data{};
}
