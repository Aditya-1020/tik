# pragma once

#include <fstream>
#include <sstream>
#include <string>
#include <thread>
#include <iostream>
#include <vector>
#include <map>

class OrderBook {
    // Node: first is price and second is quantity
    std::map<double, int> bids;   // price -> quantity (decending)
    std::map<double, int> asks;   // price -> quantity (acending)

public:
    void updateBid(double price, int quantity);
    void updateAsk(double price, int quantity);

    // Helper
    double getBestBid() const;
    double getBestAsk() const;
    int getBidQuantity(double price) const;
    int getAskQuantity(double price) const;
};