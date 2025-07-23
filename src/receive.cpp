// UDP receiver
#include "receive.h"
#include <optional>
#include <sched.h>
#include <numeric>
#include <algorithm>

Data_receiver::Data_receiver() : order_manager("config.json") {
    for (int i = 0; i < MESSAGE_POOL_SIZE; ++i) {
        auto msg = new std::string();
        msg->reserve(MESSAGE_ALLOC);
        available_messages.push(msg);
    }
}

Data_receiver::~Data_receiver() {
    stop();
    
    std::string *msg;
    while (available_messages.pop(msg)) {
        delete msg;
    } 
    while (market_data_queue.pop(msg)) {
        delete msg;
    }
}

void Data_receiver::pinThreadCPU(std::thread &t, int cpu_num) {
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(cpu_num, &cpuset);
    int rec = pthread_setaffinity_np(t.native_handle(), sizeof(cpuset), &cpuset);
    if (rec != 0) {
        std::cerr << "Error calling pthread " << rec << '\n';
    }
}

void Data_receiver::start() {
    should_stop = false;

    try {
        // start thread        
        market_data_thread = std::thread(&Data_receiver::recieveMarketDataLoop, this);
        order_processing_thread = std::thread(&Data_receiver::processOrdersLoop, this);

        pinThreadCPU(market_data_thread, 0);
        pinThreadCPU(order_processing_thread, 1);

        std::this_thread::sleep_for(std::chrono::milliseconds(100));

    } catch (const std::exception &e) {
        std::cerr << "ERROR: failed thread start" << std::endl;
        stop();
        throw;
    }
}

void Data_receiver::stop() {
    should_stop = true;

    std::vector<std::thread*> threads = {
        &market_data_thread, &order_processing_thread
    };
    
    for (auto* t : threads) {
        if (t->joinable()) {
            t->join();
        }
    }
    
    std::cout << "STOPPed" << std::endl;

}

void Data_receiver::recieveMarketDataLoop(){

    struct sched_param param;
    param.sched_priority = sched_get_priority_max(SCHED_FIFO);
    pthread_setschedparam(pthread_self(), SCHED_FIFO, &param);

    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        std::cerr << "Socket Creation Failed\n";
        return;
    }
    
    int recv_buf_size = BUFFER_SIZE * BUFFER_SIZE;
    setsockopt(sock, SOL_SOCKET, SO_RCVBUF, &recv_buf_size, sizeof(recv_buf_size));
    
    sockaddr_in server_address{};
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(RECIEVE_PORT);
    server_address.sin_addr.s_addr = inet_addr("127.0.0.1");


    if (bind(sock, (sockaddr *)&server_address, sizeof(server_address)) < 0){
        std::cerr << "Bind failed at port " << RECIEVE_PORT << ": " << strerror(errno) << std::endl;
        std::cerr << "Connection Failed\n";
        close(sock);
        return;
    }

    char buffer[BUFFER_SIZE];
    sockaddr_in client_address{};
    socklen_t client_len = sizeof(client_address);

    
    while (!should_stop){
        std::memset(buffer, 0, sizeof(buffer));
        int bytes_read = recvfrom(sock, buffer, sizeof(buffer), 0, (sockaddr *)&client_address, &client_len);

        if (bytes_read <= 0) continue;

        message_received++;

        // Message from pool
         std::string *msg_ptr;
        if (available_messages.pop(msg_ptr)) {
            available_messages.pop(msg_ptr);
        } else {
            msg_ptr = new std::string();
            msg_ptr->reserve(MESSAGE_ALLOC);
        }

        msg_ptr->assign(buffer, bytes_read); // reuse string memory

        while (!market_data_queue.push(msg_ptr)) {
            std::this_thread::yield();
        }
    }
    close(sock);
}

void Data_receiver::processOrdersLoop() {
    struct sched_param param;
    param.sched_priority = sched_get_priority_max(SCHED_FIFO);
    pthread_setschedparam(pthread_self(), SCHED_FIFO, &param);

    std::cout << "processing order thread" << std::endl;

    std::string *raw_msg;
    
    while (!should_stop) {
        if (!market_data_queue.pop(raw_msg)) {
            std::this_thread::yield();
            continue;
        }

        auto start_time = std::chrono::high_resolution_clock::now();

        MessageRouter::parseMarketData(*raw_msg, orderbook);
        
        auto after_parse_time = std::chrono::high_resolution_clock::now();

        std::optional<TradeOrder> potential_order = order_manager.evaluateMarket(orderbook);
        auto after_eval_time = std::chrono::high_resolution_clock::now();

        if (potential_order.has_value()) {
            orders_generated++;
            TradeOrder& order_to_send = potential_order.value();
            
            std::string fix_message = FIXParser::serializeOrder(order_to_send, *this);
            
            auto end_time = std::chrono::high_resolution_clock::now();

            auto tick_to_trade_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(end_time - start_time).count();
            auto parser_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(after_parse_time - start_time).count();
            auto order_manager_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(after_eval_time - after_parse_time).count();
            
            if (latency_index.load() < MAX_LATENCY_SAMPLES) {
                int idx = latency_index.fetch_add(1);
                if (idx < MAX_LATENCY_SAMPLES) {
                    tick_to_trade_latency[idx] = tick_to_trade_ns;
                    parser_latency[idx] = parser_ns;
                    order_manager_latency[idx] = order_manager_ns;
                }
            }
        }
        
        raw_msg->clear(); // return to pool
        while (!available_messages.push(raw_msg)) {
            std::this_thread::yield();
        }
    }
    std::cout << "[thread order] order thread stopped" << std::endl;
}

void Data_receiver::sendMarketData(const std::string_view send_order_message) {
    static thread_local int send_sock = -1;
    static thread_local sockaddr_in exchange_address{};
    
    if (send_sock == -1) { // Reuse socket connection
        send_sock = socket(AF_INET, SOCK_DGRAM, 0);
        if (send_sock < 0) {
            std::cerr << "Sending Socket Creation Failed\n";
            return;
        }
        
        exchange_address.sin_family = AF_INET;
        exchange_address.sin_port = htons(EXCHANGE_PORT);
        exchange_address.sin_addr.s_addr = inet_addr("127.0.0.1");
    }

    ssize_t bytes_sent = sendto(send_sock, send_order_message.data(), send_order_message.length(), 0, 
                                (sockaddr *)&exchange_address, sizeof(exchange_address));

    if (bytes_sent < 0) {
        std::cerr << "Failed to send order Message\n";
    }
}

void Data_receiver::printstats() const {
    std::cout << "Performance stats\n";
    std::cout << "messages received: " << message_received.load() << '\n';
    std::cout << "orders generated: " << orders_generated.load() << '\n';

    int sample_count = std::min(latency_index.load(), MAX_LATENCY_SAMPLES);
    if (sample_count > 0) {
        auto print_latency = [sample_count](const std::string& name, const std::array<long long, MAX_LATENCY_SAMPLES>& latencies) {
            long long sum = 0;
            long long min_val = LLONG_MAX;
            long long max_val = 0;
            
            for (int i = 0; i < sample_count; ++i) {
                sum += latencies[i];
                min_val = std::min(min_val, latencies[i]);
                max_val = std::max(max_val, latencies[i]);
            }
            
            double avg = static_cast<double>(sum) / sample_count;
            std::cout << name << " (ns): Avg=" << avg << ", Min=" << min_val << ", Max=" << max_val << '\n';
        };
        
        print_latency("tick to trade: ", tick_to_trade_latency);
        print_latency("parser latency: ", parser_latency);
        print_latency("order manager: ", order_manager_latency);
    }
}