#pragma once

#include <string>
#include <vector>

// Forward declarations to minimize dependencies
class OrderBook;
struct TradeOrder;

struct Market_data {
    std::string symbol = "";
    double bid_price = 0.0;
    double ask_price = 0.0;
    int bid_quantity = 0;
    int ask_quantity = 0;
    std::string timestamp = "";
    std::string message_type = "";
    bool is_valid = false;
};

class MessageRouter {
public:
    static void parseMarketData(const std::string& message, OrderBook& book);
};

class FIXParser {
public:
    enum class State {
        INITIAL_STATE,
        READING_TAG,
        READING_VALUE,
        MD_ENTRY_TYPE_FOUND,
        EXPECTING_PRICE,
        EXPECTING_QUANTITY,
        ERROR_STATE,
        COMPLETE
    };

    struct MDContext {
        std::string entry_type;
        double price = 0.0;
        int quantity = 0;
        bool has_price = false;
        bool has_quantity = false;

        MDContext() : entry_type(""), price(0.0), quantity(0), has_price(false), has_quantity(false) {}

        void reset() {
            entry_type = "";
            price = 0.0;
            quantity = 0;
            has_price = false;
            has_quantity = false;
        }
    };

    // Parsing functions
    static void parseMarketData(const std::string& message, OrderBook& book);
    
    // NEW: Serialization function - ADD HERE
    static std::string serializeOrder(const TradeOrder& order, 
                                    const std::string& sender_id = "TRADER",
                                    const std::string& target_id = "EXCHANGE");

private:
    // Parsing helpers
    static State processChar(char c, State current_state, std::string& current_tag, 
                           std::string& current_value, MDContext& md_context, 
                           OrderBook& book, std::string& symbol);
    static void processField(const std::string& tag, const std::string& value, 
                           MDContext& md_context, OrderBook& book, std::string& symbol);
    static State determineNextState(const std::string& tag, const MDContext& md_context);
    static void commitMDEntry(MDContext& md_context, OrderBook& book, const std::string& symbol);
    
    // NEW: Serialization helpers - ADD HERE
    static std::string generateClOrdID();
    static std::string getCurrentTimestamp();
    static int calculateChecksum(const std::string& message);
    static std::string formatPrice(double price);
};

class SBEParser {
public:
    static Market_data parseMarketData(const std::vector<uint8_t>& buffer);
};