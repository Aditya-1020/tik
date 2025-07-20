#pragma once

#include <string>
#include <optional>
#include <iostream>

class OrderBook; // Forward declertion brek circular dependency

struct TradeOrder {
    enum class Side { BUY, SELL };
    std::string symbol;
    Side side;
    double price;
    int quantity;
};

class OrderManager {
public:
    OrderManager();
    std::optional<TradeOrder> evaluateMarket(const OrderBook& book, std::string_view symbol);

private:
    double TargetBuyPrice;
};