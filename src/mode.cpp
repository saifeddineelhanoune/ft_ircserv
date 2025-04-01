#include "../include/server.hpp"

// Helper functions for channel modes
void Server::handleInviteOnlyMode(const std::string& channelName, bool addMode) {
    channels[channelName].setInviteOnly(addMode);
}

void Server::handleTopicRestrictionMode(const std::string& channelName, bool addMode) {
    channels[channelName].setTopicRestricted(addMode);
}

void Server::handleChannelKeyMode(int fd, const std::string& channelName, bool addMode, 
                                 std::vector<std::string>& args, int& argIndex, 
                                 std::string& modeParams) {
    if (addMode) {
        if (args.size() <= (size_t)argIndex) {
            sendError(fd, "461", "MODE", "Not enough parameters");
            return;
        }
        channels[channelName].setKey(args[argIndex]);
        modeParams += " " + args[argIndex];
        argIndex++;
    } else {
        channels[channelName].setKey("");
    }
}

bool Server::handleOperatorMode(int fd, const std::string& channelName, bool addMode, 
                               std::vector<std::string>& args, int& argIndex, 
                               std::string& modeParams) {
    if (args.size() <= (size_t)argIndex) {
        sendError(fd, "461", "MODE", "Not enough parameters");
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
    
    if (addMode) {
        channels[channelName].addOperator(targetFd);
    } else {
        channels[channelName].removeOperator(targetFd);
    }
    
    modeParams += " " + targetNick;
    argIndex++;
    return true;
}

bool Server::handleUserLimitMode(int fd, const std::string& channelName, bool addMode, 
                                std::vector<std::string>& args, int& argIndex, 
                                std::string& modeParams) {
    if (addMode) {
        if (args.size() <= (size_t)argIndex) {
            sendError(fd, "461", "MODE", "Not enough parameters");
            return false;
        }
        
        int limit = 0;
        try {
            limit = std::atoi(args[argIndex].c_str());
        } catch (...) {
            sendError(fd, "461", "MODE", "Invalid user limit");
            argIndex++;
            return false;
        }
        
        if (limit < 0) {
            sendError(fd, "461", "MODE", "Invalid user limit");
            argIndex++;
            return false;
        }
        
        channels[channelName].setUserLimit(limit);
        modeParams += " " + args[argIndex];
        argIndex++;
    } else {
        channels[channelName].setUserLimit(0);
    }
    return true;
}

// Function to display current channel modes
void Server::displayChannelModes(int fd, const std::string& channelName) {
    std::string modeString = "+";
    if (channels[channelName].isInviteOnly()) modeString += "i";
    if (channels[channelName].isTopicRestricted()) modeString += "t";
    if (!channels[channelName].getKey().empty()) modeString += "k";
    if (channels[channelName].getUserLimit() > 0) modeString += "l";
    
    clients[fd].response = ":" + std::string(serverName) + " 324 " + clients[fd].getNick() + " " + channelName + " " + modeString + "\r\n";
    clients[fd].sendResponse();
}

void Server::cmdMode(int fd, std::vector<std::string>& args) {
    if (args.size() < 2) {
        sendError(fd, "461", "MODE", "Not enough parameters");
        return;
    }
    
    if (!clients[fd].getAuth()) {
        sendError(fd, "464", "MODE", "You must authenticate first");
        return;
    }
    
    std::string target = args[1];
    
    // If it's a channel mode
    if (target[0] == '#') {
        // Just querying channel modes
        if (args.size() == 2) {
            if (channels.find(target) == channels.end()) {
                sendError(fd, "403", target, "No such channel");
                return;
            }
            
            displayChannelModes(fd, target);
            return;
        }
        
        // Setting channel modes
        if (channels.find(target) == channels.end()) {
            sendError(fd, "403", target, "No such channel");
            return;
        }
        
        if (!channels[target].hasUser(&clients[fd])) {
            sendError(fd, "442", target, "You're not on that channel");
            return;
        }
        
        if (!channels[target].isOperator(fd)) {
            sendError(fd, "482", target, "You're not channel operator");
            return;
        }
        
        processChannelModes(fd, target, args);
    } else {
        processUserModes(fd, target, args);
    }
}

void Server::processChannelModes(int fd, const std::string& target, std::vector<std::string>& args) {
    std::string modeString = args[2];
    bool addMode = true;
    if (modeString[0] == '+') {
        addMode = true;
        modeString = modeString.substr(1);
    } else if (modeString[0] == '-') {
        addMode = false;
        modeString = modeString.substr(1);
    }
    
    std::string modeChanges;
    std::string modeParams;
    int argIndex = 3;
    
    for (size_t i = 0; i < modeString.length(); ++i) {
        char mode = modeString[i];
        
        bool modeSuccess = false;
        
        switch (mode) {
            case 'i': { // Invite-only
                handleInviteOnlyMode(target, addMode);
                modeChanges += mode;
                modeSuccess = true;
                break;
            }
                
            case 't': { // Topic restriction
                handleTopicRestrictionMode(target, addMode);
                modeChanges += mode;
                modeSuccess = true;
                break;
            }
                
            case 'k': { // Channel key
                handleChannelKeyMode(fd, target, addMode, args, argIndex, modeParams);
                modeChanges += mode;
                modeSuccess = true;
                break;
            }
                
            case 'o': { // Operator status
                modeSuccess = handleOperatorMode(fd, target, addMode, args, argIndex, modeParams);
                if (modeSuccess)
                    modeChanges += mode;
                break;
            }
                
            case 'l': { // User limit
                modeSuccess = handleUserLimitMode(fd, target, addMode, args, argIndex, modeParams);
                if (modeSuccess)
                    modeChanges += mode;
                break;
            }
                
            default: {
                // Unsupported mode
                break;
            }
        }
    }
    
    if (!modeChanges.empty()) {
        std::string modeMsg = ":" + clients[fd].getNick() + "!" + clients[fd].getUser() + "@" + std::string(serverName) + " MODE " + target + " ";
        modeMsg += (addMode ? "+" : "-") + modeChanges + modeParams + "\r\n";
        broadcastToChannel(target, modeMsg, -1);
    }
}

void Server::processUserModes(int fd, const std::string& target, std::vector<std::string>& args) {
    // User mode (not required by the subject but good to have)
    if (clients[fd].getNick() != target) {
        sendError(fd, "502", "", "Cannot change mode for other users");
        return;
    }
    
    if (args.size() < 3) {
        // Just inform about current modes
        clients[fd].response = ":" + std::string(serverName) + " 221 " + clients[fd].getNick() + " +\r\n";
        clients[fd].sendResponse();
        return;
    }
    
    // For now, we don't implement user modes as they aren't required
    clients[fd].response = ":" + std::string(serverName) + " 221 " + clients[fd].getNick() + " +\r\n";
    clients[fd].sendResponse();
}