#include "../include/server.hpp"

void Server::cmdPass(int fd, std::vector<std::string>& args) {
    if (args.size() < 2) {
        sendError(fd, "461", "PASS", "Not enough parameters");
        return;
    }
    
    if (args[1] == data.passwd) {
        if (clients[fd].getAuth() == false)
            clients[fd].setPassauth();
    } else {
        sendError(fd, "464", "PASS", "Password incorrect");
    }
}