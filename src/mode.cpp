#include "../include/server.hpp"

void Server::cmdMode(int fd, std::vector<std::string>& args) {
    if (args.size() < 3) {
        sendError(fd, "461", "MODE", "Not enough parameters");
        return;
    }
    
    if (!clients[fd].getAuth()) {
        sendError(fd, "464", "MODE", "You must authenticate first");
        return;
    }
    
    std::string channelName = args[1];
    std::string modeString = args[2];
    
    // Check if channel exists
    if (channels.find(channelName) == channels.end()) {
        sendError(fd, "403", channelName, "No such channel");
        return;
    }
    
    // Check if user is in the channel
    if (!channels[channelName].hasUser(&clients[fd])) {
        sendError(fd, "442", channelName, "You're not on that channel");
        return;
    }
    
    // Check if user is channel operator
    if (!channels[channelName].isOperator(fd)) {
        sendError(fd, "482", channelName, "You're not channel operator");
        return;
    }
    
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
        
        // Variables needed for mode 'o'
        std::string targetNick;
        int targetFd = -1;
        std::vector<Client*>* usersPtr = NULL;
        std::vector<Client*>::iterator userIt;
        
        switch (mode) {
            case 'i': // Invite-only
                channels[channelName].setInviteOnly(addMode);
                modeChanges += mode;
                break;
                
            case 't': // Topic restriction
                channels[channelName].setTopicRestricted(addMode);
                modeChanges += mode;
                break;
                
            case 'k': // Channel key (password)
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
                modeChanges += mode;
                break;
                
            case 'o': // Operator status
                if (args.size() <= (size_t)argIndex) {
                    sendError(fd, "461", "MODE", "Not enough parameters");
                    return;
                }
                
                // Find target user
                targetNick = args[argIndex];
                targetFd = -1;
                usersPtr = &channels[channelName].getUsers();
                
                for (userIt = usersPtr->begin(); userIt != usersPtr->end(); ++userIt) {
                    if ((*userIt)->getNick() == targetNick) {
                        targetFd = (*userIt)->getFd();
                        break;
                    }
                }
                
                if (targetFd == -1) {
                    clients[fd].response = "441 " + targetNick + " " + channelName + " :They aren't on that channel\r\n";
                            clients[fd].sendResponse();
                    return;
                }
                
                if (addMode) {
                    channels[channelName].addOperator(targetFd);
                } else {
                    channels[channelName].removeOperator(targetFd);
                }
                
                modeChanges += mode;
                modeParams += " " + targetNick;
                argIndex++;
                break;
                
            case 'l': // User limit
                if (addMode) {
                    if (args.size() <= (size_t)argIndex) {
                        sendError(fd, "461", "MODE", "Not enough parameters");
                        return;
                    }
                    
                    // Convert limit to integer
                    int limit = 0;
                    std::istringstream iss(args[argIndex]);
                    if (!(iss >> limit) || limit < 0) {
                        sendError(fd, "461", "MODE", "Invalid user limit");
                        return;
                    }
                    
                    channels[channelName].setUserLimit(limit);
                    modeParams += " " + args[argIndex];
                    argIndex++;
                } else {
                    channels[channelName].setUserLimit(0);
                }
                modeChanges += mode;
                break;
                
            default:
                // Ignore unknown modes
                break;
        }
    }
    
    if (!modeChanges.empty()) {
        std::string modeMsg = ":" + clients[fd].getNick() + " MODE " + channelName + " ";
        modeMsg += (addMode ? "+" : "-") + modeChanges + modeParams + "\r\n";
        broadcastToChannel(channelName, modeMsg, -1);
    }
}