#include "../include/server.hpp"

void Server::cmdPass(int fd, std::vector<std::string>& args) {
    if (args.size() < 2) {
        clients[fd].response = "461 * PASS :Not enough parameters\r\n";
        return;
    }
    
    if (args[1] == data.passwd) {
        if (clients[fd].getAuth() == false)
            clients[fd].setPassauth();
    } else {
        clients[fd].response = "464 * :Password incorrect\r\n";
        clients[fd].sendResponse();
    }
}