#pragma once

#include <string>
#include <optional>
#include <iostream>
#include "json.hpp"


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
    OrderManager(const std::string &config_path);
    std::optional<TradeOrder> evaluateMarket(const OrderBook& book, std::string_view symbol);

private:
    std::string symbol;
    int order_quantity;
    double target_buy_price;
};