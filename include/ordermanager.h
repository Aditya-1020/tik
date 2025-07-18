#pragma once

#include <string>
#include <optional>
#include <iostream>
#include "orders.h"


struct TradeOrder {
    enum class Side {BUY, SELL};
    std::string symbol;
    Side side;
    double price;
    int quantity;
};

class OrderManager {
public:
    OrderManager();

    std::optional<TradeOrder> evaluateMarket(const OrderBook &book, const std::string &symbol);

private:
    double TargetBuyPrice;
};