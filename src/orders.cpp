// Read market data and handle order-in/out

#include <fstream>
#include <iostream>
#include <string.h>
#include <vector>

std::vector<char> ReadFile(const std::string &filename){
    std::ifstream file(filename, std::ios::binary | std::ios::ate);
    if (!file) {
        throw std::runtime_error("Could not open file");
    }

    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    std::vector<char> buffer(size);
    if (!file.read(buffer.data(), size)){
        throw::std::runtime_error("Error reading file");
    }
    
    return buffer;
}

std::vector<char> data = ReadFile("example.txt"); // Pass file in


class Orders {
    void read_market_data() {



    }
};