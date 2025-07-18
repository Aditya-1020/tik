#pragma once
#include <string>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <iostream>

#include "orders.h"
#include "parser.h"
#include "ordermanager.h"

#define BUFFER_SIZE 1024

class Data_receiver {
public:
    void reciveMarketData();

private:
    OrderBook orderbook;
    OrderManager order_manager;
};