# pragma once

#include "parser.h"
#include "receive.h"
#include <fstream>
#include <sstream>
#include <string>
#include <thread>
#include <iostream>
#include <vector>
#include <map>

class OrderBook {
    std::map<double, int> bids;   // price -> quantity (decending)
    std::map<double, int> asks;   // price -> quantity (acending)

public:
    void updateBid(double price, int quantity);
    void updateAsk(double price, int quantity);
};