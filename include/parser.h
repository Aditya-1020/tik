#pragma once

#include <fstream>
#include <sstream>
#include <string>
#include <thread>
#include <iostream>
#include <vector>
#include <netinet/in.h>
#include <unistd.h>
#include <map>
#include <unordered_map>
#include <functional>

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
    static Market_data parseMarketData(const std::string &message);
};

class FIXParser {
public:
    enum class State{
        INITIAL_STATE,
        READING_TAG,
        READING_VALUE,
        PROCESSING_REAPEAT_GRP,
        MD_ENTRY_TYPE_FOUND,
        EXPECTING_PRICE,
        EXPECTING_QUANTITY,
        ERROR_STATE,
        COMPLETE
    };

    struct MDContext {
        std::string entry_type; // 0 = bid, 1 = ask -- Self note why not use bool?
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

    static Market_data parseMarketData(const std::string &message);
    
private:
    
    // helper Functions for sm
    // handle char based on state
    static State processChar(char c, State current_state, std::string &current_tag, std::string &current_value, MDContext &md_context, Market_data &result);
    // process tag=value
    static void processField(const std::string &tag, const std::string &value, MDContext &md_context, Market_data &result); 
    static State determineNextState(const std::string &tag, const MDContext &md_context);
    static void commitMDEntry(MDContext &md_context, Market_data &result); // commit market data entry to result

};

class SBEParser {
public:
    static Market_data parseMarketData(const std::vector<uint8_t> &buffer);
};