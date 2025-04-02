#include "../include/server.hpp"

void Server::sendChannelNames(int fd, const std::string& channelName) {
    // Get the list of users in the channel
    std::vector<Client*>& users = this->channels[channelName].getUsers();
    std::string namesList = "";
    
    // Build the list of names with appropriate prefixes
    for (std::vector<Client*>::iterator it = users.begin(); it != users.end(); ++it) {
        // Add @ prefix for channel operators
        if (this->channels[channelName].isOperator((*it)->getFd())) {
            namesList += "@";
        }
        namesList += (*it)->getNick();
        
        // Add space between names unless it's the last name
        std::vector<Client*>::iterator next = it;
        next++;
        if (next != users.end()) {
            namesList += " ";
        }
    }
    
    // Send RPL_NAMREPLY (353)
    std::string namreply = ":" + serverName + " 353 " + clients[fd].getNick() + " = " + 
                          channelName + " :" + namesList + "\r\n";
    clients[fd].response = namreply;
    clients[fd].sendResponse();
    
    // Send RPL_ENDOFNAMES (366)
    std::string endofnames = ":" + serverName + " 366 " + clients[fd].getNick() + " " + 
                            channelName + " :End of /NAMES list\r\n";
    clients[fd].response = endofnames;
    clients[fd].sendResponse();
}

void Server::cmdJoin(int fd, std::vector<std::string>& args) {
    if (args.size() < 2) {
        sendError(fd, "461", "JOIN", "Not enough parameters");
        return;
    }
    
    if (!clients[fd].getAuth()) {
        sendError(fd, "451", "JOIN", "You have not registered");
        return;
    }
    
    // Split channel list and key list
    std::vector<std::string> channelList;
    std::vector<std::string> keyList;
    
    // Parse channel names (comma-separated)
    std::string channels_str = args[1];
    std::string::size_type pos = 0, lastPos = 0;
    while ((pos = channels_str.find(",", lastPos)) != std::string::npos) {
        channelList.push_back(channels_str.substr(lastPos, pos - lastPos));
        lastPos = pos + 1;
    }
    channelList.push_back(channels_str.substr(lastPos));
    
    // Parse keys (comma-separated)
    if (args.size() > 2) {
        std::string keys = args[2];
        pos = 0, lastPos = 0;
        while ((pos = keys.find(",", lastPos)) != std::string::npos) {
            keyList.push_back(keys.substr(lastPos, pos - lastPos));
            lastPos = pos + 1;
        }
        keyList.push_back(keys.substr(lastPos));
    }
    
    for (size_t i = 0; i < channelList.size(); i++) {
        std::string channelName = channelList[i];
        std::string key = (i < keyList.size()) ? keyList[i] : "";
        
        // Ensure channel name starts with #
        if (channelName[0] != '#') {
            channelName = "#" + channelName;
        }
        
        // Check if channel exists
        bool isNewChannel = (this->channels.find(channelName) == this->channels.end());
        
        // Check channel key if needed
        if (!isNewChannel && !this->channels[channelName].getKey().empty()) {
            if (key != this->channels[channelName].getKey()) {
                sendError(fd, "475", channelName, "Cannot join channel (+k) - bad key");
                continue;
            }
        }
        
        // Check invite-only status
        if (!isNewChannel && this->channels[channelName].isInviteOnly()) {
            if (!this->channels[channelName].isInvited(fd)) {
                sendError(fd, "473", channelName, "Cannot join channel (+i) - you must be invited");
                continue;
            }
            // Clear invite status once used
            this->channels[channelName].removeInvitedUser(fd);
        }
        
        // Check user limit
        if (!isNewChannel && 
            this->channels[channelName].getUserLimit() > 0 && 
            this->channels[channelName].getUsers().size() >= (size_t)this->channels[channelName].getUserLimit()) {
            sendError(fd, "471", channelName, "Cannot join channel (+l) - channel is full");
            continue;
        }
        
        // Create channel if it doesn't exist
        if (isNewChannel) {
            Logger::channel("New channel created: " + channelName + " by " + clients[fd].getNick());
            Channel newChannel;
            this->channels[channelName] = newChannel;
        }
        
        // If user is already in the channel, don't add them again
        if (this->channels[channelName].hasUser(&clients[fd])) {
            Logger::warning("User " + clients[fd].getNick() + " tried to join " + channelName + " but is already in the channel");
            continue;
        }
        
        Logger::channel("Adding Client " + clients[fd].getNick() + " To channel " + channelName);
        
        // Add user to channel
        this->channels[channelName].addUser(&clients[fd]);
        
        // Make the first user an operator
        if (isNewChannel) {
            this->channels[channelName].addOperator(fd);
            Logger::channel("User " + clients[fd].getNick() + " is now an operator of " + channelName);
        }
        
        // Send JOIN message to all users in the channel
        std::string response = ":" + clients[fd].getNick() + "!" + clients[fd].getUser() + "@" + serverName + " JOIN :" + channelName + "\r\n";
        broadcastToChannel(channelName, response, -1);
        
        // Send channel topic
        std::string topic = this->channels[channelName].getTopic();
        if (!topic.empty()) {
            std::string topicResponse = ":" + serverName + " 332 " + clients[fd].getNick() + " " + channelName + " :" + topic + "\r\n";
            clients[fd].response = topicResponse;
            clients[fd].sendResponse();
        } else {
            // Send RPL_NOTOPIC if no topic is set (helpful for clients)
            std::string noTopicResponse = ":" + serverName + " 331 " + clients[fd].getNick() + " " + channelName + " :No topic is set\r\n";
            clients[fd].response = noTopicResponse;
            clients[fd].sendResponse();
        }
        
        // Send names list (users in the channel)
        sendChannelNames(fd, channelName);
    }
}

// // Helper method to send the list of users in a channel
// void Server::sendChannelNames(int fd, const std::string& channelName) {
//     if (channels.find(channelName) == channels.end()) {
//         return;
//     }
    
//     std::string namesList;
//     std::vector<Client*>& users = channels[channelName].getUsers();
//     for (size_t i = 0; i < users.size(); i++) {
//         if (channels[channelName].isOperator(users[i]->getFd())) {
//             namesList += "@";
//         }
//         namesList += users[i]->getNick() + " ";
//     }
    
//     clients[fd].response = ":" + std::string(serverName) + " 353 " + clients[fd].getNick() + " = " + channelName + " :" + namesList + "\r\n";
//     clients[fd].sendResponse();
//     clients[fd].response = ":" + std::string(serverName) + " 366 " + clients[fd].getNick() + " " + channelName + " :End of /NAMES list.\r\n";
//     clients[fd].sendResponse();
// }

