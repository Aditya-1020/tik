#pragma once

#include <map>
#include <string>

class OrderBook {
private:
    std::map<double, int> bids;   // price -> quantity (descending)
    std::map<double, int> asks;   // price -> quantity (ascending)

    double best_bid = 0.0;
    double best_ask = 0.0;

public:
    void updateBid(double price, int quantity);
    void updateAsk(double price, int quantity);

    // Getters
    double getBestBid() const;
    double getBestAsk() const;
    
    int getBidQuantity(double price) const;
    int getAskQuantity(double price) const;
};