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
    std::string symbol = "";
    double bid_price = 0.0;
    double ask_price = 0.0;
    int bid_quantity = 0;
    int ask_quantity = 0;
    std::string timestamp;
};

class MessageRouter {
public:
    static Market_data parseMarketData(const std::string &message);
};

class FIXParser {
public:
    static Market_data parseMarketData(const std::string& message);
};

class SBEParser {
public:
    static Market_data parseMarketData(const std::vector<uint8_t> &buffer);
};