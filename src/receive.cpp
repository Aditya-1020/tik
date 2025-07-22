// UDP receiver
#include "receive.h"
#include <optional>
#include <sched.h>
#include <numeric>
#include <algorithm> // min, max in queu

Data_receiver::Data_receiver() : order_manager("config.json") {}

void Data_receiver::pinThreadCPU(std::thread &t, int cpu_num) {
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(cpu_num, &cpuset);
    int rec = pthread_setaffinity_np(t.native_handle(), sizeof(cpuset), &cpuset);
    if (rec != 0) {
        std::cerr << "Error calling pthread" << rec << '\n';
    }
}


void Data_receiver::start() {
    should_stop = false;

    try {
        // start thread        
        market_data_thread = std::thread(&Data_receiver::recieveMarketDataLoop, this);
        order_processing_thread = std::thread(&Data_receiver::processOrdersLoop, this);

        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        // test print
        std::cout << "Started" << std::endl;
    
    } catch (const std::exception &e) {
        std::cerr << "ERROR: failed thread start" << std::endl;
        stop();
        throw;
    }
}

void Data_receiver::stop() {
    should_stop = true;
    queue_cv.notify_all();

    // if (market_data_thread.joinable()) {
    //     market_data_thread.join();
    // }
    // if (order_processing_thread.joinable()) {
    //     order_processing_thread.join();
    // }

    // Test later for vector based thread cheking faster on my system.
    // stores pointers to the threads in vector and checks each to join
    
    std::vector<std::thread*> threads = {
        &market_data_thread, &order_processing_thread
    };
    
    for (auto* t : threads) {
        if (t->joinable()) {
            t->join();
        }
    }

    // test print
    std::cout << "STOPPed" << std::endl;
}

void Data_receiver::recieveMarketDataLoop(){

    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        std::cerr << "Socket Creation Failed\n";
        return;
    }
    
    sockaddr_in server_address{};
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(RECIEVE_PORT);
    server_address.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (bind(sock, (sockaddr *)&server_address, sizeof(server_address)) < 0){
        std::cerr << "Connection Failed\n";
        close(sock);
        return;
    }

    char buffer[BUFFER_SIZE];
    sockaddr_in client_address{};
    socklen_t client_len = sizeof(client_address);
    
    std::cout << "[THREAD] started on port" << RECIEVE_PORT << std::endl;
    
    while (!should_stop){
        std::memset(buffer, 0, sizeof(buffer));
        int bytes_read = recvfrom(sock, buffer, sizeof(buffer), 0, (sockaddr *)&client_address, &client_len);

        if (bytes_read <= 0) continue;

        std::string raw_msg(buffer, bytes_read);

        {
            std::lock_guard<std::mutex> lock(queue_mutex);
            market_data_queue.push(raw_msg);
        }
        queue_cv.notify_all();
    }
    close(sock);
    // test print
    std::cout << "[thread] reciever stopped" << std::endl;
}

void Data_receiver::processOrdersLoop() {
    // test print
    std::cout << "processing order thread" << std::endl;

    while (!should_stop) {
        std::unique_lock<std::mutex> lock(queue_mutex);
        queue_cv.wait(lock, [this] {return !market_data_queue.empty() || should_stop;}); // true if data in queue or stop

        if (should_stop) break;

        while (!market_data_queue.empty()) {
            auto start_time = std::chrono::high_resolution_clock::now();

            std::string raw_msg = market_data_queue.front();
            market_data_queue.pop();
            lock.unlock();

            auto after_queue_time = std::chrono::high_resolution_clock::now();

            MessageRouter::parseMarketData(raw_msg, orderbook);
            
            auto after_parse_time = std::chrono::high_resolution_clock::now();

            std::optional<TradeOrder> potential_order = order_manager.evaluateMarket(orderbook, "EUR/USD");
            auto after_eval_time = std::chrono::high_resolution_clock::now();

            if (potential_order.has_value()) {
                orders_generated++;
                TradeOrder order_to_send = potential_order.value();
                
                std::string fix_message = FIXParser::serializeOrder(order_to_send, *this); // Serialize and send order
                
                auto end_time = std::chrono::high_resolution_clock::now();

                auto tick_to_trade_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(end_time - start_time).count();
                auto parser_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(after_parse_time - after_queue_time).count();
                auto order_manager_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(after_eval_time - after_parse_time).count();
                
                std::lock_guard<std::mutex> latency_lock(latency_mutex);
                tick_to_trade_latency.push_back(tick_to_trade_ns);
                parser_latency.push_back(parser_ns);
                order_manager_latency.push_back(order_manager_ns);
            }
            
            lock.lock();
        }
    }
    // test print
    std::cout << "[thread order] order thread stopped" << std::endl;
}

void Data_receiver::sendMarketData(const std::string_view send_order_message) {
    // Send back the string to market

    int send_sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (send_sock < 0) {
        std::cerr << "Sending Socket Creation Failed\n";
        return;
    }

    sockaddr_in exchange_address{};
    exchange_address.sin_family = AF_INET;
    exchange_address.sin_port = htons(EXCHANGE_PORT);
    exchange_address.sin_addr.s_addr = inet_addr("127.0.0.1");

    ssize_t bytes_sent = sendto(send_sock, send_order_message.data(), send_order_message.length(), 0, (sockaddr *)&exchange_address, sizeof(exchange_address));

    if (bytes_sent < 0) {
        std::cerr << "Failed to send order Message\n";
        return;
    }

    close(send_sock);
}

void Data_receiver::printstats() const {
    std::lock_guard<std::mutex> lock(latency_mutex);

    std::cout << "Performance stats\n";
    std::cout << "messages recieved: " << message_received.load() << '\n';
    std::cout << "orders generated: " << orders_generated.load() << '\n';

    /*Test*/
    if (!tick_to_trade_latency.empty()) {
        auto print_latency = [](const std::string& name, const std::vector<long long>& latencies) {
            long long sum = std::accumulate(latencies.begin(), latencies.end(), 0LL);
            long long min = *std::min_element(latencies.begin(), latencies.end());
            long long max = *std::max_element(latencies.begin(), latencies.end());
            double avg = static_cast<double>(sum) / latencies.size();
            
            std::cout << name << " (ns): Avg=" << avg << ", Min=" << min << ", Max=" << max << '\n';
        };
        
        print_latency("tik to trade: ", tick_to_trade_latency);
        print_latency("parser latency: ", parser_latency);
        print_latency("Order manager: ", order_manager_latency);
    }
}