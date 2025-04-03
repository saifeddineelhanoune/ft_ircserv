#include "../include/server.hpp"

void Server::cmdMode(int fd, std::vector<std::string>& args) {
    if (args.size() < 2) {
        sendError(fd, "461", "MODE", "Not enough parameters");
        return;
    }
    
    if (!clients[fd].getAuth()) {
        sendError(fd, "451", "MODE", "You have not registered");
        return;
    }
    
    std::string target = args[1];
    
    // Check if mode is for a channel or user
    if (target[0] == '#') {
        // Channel mode
        if (channels.find(target) == channels.end()) {
            sendError(fd, "403", target, "No such channel");
            return;
        }
        
        // If no mode specified, just display current modes
        if (args.size() == 2) {
            displayChannelModes(fd, target,1);
            return;
        }
        
        // Check if user is a channel operator
        if (!channels[target].isOperator(fd)) {
            sendError(fd, "482", target, "You're not channel operator");
            return;
        }
        
        processChannelModes(fd, target, args);
    } else {
        // User mode - only allow users to set modes for themselves
        if (target != clients[fd].getNick()) {
            sendError(fd, "502", "", "Cannot change mode for other users");
            return;
        }
        
        processUserModes(fd, target, args);
    }
}

void Server::processChannelModes(int fd, const std::string& target, std::vector<std::string>& args) {
    std::string modeString = args[2];
    bool addMode = true;
    
    // Check if mode string starts with + or -
    if (modeString[0] == '+') {
        addMode = true;
        modeString = modeString.substr(1);
    } else if (modeString[0] == '-') {
        addMode = false;
        modeString = modeString.substr(1);
    }
    
    std::string modeChanges;
    std::string modeParams;
    int argIndex = 3; // Start looking for mode parameters at index 3
    bool modeChanged = false;
    
    for (size_t i = 0; i < modeString.length(); ++i) {
        char mode = modeString[i];
        bool modeSuccess = false;
        
        switch (mode) {
            case 'i': // Invite-only
                handleInviteOnlyMode(target, addMode);
                modeChanges += mode;
                modeSuccess = true;
                modeChanged = true;
                break;
                
            case 't': // Topic restriction
                handleTopicRestrictionMode(target, addMode);
                modeChanges += mode;
                modeSuccess = true;
                modeChanged = true;
                break;
                
            case 'k': // Channel key
                handleChannelKeyMode(fd, target, addMode, args, argIndex, modeParams, &modeSuccess);
                if (modeSuccess) {
                    modeChanges += mode;
                    modeChanged = true;
                }
                break;
                
            case 'o': // Operator status
                modeSuccess = handleOperatorMode(fd, target, addMode, args, argIndex, modeParams);
                if (modeSuccess) {
                    modeChanges += mode;
                    modeChanged = true;
                }
                break;
                
            case 'l': // User limit
                modeSuccess = handleUserLimitMode(fd, target, addMode, args, argIndex, modeParams);
                if (modeSuccess) {
                    modeChanges += mode;
                    modeChanged = true;
                }
                break;
                
            default:
                // Inform about unsupported mode
                sendError(fd, "472", std::string(1, mode), "is unknown mode char to me");
                break;
        }
    }
    
    // Only broadcast if mode changes were successful
    if (modeChanged && !modeChanges.empty()) {
        // Format according to RFC 1459/2812
        std::string modeMsg = ":" + clients[fd].getNick() + "!" + clients[fd].getUser() + "@" + serverName + 
                             " MODE " + target + " " + (addMode ? "+" : "-") + modeChanges;
        
        if (!modeParams.empty()) {
            modeMsg += modeParams;
        }
        
        modeMsg += "\r\n";
        
        // Send to all channel users including the sender
        broadcastToChannel(target, modeMsg, -1);
        
        Logger::channel("Mode changed for " + target + ": " + (addMode ? "+" : "-") + modeChanges);
    }
}



bool Server::handleUserLimitMode(int fd, const std::string& channelName, bool addMode, 
                                std::vector<std::string>& args, int& argIndex, 
                                std::string& modeParams) {
    if (addMode) {
        if (args.size() <= (size_t)argIndex) {
            sendError(fd, "461", "MODE +l", "Not enough parameters");
            return false;
        }
        
        std::string limitStr = args[argIndex];
        if (!isStringDigits(limitStr)) {
            sendError(fd, "461", "MODE +l", "User limit must be a number");
            argIndex++;
            return false;
        }
        
        int limit = atoi(limitStr.c_str());
        channels[channelName].setUserLimit(limit);
        modeParams += " " + limitStr;
        argIndex++;
    } else {
        // Remove user limit
        channels[channelName].setUserLimit(0);
    }
    
    return true;
}


void Server::handleTopicRestrictionMode(const std::string& channelName, bool addMode) {
    channels[channelName].setTopicRestricted(addMode);
}

void Server::displayChannelModes(int fd, const std::string& channelName,bool wlcMode) {
    std::string modes = "+";
    std::string params = "";
    
    // Add current modes to the string
    if (channels[channelName].isInviteOnly()) {
        modes += "i";
    }
    
    if (channels[channelName].isTopicRestricted()) {
        modes += "t";
    }
    
    if (!channels[channelName].getKey().empty()) {
        modes += "k";
        params += " " + channels[channelName].getKey();
    }
    
    if (channels[channelName].getUserLimit() > 0) {
        modes += "l";
        std::stringstream ss;
        ss << channels[channelName].getUserLimit();
        params += " " + ss.str();
    }
    
    // Send mode information to user
    std::string response = ":" + serverName + " 324 " + clients[fd].getNick() + " " + 
                          channelName + " " + modes + params + "\r\n";
    clients[fd].response = response;
    clients[fd].sendResponse();
    
    // Send channel creation time (required by many clients)
    if (wlcMode)
    {
        clients[fd].response = ":" + serverName + " 329 " + clients[fd].getNick() + " " + 
                             channelName + " " + "1234567890" + "\r\n"; // Use actual timestamp if available
        clients[fd].sendResponse();
    }
}

bool Server::handleOperatorMode(int fd, const std::string& channelName, bool addMode, 
                               std::vector<std::string>& args, int& argIndex, 
                               std::string& modeParams) {
    if (args.size() <= (size_t)argIndex) {
        sendError(fd, "461", "MODE +o/-o", "Not enough parameters");
        return false;
    }
    
    // Find target user
    std::string targetNick = args[argIndex];
    int targetFd = -1;
    std::vector<Client*>& usersVec = channels[channelName].getUsers();
    
    for (size_t j = 0; j < usersVec.size(); j++) {
        if (usersVec[j]->getNick() == targetNick) {
            targetFd = usersVec[j]->getFd();
            break;
        }
    }
    
    if (targetFd == -1) {
        sendError(fd, "441", targetNick, "They aren't on that channel");
        argIndex++;
        return false;
    }
    
    // Check if already operator when adding or not operator when removing
    if (addMode && channels[channelName].isOperator(targetFd)) {
        Logger::warning(targetNick + " is already an operator in " + channelName);
        argIndex++;
        return false;
    } else if (!addMode && !channels[channelName].isOperator(targetFd)) {
        Logger::warning(targetNick + " is not an operator in " + channelName);
        argIndex++;
        return false;
    }
    
    if (addMode) {
        channels[channelName].addOperator(targetFd);
        Logger::channel("User " + targetNick + " is now an operator of " + channelName);
    } else {
        channels[channelName].removeOperator(targetFd);
        Logger::channel("User " + targetNick + " is no longer an operator of " + channelName);
    }
    
    modeParams += " " + targetNick;
    argIndex++;
    return true;
}


void Server::handleChannelKeyMode(int fd, const std::string& channelName, bool addMode, 
                                 std::vector<std::string>& args, int& argIndex, 
                                 std::string& modeParams, bool* modeSuccess) {
    if (addMode) {
        if (args.size() <= (size_t)argIndex) {
            sendError(fd, "461", "MODE +k", "Not enough parameters");
            *modeSuccess = false;
            return;
        }
        
        std::string key = args[argIndex];
        channels[channelName].setKey(key);
        modeParams += " " + key;
        argIndex++;
        *modeSuccess = true;
    } else {
        // For -k, we don't require a parameter, but accept it if provided
        channels[channelName].setKey("");
        if (args.size() > (size_t)argIndex) {
            argIndex++;
        }
        *modeSuccess = true;
    }
}

void Server::handleInviteOnlyMode(const std::string& channelName, bool addMode) {
    channels[channelName].setInviteOnly(addMode);
}


void Server::processUserModes(int fd, const std::string& target, std::vector<std::string>& args) {
    (void) target;
    if (args.size() < 3) {
        // Just inform about current modes (most IRC clients expect this)
        clients[fd].response = ":" + serverName + " 221 " + clients[fd].getNick() + " +\r\n";
        clients[fd].sendResponse();
        return;
    }
    
    // For now, we don't implement user modes as they aren't required
    clients[fd].response = ":" + serverName + " 221 " + clients[fd].getNick() + " +\r\n";
    clients[fd].sendResponse();
}