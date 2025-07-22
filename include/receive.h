#pragma once

#include <string>
#include <iostream>
#include <cstring>

#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <atomic>

#include <chrono>
#include <vector>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>

#include "orders.h"
#include "parser.h"
#include "ordermanager.h"

#define BUFFER_SIZE 1024
#define RECIEVE_PORT 9000
#define EXCHANGE_PORT 9001

class Data_receiver {
public:

    Data_receiver();

    void start();
    void stop();
    void sendMarketData(const std::string_view send_order_message);
    void printstats() const;

private:
    std::thread market_data_thread;
    std::thread order_processing_thread;

    std::queue<std::string> market_data_queue;
    std::mutex queue_mutex; // temp: lock thread to this to read
    std::condition_variable queue_cv;
    std::atomic<bool> should_stop{false};

    std::atomic<uint64_t> message_received{0};
    std::atomic<uint64_t> orders_generated{0};
    
    // measure
    mutable std::vector<long long> tick_to_trade_latency;
    mutable std::vector<long long> parser_latency;
    mutable std::vector<long long> order_manager_latency;

    mutable std::mutex latency_mutex;


    OrderBook orderbook;
    OrderManager order_manager;

    void recieveMarketDataLoop(); // take UDP and quue them
    void processOrdersLoop(); // process queued messages and gentrate ordrs

    void pinThreadCPU(std::thread &t, int cpu_num);
};