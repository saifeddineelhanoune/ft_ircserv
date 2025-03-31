#include "../include/server.hpp"

void Server::cmdJoin(int fd, std::vector<std::string>& args) {
    if (args.size() < 2) {
        sendError(fd, "461", "JOIN", "Not enough parameters");
        return;
    }
    
    if (!clients[fd].getAuth()) {
        sendError(fd, "451", "JOIN", "You have not registered");
        return;
    }
    
    std::string channelName = args[1];
    if (channelName[0] != '#') {
        channelName = "#" + channelName;
    }
    
    // Check if channel has a key and if the provided key is correct
    if (channels.find(channelName) != channels.end() && !channels[channelName].getKey().empty()) {
        if (args.size() < 3 || args[2] != channels[channelName].getKey()) {
            sendError(fd, "475", channelName, "Cannot join channel (+k) - bad key");
            return;
        }
    }
    
    // Check if channel is invite-only and user is invited
    if (channels.find(channelName) != channels.end() && channels[channelName].isInviteOnly()) {
        if (!channels[channelName].isInvited(fd)) {
            sendError(fd, "473", channelName, "Cannot join channel (+i) - you must be invited");
            return;
        }
    }
    
    // Check user limit
    if (channels.find(channelName) != channels.end() && 
        channels[channelName].getUserLimit() > 0 && 
        channels[channelName].getUsers().size() >= (size_t)channels[channelName].getUserLimit()) {
        sendError(fd, "471", channelName, "Cannot join channel (+l) - channel is full");
        return;
    }
    
    bool isNewChannel = channels.find(channelName) == channels.end();
    
    // Create channel if it doesn't exist
    if (isNewChannel) {
        Channel newChannel;
        channels[channelName] = newChannel;
    }
    
    channels[channelName].addUser(&clients[fd]);
    
    // Make the first user who joins a channel an operator
    if (isNewChannel || channels[channelName].getUsers().size() == 1) {
        channels[channelName].addOperator(fd);
    }
    
    std::string response = ":" + clients[fd].getNick() + " JOIN " + channelName + "\r\n";
    broadcastToChannel(channelName, response, -1);
    
    // Send channel topic to the user
    std::string topic = channels[channelName].getTopic();
    if (!topic.empty()) {
        std::string topicResponse = "332 " + clients[fd].getNick() + " " + channelName + " :" + topic + "\r\n";
        clients[fd].response = topicResponse;
        clients[fd].sendResponse();
    }
}