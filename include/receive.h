#pragma once
#include <fstream>
#include <sstream>
#include <string>
#include <thread>
#include <iostream>
#include <vector>
#include <netinet/in.h>
#include <unistd.h>
#include "parser.h"
#include "receive.h"
#include <sys/socket.h>
#include <cstring>
#include <arpa/inet.h>
#include <unistd.h>

#define BUFFER_SIZE 1024

class Data_receiver {
public:
    void reciveMarketData();
};