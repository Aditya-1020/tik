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

    symbol = config["symbol"];
    target_buy_price = config["target_buy_price"];
    order_quantity = config["order_quantity"];

    std::cout << "[Ordermanager] Initialized order for " << symbol << ":" << target_buy_price << std::endl;
}

std::optional<TradeOrder> OrderManager::evaluateMarket(const OrderBook &book, const std::string_view symbol_sv) {
    if (symbol_sv != this->symbol) {
        return std::nullopt;
    }

    double best_ask = book.getBestAsk();


    if (best_ask > 0.0) {
        if (best_ask < this->target_buy_price) {
            std::cout << "[OrderManager] Strategy triggered! Best ask (" << best_ask << ") is below target (" << this->target_buy_price << ")." << std::endl;

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
