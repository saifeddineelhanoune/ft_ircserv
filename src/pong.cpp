#include "../include/server.hpp"

void Server::cmdPong(int fd, std::vector<std::string>& args) {
    if (args.size() < 2) {
        Logger::debug("Received empty PONG from " + clients[fd].getNick());
        return;
    }
    
    Logger::debug("Received PONG from " + clients[fd].getNick() + " with token: " + args[1]);
    
}