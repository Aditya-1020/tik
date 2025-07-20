#pragma once

#include <string>
#include <iostream>
#include <cstring>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#include "orders.h"
#include "parser.h"
#include "ordermanager.h"

#define BUFFER_SIZE 1024
#define RECIEVE_PORT 9000
#define EXCHANGE_PORT 9001
class Data_receiver {
public:
    void reciveMarketData();
    void sendMarketData(const std::string_view send_order_message);

private:
    OrderBook orderbook;
    OrderManager order_manager;
};