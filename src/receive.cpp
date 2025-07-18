// UDP receiver
#include "receive.h"

void Data_receiver::reciveMarketData(){
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        std::cerr << "Socket Creation Failed\n";
        return;
    }
    
    sockaddr_in server_address{};
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(9000);
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
        Market_data md = MessageRouter::parseMarketData(raw_msg);
        
        std::cout << "Received: " << md.symbol << " " << md.bid_price << " " << md.ask_price << "\n";
    }
    
    
    // todw: serialize order and send it
    
    close(sock);
}
