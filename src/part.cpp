#include "../include/server.hpp"

void Server::cmdPart(int fd, std::vector<std::string>& args) {
    if (args.size() < 2) {
        sendError(fd, "461", "PART", "Not enough parameters");
        return;
    }
    
    std::string channelName = args[1];
    if (channels.find(channelName) == channels.end()) {
        sendError(fd, "403", channelName, "No such channel");
        return;
    }
    
    std::string response = ":" + clients[fd].getNick() + " PART " + channelName + "\r\n";
    broadcastToChannel(channelName, response, -1);
    channels[channelName].removeUser(&clients[fd]);
    
    // Remove channel if empty
    if (channels[channelName].isEmpty()) {
        channels.erase(channelName);
    }
}