// UDP receiver
#include "receive.h"
#include <optional>
#include <sched.h>

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
            std::string raw_msg = market_data_queue.front();
            market_data_queue.pop();

            lock.unlock();

            MessageRouter::parseMarketData(raw_msg, orderbook);

            std::optional<TradeOrder> potential_order = order_manager.evaluateMarket(orderbook, "EUR/USD");

            if (potential_order.has_value()) {
                TradeOrder order_to_send = potential_order.value();

                // Serialize and send order
                std::string fix_message = FIXParser::serializeOrder(order_to_send, *this);
                
                std::cout << "[Thread-Order] Order placed: " << (order_to_send.side == TradeOrder::Side::BUY ? "BUY" : "SELL") 
                          << " " << order_to_send.quantity << " " << order_to_send.symbol 
                          << " @ " << order_to_send.price << std::endl;
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
    std::cout << "Messages recieved: " << message_received.load() << '\n';
    std::cout << "Orders generated: " << orders_generated.load() << '\n';
}