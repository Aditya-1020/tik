#include "ordermanager.h"
#include "orders.h"

OrderManager::OrderManager() {
    TargetBuyPrice = 1.1235;
    std::cout << "[OrderManager] initialized. Target buy price for EUR/USD: " << TargetBuyPrice << std::endl;
}

std::optional<TradeOrder> OrderManager::evaluateMarket(const OrderBook &book, const std::string_view symbol) {
    if (symbol != "EUR/USD") {
        return std::nullopt;
    }

    double best_ask = book.getBestAsk();


    if (best_ask > 0.0) {
        if (best_ask < TargetBuyPrice) {
            std::cout << "[OrderManager] Strategy triggered! Best ask (" << best_ask << ") is bwlow target (" << TargetBuyPrice << ")." << std::endl;

            TradeOrder new_order;
            new_order.symbol = symbol;
            new_order.side = TradeOrder::Side::BUY;
            new_order.price = best_ask;
            new_order.quantity = 1000;

            return new_order;
        }
    }
    return std::nullopt;
}
