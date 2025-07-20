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

class Data_receiver {
public:
    void reciveMarketData();
    void sendMarketData();

private:
    OrderBook orderbook;
    OrderManager order_manager;
};