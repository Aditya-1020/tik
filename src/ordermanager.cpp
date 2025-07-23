#include "ordermanager.h"
#include "orders.h"
#include <fstream>

OrderManager::OrderManager(const std::string &config_path) {
    std::ifstream config_file(config_path);
    if (!config_file.is_open()) {
        throw std::runtime_error("Coud not open config file: " + config_path);
    }

    nlohmann::json config;
    config_file >> config;

    // this->symbol = config["symbol"];
    this->target_buy_price = config["target_buy_price"];
    this->order_quantity = config["order_quantity"];

}

std::optional<TradeOrder> OrderManager::evaluateMarket(const OrderBook &book,[[maybe_unused]] const std::string_view symbol) {
    double best_ask = book.getBestAsk();

    if (best_ask > 0.0 && best_ask < this->target_buy_price) {
        if (best_ask < this->target_buy_price) {
            TradeOrder new_order;
            new_order.symbol = this->symbol;
            new_order.side = TradeOrder::Side::BUY;
            new_order.price = best_ask;
            new_order.quantity = this->order_quantity;

            return new_order;
        }
    }
    return std::nullopt;
}
