#include "receive.h"
#include "parser.h"
#include <iostream>
#include <csignal>
#include <atomic>


std::atomic<bool> shutdown_request{false};

void signalhandle([[maybe_unused]] int signal) {
    shutdown_request = true;
}

int main(){

    std::signal(SIGINT, signalhandle);
    std::signal(SIGTERM, signalhandle);

    std::cout << "program started" << std::endl;

    Data_receiver receiver;
    
    try {
        receiver.start();

        while(!shutdown_request) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }

        // std::cout << "shutdwn" << std::endl;
        receiver.stop();
        receiver.printstats();

    } catch (const std::exception &e) {
        std::cerr << "ERROR" << e.what() << std::endl;
        return 1;
    }

    return 0;
}