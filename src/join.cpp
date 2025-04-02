#include "../include/server.hpp"

struct IsSpace {
    bool operator()(unsigned char c) const {
        return std::isspace(c);
    }
};

// Process a single channel:key pair
void Server::processChannelKeyPair(const std::string& pair, 
                                 std::vector<std::string>& channelList,
                                 std::vector<std::string>& keyList) {
    std::string::size_type colonPos = pair.find(':');
    
    if (colonPos != std::string::npos) {
        // Channel with key (format: #channel:key)
        std::string channel = pair.substr(0, colonPos);
        std::string key = pair.substr(colonPos + 1);
        
        if (!channel.empty()) {
            channelList.push_back(channel);
            keyList.push_back(key);
        }
    } else {
        // Channel without key
        if (!pair.empty()) {
            channelList.push_back(pair);
        }
    }
}

void Server::parseChannelsAndKeys(std::vector<std::string>& args, 
                                 std::vector<std::string>& channelList,
                                 std::vector<std::string>& keyList) {
    // Get the channels argument
    std::string channelsArg = args[1];
    
    // Remove any whitespace
    channelsArg.erase(std::remove_if(channelsArg.begin(), channelsArg.end(), 
                                    IsSpace()), 
                     channelsArg.end());
    
    // Split by commas to get channel:key pairs
    std::string::size_type pos = 0;
    std::string::size_type lastPos = 0;
    while ((pos = channelsArg.find(',', lastPos)) != std::string::npos) {
        std::string pair = channelsArg.substr(lastPos, pos - lastPos);
        processChannelKeyPair(pair, channelList, keyList);
        lastPos = pos + 1;
    }
    
    // Process the last pair
    if (lastPos < channelsArg.length()) {
        std::string pair = channelsArg.substr(lastPos);
        processChannelKeyPair(pair, channelList, keyList);
    }
    
    // For backward compatibility - parse separate key parameter if provided and no keys found in channel format
    if (keyList.empty() && args.size() > 2) {
        std::string keysArg = args[2];
        keysArg.erase(std::remove_if(keysArg.begin(), keysArg.end(), IsSpace()), keysArg.end());
        
        pos = 0;
        lastPos = 0;
        while ((pos = keysArg.find(',', lastPos)) != std::string::npos) {
            keyList.push_back(keysArg.substr(lastPos, pos - lastPos));
            lastPos = pos + 1;
        }
        
        if (lastPos < keysArg.length()) {
            keyList.push_back(keysArg.substr(lastPos));
        }
    }
    
    // Debug log
    std::stringstream ss;
    ss << channelList.size();
    std::string sizeStr = ss.str();
    ss.str("");
    ss << keyList.size();
    std::string keySizeStr = ss.str();
    
    Logger::debug("Parsed channels: " + sizeStr + ", keys: " + keySizeStr);
    
    for (size_t i = 0; i < channelList.size(); i++) {
        std::string keyLog = (i < keyList.size()) ? keyList[i] : "no key";
        Logger::debug("Channel: " + channelList[i] + ", Key: " + keyLog);
    }
}

bool Server::checkChannelKey(int fd, const std::string& channelName, const std::string& providedKey, bool isNewChannel) {
    // For a new channel, any key is valid as it will be set
    if (isNewChannel) {
        Logger::debug("Creating new channel: " + channelName + " with key: " + 
                     (providedKey.empty() ? "none" : providedKey));
        return true;
    }
    
    // For existing channels, check if key matches
    if (channels[channelName].getKey().empty()) {
        // No key required
        Logger::debug("Channel " + channelName + " has no key requirement");
        return true;
    } else if (!providedKey.empty() && providedKey == channels[channelName].getKey()) {
        // Key matches
        Logger::debug("Channel " + channelName + " key matches");
        return true;
    } else {
        // Key doesn't match or wasn't provided
        Logger::warning("Invalid key for channel " + channelName + 
                       ": expected '" + channels[channelName].getKey() + 
                       "', got '" + providedKey + "'");
        sendError(fd, "475", channelName, "Cannot join channel (+k) - bad key");
        return false;
    }
}

bool Server::processChannel(int fd, const std::string& channelName, const std::string& key) {
    // Check if channel exists
    bool isNewChannel = channels.find(channelName) == channels.end();
    
    // Check if channel name is valid (starts with # or &)
    if (channelName.empty() || (channelName[0] != '#' && channelName[0] != '&')) {
        sendError(fd, "403", channelName, "No such channel");
        return false;
    }
    
    // Validate key for existing channels
    if (!checkChannelKey(fd, channelName, key, isNewChannel)) {
        return false;
    }
    
    // Create new channel if it doesn't exist
    if (isNewChannel) {
        channels[channelName] = Channel();
        if (!key.empty()) {
            channels[channelName].setKey(key);
        }
    } else {
        // Check if channel is invite-only
        if (channels[channelName].isInviteOnly() && !channels[channelName].isInvited(fd)) {
            sendError(fd, "473", channelName, "Cannot join channel (+i) - you must be invited");
            return false;
        }
        
        // Check if channel has user limit
        if (channels[channelName].getUserLimit() > 0 && 
            channels[channelName].getUsers().size() >= (size_t)channels[channelName].getUserLimit()) {
            sendError(fd, "471", channelName, "Cannot join channel (+l) - channel is full");
            return false;
        }
    }
    
    // Remove client from invite list if they were invited
    if (channels[channelName].isInvited(fd)) {
        channels[channelName].removeInvitedUser(fd);
    }
    
    // Add client to channel
    channels[channelName].addUser(&clients[fd]);
    
    // If this is a new channel, make the first user an operator
    if (isNewChannel) {
        channels[channelName].addOperator(fd);
    }
    
    // Send JOIN message to all users in channel
    std::string joinMsg = ":" + clients[fd].getNick() + " JOIN " + channelName + "\r\n";
    broadcastToChannel(channelName, joinMsg, -1);
    
    // Send topic if exists
    std::string topic = channels[channelName].getTopic();
    if (!topic.empty()) {
        clients[fd].response = ":" + serverName + " 332 " + clients[fd].getNick() + " " + channelName + " :" + topic + "\r\n";
        clients[fd].sendResponse();
    }
    
    // Send names list
    sendChannelNames(fd, channelName);
    
    return true;
}

void Server::sendChannelNames(int fd, const std::string& channelName) {
    std::string nameList = "";
    std::vector<Client*>& users = channels[channelName].getUsers();
    
    for (size_t i = 0; i < users.size(); i++) {
        if (channels[channelName].isOperator(users[i]->getFd())) {
            nameList += "@" + users[i]->getNick();
        } else {
            nameList += users[i]->getNick();
        }
        Logger::channel("Name " + nameList);
        if (i < users.size() - 1) {
            nameList += " ";
        }
    }
    Logger::channel("Name list " + nameList);
    clients[fd].response = ":" + serverName + " 353 " + clients[fd].getNick() + " = " + channelName + " :" + nameList + "\r\n";
    clients[fd].sendResponse();
    Logger::channel("End of names");
    clients[fd].response = ":" + serverName + " 366 " + clients[fd].getNick() + " " + channelName + " :End of /NAMES list\r\n";
    clients[fd].sendResponse();
}

void Server::cmdJoin(int fd, std::vector<std::string>& args) {
    if (args.size() < 2) {
        sendError(fd, "461", "JOIN", "Not enough parameters");
        return;
    }
    Logger::client("Client " + clients[fd].getNick() + " is joining channel " + args[1]);
    // Special case: JOIN 0 means leave all channels
    if (args[1] == "0") {
        std::map<std::string, Channel>::iterator it;
        std::vector<std::string> channelsToLeave;
        Logger::client("Client " + clients[fd].getNick() + " is leaving all channels");
        Logger::info("Leaving all channels");
        for (it = channels.begin(); it != channels.end(); ++it) {
            if (it->second.hasUser(&clients[fd])) {
                channelsToLeave.push_back(it->first);
            }
        }
        
        // Then leave each channel
        for (size_t i = 0; i < channelsToLeave.size(); i++) {
            std::string partMsg = ":" + clients[fd].getNick() + " PART " + channelsToLeave[i] + " :Leaving all channels\r\n";
            broadcastToChannel(channelsToLeave[i], partMsg, -1);
            channels[channelsToLeave[i]].removeUser(&clients[fd]);
            Logger::channel("Channel " + channelsToLeave[i] + " removed user " + clients[fd].getNick());
            if (channels[channelsToLeave[i]].isEmpty()) {
                channels.erase(channelsToLeave[i]);
            }
        }
        return;
    }
    Logger::info("Joining channel " + args[1]);
    std::vector<std::string> channelList;
    std::vector<std::string> keyList;
    parseChannelsAndKeys(args, channelList, keyList);
    Logger::channel("Channel list " + channelList[0]);
    for (size_t i = 0; i < channelList.size(); i++) {
        std::string key = (i < keyList.size()) ? keyList[i] : "";
        processChannel(fd, channelList[i], key);
    }
}
