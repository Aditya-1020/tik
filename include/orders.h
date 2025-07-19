#pragma once

#include <map>

class OrderBook {
private:
    std::map<double, int> bids;   // price -> quantity (descending)
    std::map<double, int> asks;   // price -> quantity (ascending)

public:
    void updateBid(double price, int quantity);
    void updateAsk(double price, int quantity);

    // Getters
    double getBestBid() const;
    double getBestAsk() const;
    int getBidQuantity(double price) const;
    int getAskQuantity(double price) const;
};