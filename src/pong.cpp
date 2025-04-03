#include "../include/server.hpp"

void Server::cmdPong(int fd, std::vector<std::string>& args) {
    // In a more complex implementation, you might want to track 
    // which clients have responded to pings and handle timeouts
    
    if (args.size() < 2) {
        Logger::debug("Received empty PONG from " + clients[fd].getNick());
        return;
    }
    
    Logger::debug("Received PONG from " + clients[fd].getNick() + " with token: " + args[1]);
    
    // No response needed, just log it
}