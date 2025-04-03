#include "../include/server.hpp"

void Server::cmdPart(int fd, std::vector<std::string>& args) {
    if (args.size() < 2) {
        sendError(fd, "461", "PART", "Not enough parameters");
        return;
    }
    
    std::stringstream ss(args[1]);
    std::string channelName;
    while (std::getline(ss, channelName, ',')) {
        if (channels.find(channelName) == channels.end()) {
            sendError(fd, "403", channelName, "No such channel");
            continue;
        }
        
        std::string response = ":" + clients[fd].getNick() + " PART " + channelName + "\r\n";
        broadcastToChannel(channelName, response, -1);
        channels[channelName].removeUser(&clients[fd]);
        
        if (channels[channelName].isEmpty()) {
            channels.erase(channelName);
        }
    }
}