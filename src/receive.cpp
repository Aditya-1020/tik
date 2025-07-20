// UDP receiver
#include "receive.h"
#include <optional>

void Data_receiver::reciveMarketData(){
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
    
    while (true){
        std::memset(buffer, 0, sizeof(buffer));
        int bytes_read = recvfrom(sock, buffer, sizeof(buffer), 0, (sockaddr *)&client_address, &client_len);

        if (bytes_read <= 0) break;

        std::string raw_msg(buffer, bytes_read);
        MessageRouter::parseMarketData(raw_msg, orderbook);

        double best_bid = orderbook.getBestBid();
        double best_ask = orderbook.getBestAsk();
        
        std::cout << "Update Proesed | Current Best Bid/Ask: " << best_bid << "/" << best_ask << "\n" << std::endl;

        std::optional<TradeOrder> potential_order = order_manager.evaluateMarket(orderbook, "EUR/USD");

        if (potential_order.has_value()) {
            TradeOrder order_to_send = potential_order.value();

            // TODU: Serialize order into FIX/SBE and send back to exchange

            
            

                // temp print testing
                std::cout << "_------_" << std::endl;
                std::cout << "Trade action: " << (order_to_send.side == TradeOrder::Side::BUY ? "BUY" : "SELL") << " " << order_to_send.quantity << " " << order_to_send.symbol << " @ " << order_to_send.price << std::endl;
                std::cout << "_------_" << std::endl;
            }
        }
    close(sock);
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
    exchange_address.sin_addr.s_addr = inet_addr("EXCHANGE_IP");

    ssize_t bytes_sent = sendto(send_sock, send_order_message.data(), send_order_message.length(), 0, (sockaddr *)&exchange_address, sizeof(exchange_address));

    if (bytes_sent < 0) {
        std::cerr << "Failed to send order Message\n";
        return;
    } else std::cout << "Order sent: " << bytes_sent << "bytes\n";


    close(send_sock);
    
    // DEBUG
    // std::cout << "[SEND]: FIX MSSAGE : " << send_order_message << std::endl;
}