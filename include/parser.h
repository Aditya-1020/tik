#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include <string_view>

#define CHECKSUM_MODULO 256
#define EST_FIX_MESSAGE_SIZE 256

// Forward declarations
class OrderBook; 
struct TradeOrder;
class Data_receiver;

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
    static void parseMarketData(const std::string_view &message, OrderBook &book);
};

class FIXParser {
public:
    enum class State{
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
        std::string_view entry_type; // Changed to string_view for performance
        double price = 0.0;
        int quantity = 0;
        bool has_price = false;
        bool has_quantity = false;

        void reset() {
            entry_type = std::string_view{};
            price = 0.0;
            quantity = 0;
            has_price = false;
            has_quantity = false;
        }
    };

    // Parsing
    static void parseMarketData(const std::string_view &message, OrderBook &book);

    // Serialize Order
    static std::string serializeOrder(const TradeOrder &order, Data_receiver &receiver,
        std::string_view sender_id = "TRADER",
        std::string_view target_id = "EXCHANGE");

private:
    // Parsing helpers (kept for compatibility)
    static State processChar(char c, State current_state, std::string &current_tag, std::string &current_value, MDContext &md_context, OrderBook &book, std::string &symbol);
    static void processField(std::string_view tag, std::string_view value, MDContext &md_context, OrderBook &book, std::string &symbol); 
    static State determineNextState(const std::string_view &tag, const MDContext &md_context);
    static void commitMDEntry(MDContext &md_context, OrderBook &book);
    
    // Serialize helpers
    static std::string generateOrderID();
    static std::string getTimestamp();
    static int calculateChecksum(const std::string_view &message);
    static std::string formatPrice(double price);
};

// class SBEParser {
// public:
//     static Market_data parseMarketData(const std::vector<uint8_t> &buffer);
// };