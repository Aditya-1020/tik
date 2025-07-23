#pragma once

#include <string>
#include <iostream>
#include <cstring>
#include <memory>
#include <array>
#include <climits>

#include <thread>
#include <atomic>
#include <chrono>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include <boost/lockfree/spsc_queue.hpp>

#include "orders.h"
#include "parser.h"
#include "ordermanager.h"

#define BUFFER_SIZE 1024
#define RECIEVE_PORT 9000
#define EXCHANGE_PORT 9001
#define MESSAGE_POOL_SIZE 1000
#define MAX_LATENCY_SAMPLES 10000

#pragma once

#include <string>
#include <iostream>
#include <cstring>
#include <memory>
#include <array>
#include <climits>

#include <thread>
#include <atomic>
#include <chrono>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include <boost/lockfree/spsc_queue.hpp>

#include "orders.h"
#include "parser.h"
#include "ordermanager.h"

#define BUFFER_SIZE 1024
#define RECIEVE_PORT 9000
#define EXCHANGE_PORT 9001
#define MESSAGE_POOL_SIZE 1000
#define MAX_LATENCY_SAMPLES 10000
#define MESSAGE_ALLOC 512

class Data_receiver {
public:
    Data_receiver(); // constructor
    ~Data_receiver(); // destructor

    void start();
    void stop();
    void sendMarketData(const std::string_view send_order_message);
    void printstats() const;

private:
    std::thread market_data_thread;
    std::thread order_processing_thread;

    // lock fre queue
    boost::lockfree::spsc_queue<std::string*> market_data_queue{BUFFER_SIZE};
    boost::lockfree::spsc_queue<std::string*> available_messages{MESSAGE_POOL_SIZE};
    
    std::atomic<bool> should_stop{false};
    std::atomic<uint64_t> message_received{0};
    std::atomic<uint64_t> orders_generated{0};
    
    std::array<long long, MAX_LATENCY_SAMPLES> tick_to_trade_latency{};
    std::array<long long, MAX_LATENCY_SAMPLES> parser_latency{};
    std::array<long long, MAX_LATENCY_SAMPLES> order_manager_latency{};
    std::atomic<int> latency_index{0};

    OrderBook orderbook;
    OrderManager order_manager;

    void recieveMarketDataLoop();
    void processOrdersLoop();
    void pinThreadCPU(std::thread &t, int cpu_num);
};