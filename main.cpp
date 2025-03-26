#include"include/server.hpp"
#include"include/client.hpp"
#include"include/channel.hpp"

#include <sstream>
#include <string>
#include <iostream>

int stringToInt(const std::string& str) {
    std::stringstream ss(str);
    int result;
    ss >> result;
    if (ss.fail()) {
        throw std::invalid_argument("Invalid input string");
    }
    return result;
}

int main (int argc, char **argv) {
    if (argc != 3) {
        std::cerr << "Usage: ./server <password> <port>" << std::endl;
        return 1;
    }
    if (!isStringDigits(argv[1]))
    {
        std::cerr << "Error: Port is not a number" << std::endl;
        return false;
    }
    Server server(argv[2], stringToInt(argv[1]));
    server.startServer();
    return 0;
}