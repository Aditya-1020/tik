#include "orders.h"

void OrderBook::updateBid(double price, int quantity) {
    if (quantity > 0) {
        bids[price] = quantity; // update price if not in map
    } else {
        bids.erase(price); // remove at this price level
    }

    best_bid = bids.empty() ? 0.0 : bids.rbegin()->first;
}

void OrderBook::updateAsk(double price, int quantity) {
    if (quantity > 0) {
        asks[price] = quantity;
    } else {
        asks.erase(price);
    }
    
    best_ask = asks.empty() ? 0.0 : asks.begin()->first;
}

double OrderBook::getBestBid() const {
    return best_bid;
}

double OrderBook::getBestAsk() const {
    return best_ask;
}

int OrderBook::getBidQuantity(double price) const {
    auto op = bids.find(price);
    return (op != bids.end()) ? op->second : 0;
}

int OrderBook::getAskQuantity(double price) const {
    auto op = asks.find(price);
    return (op != asks.end()) ? op->second : 0;
}

// need to add tracking for indiv orders for adding, calcleing, executng
