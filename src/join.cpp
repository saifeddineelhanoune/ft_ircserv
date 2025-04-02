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
    
    // Support for multiple channels with a single JOIN command
    std::vector<std::string> channelList;
    std::vector<std::string> keyList;
    
    // Split channels by comma
    std::string channelsStr = args[1];
    std::string::size_type startPos = 0, commaPos;
    while ((commaPos = channelsStr.find(',', startPos)) != std::string::npos) {
        channelList.push_back(channelsStr.substr(startPos, commaPos - startPos));
        startPos = commaPos + 1;
    }
    channelList.push_back(channelsStr.substr(startPos));
    
    // Split keys by comma if provided
    if (args.size() > 2) {
        std::string keysStr = args[2];
        startPos = 0;
        while ((commaPos = keysStr.find(',', startPos)) != std::string::npos) {
            keyList.push_back(keysStr.substr(startPos, commaPos - startPos));
            startPos = commaPos + 1;
        }
        keyList.push_back(keysStr.substr(startPos));
    }
    
    // Process each channel
    for (size_t i = 0; i < channelList.size(); i++) {
        std::string channelName = channelList[i];
        std::string key = (i < keyList.size()) ? keyList[i] : "";
        
        if (channelName[0] != '#') {
            channelName = "#" + channelName;
        }
        
        // Check if channel exists
        bool isNewChannel = channels.find(channelName) == channels.end();
        
        // Check channel key if needed
        if (!isNewChannel && !channels[channelName].getKey().empty()) {
            if (key != channels[channelName].getKey()) {
                sendError(fd, "475", channelName, "Cannot join channel (+k) - bad key");
                continue;
            }
        }
        
        // Check invite-only status
        if (!isNewChannel && channels[channelName].isInviteOnly()) {
            if (!channels[channelName].isInvited(fd)) {
                sendError(fd, "473", channelName, "Cannot join channel (+i) - you must be invited");
                continue;
            }
        }
        
        // Check user limit
        if (!isNewChannel && 
            channels[channelName].getUserLimit() > 0 && 
            channels[channelName].getUsers().size() >= (size_t)channels[channelName].getUserLimit()) {
            sendError(fd, "471", channelName, "Cannot join channel (+l) - channel is full");
            continue;
        }
        
        // Create channel if it doesn't exist
        if (isNewChannel) {
            Logger::channel("New channel created: " + channelName + " by " + clients[fd].getNick());
            Channel newChannel;
            channels[channelName] = newChannel;
        }
        Logger::channel("Adding Client " + clients[fd].getNick() + " To channel " + channelName);
        // Add user to channel
        channels[channelName].addUser(&clients[fd]);
        
        // Make the first user an operator
        if (isNewChannel || channels[channelName].getUsers().size() == 1) {
            channels[channelName].addOperator(fd);
        }
        
        // Send JOIN message to all users in the channel
        std::string response = ":" + clients[fd].getNick() + "!" + clients[fd].getUser() + "@" + std::string(serverName) + " JOIN :" + channelName + "\r\n";
        broadcastToChannel(channelName, response, -1);
        
        // Send channel topic
        std::string topic = channels[channelName].getTopic();
        if (!topic.empty()) {
            std::string topicResponse = ":" + std::string(serverName) + " 332 " + clients[fd].getNick() + " " + channelName + " :" + topic + "\r\n";
            clients[fd].response = topicResponse;
            clients[fd].sendResponse();
        }
        
        // Send names list (users in the channel)
        sendChannelNames(fd, channelName);
    }
}

// Helper method to send the list of users in a channel
void Server::sendChannelNames(int fd, const std::string& channelName) {
    if (channels.find(channelName) == channels.end()) {
        return;
    }
    
    std::string namesList;
    std::vector<Client*>& users = channels[channelName].getUsers();
    for (size_t i = 0; i < users.size(); i++) {
        if (channels[channelName].isOperator(users[i]->getFd())) {
            namesList += "@";
        }
        namesList += users[i]->getNick() + " ";
    }
    
    clients[fd].response = ":" + std::string(serverName) + " 353 " + clients[fd].getNick() + " = " + channelName + " :" + namesList + "\r\n";
    clients[fd].sendResponse();
    clients[fd].response = ":" + std::string(serverName) + " 366 " + clients[fd].getNick() + " " + channelName + " :End of /NAMES list.\r\n";
    clients[fd].sendResponse();
}