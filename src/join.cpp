#include "../include/server.hpp"

void Server::cmdJoin(int fd, std::vector<std::string>& args) {
    if (args.size() < 2) {
        clients[fd].response = "461 * JOIN :Not enough parameters\r\n";
        return;
    }
    
    if (!clients[fd].getAuth()) {
        clients[fd].response = "464 * :You must authenticate first\r\n";
        return;
    }
    
    std::string channelName = args[1];
    if (channelName[0] != '#') {
        channelName = "#" + channelName;
    }
    
    // Check if channel has a key and if the provided key is correct
    if (channels.find(channelName) != channels.end() && !channels[channelName].getKey().empty()) {
        if (args.size() < 3 || args[2] != channels[channelName].getKey()) {
            clients[fd].response = "475 " + channelName + " :Cannot join channel (+k) - bad key\r\n";
            return;
        }
    }
    
    // Check if channel is invite-only and user is invited
    if (channels.find(channelName) != channels.end() && channels[channelName].isInviteOnly()) {
        if (!channels[channelName].isInvited(fd)) {
            clients[fd].response = "473 " + channelName + " :Cannot join channel (+i) - you must be invited\r\n";
            return;
        }
    }
    
    // Check user limit
    if (channels.find(channelName) != channels.end() && 
        channels[channelName].getUserLimit() > 0 && 
        channels[channelName].getUsers().size() >= (size_t)channels[channelName].getUserLimit()) {
        clients[fd].response = "471 " + channelName + " :Cannot join channel (+l) - channel is full\r\n";
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
    }
}