#include "orders.h"

void OrderBook::updateBid(double price, int quantity) {
    if (quantity > 0) {
        bids[price] = quantity; // update price if not in map
    } else {
        bids.erase(price); // remove at this price level
    }
}

void OrderBook::updateAsk(double price, int quantity) {
    if (quantity > 0) {
        asks[price] = quantity;
    } else {
        asks.erase(price);
    }
}

double OrderBook::getBestBid() const {
    if (bids.empty()) return 0.0;
    return bids.rbegin()->first;
}

double OrderBook::getBestAsk() const {
    if (asks.empty()) return 0.0;
    return asks.begin()->first;
}

int OrderBook::getBidQuantity(double price) const {
    auto op = bids.find(price);
    return (op != bids.end()) ? op->second : 0;
}

int OrderBook::getAskQuantity(double price) const {
    auto op = asks.find(price);
    return (op != asks.end()) ? op->second : 0;
}
