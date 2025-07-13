#pragma once

#include <fstream>
#include <sstream>
#include <regex>
#include <string>
#include <thread>
#include <iostream>
#include <vector>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include "receive.h"

struct Market_data {
    std::string symbol;
    double bid_price;
    double ask_price;
    int bid_quantity;
    int ask_quantity;
    std::string timestamp;
};

class MessageRouter {
public:
    static Market_data parseMarketData(const std::string &message);
};

class FIXParser {
public:
    static Market_data parseMarketData(const std::string &message);
};

class SBEParser {
public:
    static Market_data parseMarketData(const std::vector<uint8_t> &buffer);
};

class FileParser {
public:
    static Market_data parseMarketData(const std::string &message);
};

class CSVParser {
public:
    static Market_data parseMarketData(const std::string &message);
};