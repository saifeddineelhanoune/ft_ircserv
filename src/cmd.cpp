#include "../include/server.hpp"
#include <netdb.h>
#include <sstream>

void Server::cmdNick(int fd, std::vector<std::string>& args) {
    if (args.size() < 2) {
        Logger::warning(fd, "NICK command received with insufficient parameters");
        clients[fd].response = "461 * NICK :Not enough parameters\r\n";
        return;
    }
    
    std::string oldNick = clients[fd].getNick();
    std::string newNick = args[1];
    
    // Check if nickname is already in use
    std::map<int, Client>::iterator it;
    for (it = clients.begin(); it != clients.end(); ++it) {
        if (it->second.getNick() == newNick) {
            Logger::warning(fd, "Nickname '" + newNick + "' is already in use");
            clients[fd].response = "433 * " + newNick + " :Nickname is already in use\r\n";
            return;
        }
    }
    
    clients[fd].setNick(newNick);
    clients[fd].response = ":" + oldNick + " NICK :" + newNick + "\r\n";
    Logger::info(fd, "Nickname set: " + newNick);
}

void Server::cmdUser(int fd, std::vector<std::string>& args) {
    if (args.size() < 5) {
        clients[fd].response = "461 * USER :Not enough parameters\r\n";
        return;
    }
    
    clients[fd].setUser(args[1]);
    if (clients[fd].getNick() != "" && !clients[fd].getAuth()) {
        std::string response = "001 " + clients[fd].getNick() + " :Welcome to the IRC server\r\n";
        clients[fd].response = response;
    }
}

void Server::cmdPass(int fd, std::vector<std::string>& args) {
    if (args.size() < 2) {
        clients[fd].response = "461 * PASS :Not enough parameters\r\n";
        return;
    }
    
    if (args[1] == data.passwd) {
        clients[fd].setAuth(true);
        clients[fd].response = "001 " + clients[fd].getNick() + " :Welcome to the IRC server\r\n";
    } else {
        clients[fd].response = "464 * :Password incorrect\r\n";
        // Consider disconnecting the client after multiple failed attempts
    }
}

void Server::cmdJoin(int fd, std::vector<std::string>& args) {
    if (args.size() < 2) {
        Logger::warning(fd, "JOIN command received with insufficient parameters");
        clients[fd].response = "461 * JOIN :Not enough parameters\r\n";
        return;
    }
    
    if (!clients[fd].getAuth()) {
        Logger::warning(fd, "Unauthenticated client attempted to JOIN channel");
        clients[fd].response = "464 * :You must authenticate first\r\n";
        return;
    }
    
    std::string channelName = args[1];
    if (channelName[0] != '#') {
        channelName = "#" + channelName;
    }
    
    Logger::debug(fd, "Processing JOIN request for channel: " + channelName);
    
    // Check if channel has a key and if the provided key is correct
    if (channels.find(channelName) != channels.end() && !channels[channelName].getKey().empty()) {
        if (args.size() < 3 || args[2] != channels[channelName].getKey()) {
            Logger::warning(fd, "Incorrect key provided for channel: " + channelName);
            clients[fd].response = "475 " + channelName + " :Cannot join channel (+k) - bad key\r\n";
            return;
        }
    }
    
    // Check if channel is invite-only and user is invited
    if (channels.find(channelName) != channels.end() && channels[channelName].isInviteOnly()) {
        if (!channels[channelName].isInvited(fd)) {
            Logger::warning(fd, "Client attempted to join invite-only channel without invitation: " + channelName);
            clients[fd].response = "473 " + channelName + " :Cannot join channel (+i) - you must be invited\r\n";
            return;
        }
    }
    
    // Check user limit
    if (channels.find(channelName) != channels.end() && 
        channels[channelName].getUserLimit() > 0 && 
        channels[channelName].getUsers().size() >= (size_t)channels[channelName].getUserLimit()) {
        Logger::warning(fd, "Client attempted to join full channel: " + channelName);
        clients[fd].response = "471 " + channelName + " :Cannot join channel (+l) - channel is full\r\n";
        return;
    }
    
    bool isNewChannel = channels.find(channelName) == channels.end();
    
    // Create channel if it doesn't exist
    if (isNewChannel) {
        Logger::info("Creating new channel: " + channelName);
        Channel newChannel;
        channels[channelName] = newChannel;
    }
    
    channels[channelName].addUser(&clients[fd]);
    
    // Make the first user who joins a channel an operator
    if (isNewChannel || channels[channelName].getUsers().size() == 1) {
        channels[channelName].addOperator(fd);
        Logger::info(fd, "User granted operator status on channel: " + channelName);
    }
    
    std::string nick = clients[fd].getNick();
    std::string response = ":" + nick + " JOIN " + channelName + "\r\n";
    broadcastToChannel(channelName, response, -1);
    
    Logger::info(fd, "User " + nick + " joined channel: " + channelName);
    
    // Send channel topic to the user
    std::string topic = channels[channelName].getTopic();
    if (!topic.empty()) {
        std::string topicResponse = "332 " + nick + " " + channelName + " :" + topic + "\r\n";
        clients[fd].response = topicResponse;
        Logger::debug(fd, "Sent topic to user: " + topic);
    }
}

void Server::cmdPrivmsg(int fd, std::vector<std::string>& args) {
    if (args.size() < 3) {
        clients[fd].response = "461 * PRIVMSG :Not enough parameters\r\n";
        return;
    }
    
    if (!clients[fd].getAuth()) {
        clients[fd].response = "464 * :You must authenticate first\r\n";
        return;
    }
    
    std::string target = args[1];
    std::string message = args[2];
    
    if (target[0] == '#') {
        // Channel message
        if (channels.find(target) != channels.end()) {
            std::string response = ":" + clients[fd].getNick() + " PRIVMSG " + target + " :" + message + "\r\n";
            broadcastToChannel(target, response, fd);
        } else {
            clients[fd].response = "403 " + target + " :No such channel\r\n";
        }
    } else {
        // Private message
        bool found = false;
        std::map<int, Client>::iterator it;
        for (it = clients.begin(); it != clients.end(); ++it) {
            if (it->second.getNick() == target) {
                std::string response = ":" + clients[fd].getNick() + " PRIVMSG " + target + " :" + message + "\r\n";
                it->second.response = response;
                found = true;
                break;
            }
        }
        if (!found) {
            clients[fd].response = "401 " + target + " :No such nick\r\n";
        }
    }
}

void Server::cmdPart(int fd, std::vector<std::string>& args) {
    if (args.size() < 2) {
        clients[fd].response = "461 * PART :Not enough parameters\r\n";
        return;
    }
    
    std::string channelName = args[1];
    if (channels.find(channelName) == channels.end()) {
        clients[fd].response = "403 " + channelName + " :No such channel\r\n";
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

void Server::cmdQuit(int fd, std::vector<std::string>& args) {
    std::string quitMessage = args.size() > 1 ? args[1] : "Leaving";
    std::string response = ":" + clients[fd].getNick() + " QUIT :Quit: " + quitMessage + "\r\n";
    
    // Notify all channels this user was in
    std::map<std::string, Channel>::iterator it;
    for (it = channels.begin(); it != channels.end(); ++it) {
        if (it->second.hasUser(&clients[fd])) {
            broadcastToChannel(it->first, response, fd);
            it->second.removeUser(&clients[fd]);
        }
    }
    
    // Clean up empty channels
    it = channels.begin();
    while (it != channels.end()) {
        if (it->second.isEmpty()) {
            channels.erase(it++);
        } else {
            ++it;
        }
    }
    
    clients.erase(fd);
    close(fd);
}

void Server::welcomeClient()
{
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    
    int fd_c = accept(data.socket, (struct sockaddr*)&client_addr, &client_len);
    
    if (fd_c < 0)
    { 
        Logger::error("Failed to accept client connection");
        return;
    }
    
    std::string ip = inet_ntoa(client_addr.sin_addr);
    int port = ntohs(client_addr.sin_port);
    
    std::ostringstream fdStr, portStr;
    fdStr << fd_c;
    portStr << port;
    
    Logger::info("New client connection accepted: FD=" + fdStr.str() + 
                 " IP=" + ip + " Port=" + portStr.str());
    
    clients[fd_c] = Client(fd_c, client_addr);
    sockaddr_in addr = clients[fd_c].getAddr();

    char host[NI_MAXHOST];
    char service[NI_MAXSERV];
    getnameinfo((struct sockaddr*)&addr, sizeof(client_addr), 
                host, NI_MAXHOST, service, NI_MAXSERV, 
                NI_NUMERICHOST | NI_NUMERICSERV);

    struct pollfd _fd;
    _fd.fd = fd_c;
    _fd.events = POLLIN;
    pollfds.push_back(_fd);
    
    std::ostringstream fdStr2;
    fdStr2 << fd_c;
    Logger::debug("Client added to poll list: FD=" + fdStr2.str());
}

void Server::broadcastToChannel(const std::string& channel, const std::string& message, int excludeFd) {
    std::vector<Client*>& users = channels[channel].getUsers();
    std::vector<Client*>::iterator it;
    for (it = users.begin(); it != users.end(); ++it) {
        if ((*it)->getFd() != excludeFd) {
            (*it)->response = message;
        }
    }
}

void Server::cmdKick(int fd, std::vector<std::string>& args) {
    if (args.size() < 3) {
        Logger::warning(fd, "KICK command received with insufficient parameters");
        clients[fd].response = "461 * KICK :Not enough parameters\r\n";
        return;
    }
    
    if (!clients[fd].getAuth()) {
        Logger::warning(fd, "Unauthenticated client attempted to use KICK command");
        clients[fd].response = "464 * :You must authenticate first\r\n";
        return;
    }
    
    std::string channelName = args[1];
    std::string targetNick = args[2];
    std::string reason = args.size() > 3 ? args[3] : "No reason specified";
    
    Logger::debug(fd, "Processing KICK request: channel=" + channelName + 
                  " target=" + targetNick + " reason=" + reason);
    
    // Check if channel exists
    if (channels.find(channelName) == channels.end()) {
        Logger::warning(fd, "Attempted to KICK from non-existent channel: " + channelName);
        clients[fd].response = "403 " + channelName + " :No such channel\r\n";
        return;
    }
    
    // Check if user is an operator
    if (!channels[channelName].isOperator(fd)) {
        Logger::warning(fd, "Non-operator attempted to use KICK command on channel: " + channelName);
        clients[fd].response = "482 " + channelName + " :You're not channel operator\r\n";
        return;
    }
    
    // Find the target user
    int targetFd = -1;
    std::vector<Client*>& users = channels[channelName].getUsers();
    std::vector<Client*>::iterator it;
    for (it = users.begin(); it != users.end(); ++it) {
        if ((*it)->getNick() == targetNick) {
            targetFd = (*it)->getFd();
            break;
        }
    }
    
    if (targetFd == -1) {
        Logger::warning(fd, "Attempted to KICK non-existent user from channel: " + 
                        targetNick + " from " + channelName);
        clients[fd].response = "441 " + targetNick + " " + channelName + " :They aren't on that channel\r\n";
        return;
    }
    
    // Broadcast kick message to channel
    std::string kickerNick = clients[fd].getNick();
    std::string kickMsg = ":" + kickerNick + " KICK " + channelName + " " + targetNick + " :" + reason + "\r\n";
    broadcastToChannel(channelName, kickMsg, -1);
    
    // Remove the user from the channel
    channels[channelName].removeUser(&clients[targetFd]);
    
    Logger::info(fd, "User " + kickerNick + " kicked " + targetNick + 
                 " from channel " + channelName + " (Reason: " + reason + ")");
}

void Server::cmdInvite(int fd, std::vector<std::string>& args) {
    if (args.size() < 3) {
        clients[fd].response = "461 * INVITE :Not enough parameters\r\n";
        return;
    }
    
    if (!clients[fd].getAuth()) {
        clients[fd].response = "464 * :You must authenticate first\r\n";
        return;
    }
    
    std::string targetNick = args[1];
    std::string channelName = args[2];
    
    // Check if channel exists
    if (channels.find(channelName) == channels.end()) {
        clients[fd].response = "403 " + channelName + " :No such channel\r\n";
        return;
    }
    
    // Check if inviteOnly mode is set and user is not an operator
    if (channels[channelName].isInviteOnly() && !channels[channelName].isOperator(fd)) {
        clients[fd].response = "482 " + channelName + " :You're not channel operator\r\n";
        return;
    }
    
    // Find the target user
    int targetFd = -1;
    std::map<int, Client>::iterator it;
    for (it = clients.begin(); it != clients.end(); ++it) {
        if (it->second.getNick() == targetNick) {
            targetFd = it->first;
            break;
        }
    }
    
    if (targetFd == -1) {
        clients[fd].response = "401 " + targetNick + " :No such nick\r\n";
        return;
    }
    
    // Check if user is already in the channel
    if (channels[channelName].hasUser(&clients[targetFd])) {
        clients[fd].response = "443 " + targetNick + " " + channelName + " :is already on channel\r\n";
        return;
    }
    
    // Add user to invited list
    channels[channelName].addInvitedUser(targetFd);
    
    // Send invite notification to target user
    std::string inviteMsg = ":" + clients[fd].getNick() + " INVITE " + targetNick + " :" + channelName + "\r\n";
    clients[targetFd].response = inviteMsg;
    
    // Confirm to inviter
    clients[fd].response = "341 " + clients[fd].getNick() + " " + targetNick + " " + channelName + "\r\n";
}

void Server::cmdTopic(int fd, std::vector<std::string>& args) {
    if (args.size() < 2) {
        clients[fd].response = "461 * TOPIC :Not enough parameters\r\n";
        return;
    }
    
    if (!clients[fd].getAuth()) {
        clients[fd].response = "464 * :You must authenticate first\r\n";
        return;
    }
    
    std::string channelName = args[1];
    
    // Check if channel exists
    if (channels.find(channelName) == channels.end()) {
        clients[fd].response = "403 " + channelName + " :No such channel\r\n";
        return;
    }
    
    // Check if user is in the channel
    if (!channels[channelName].hasUser(&clients[fd])) {
        clients[fd].response = "442 " + channelName + " :You're not on that channel\r\n";
        return;
    }
    
    // If no topic parameter is provided, return the current topic
    if (args.size() == 2) {
        std::string topic = channels[channelName].getTopic();
        if (topic.empty()) {
            clients[fd].response = "331 " + clients[fd].getNick() + " " + channelName + " :No topic is set\r\n";
        } else {
            clients[fd].response = "332 " + clients[fd].getNick() + " " + channelName + " :" + topic + "\r\n";
        }
        return;
    }
    
    // Setting a new topic
    // Check if topic is restricted to operators
    if (channels[channelName].isTopicRestricted() && !channels[channelName].isOperator(fd)) {
        clients[fd].response = "482 " + channelName + " :You're not channel operator\r\n";
        return;
    }
    
    std::string newTopic = args[2];
    channels[channelName].setTopic(newTopic);
    
    // Notify all channel users about the topic change
    std::string topicMsg = ":" + clients[fd].getNick() + " TOPIC " + channelName + " :" + newTopic + "\r\n";
    broadcastToChannel(channelName, topicMsg, -1);
}

void Server::cmdMode(int fd, std::vector<std::string>& args) {
    if (args.size() < 3) {
        clients[fd].response = "461 * MODE :Not enough parameters\r\n";
        return;
    }
    
    if (!clients[fd].getAuth()) {
        clients[fd].response = "464 * :You must authenticate first\r\n";
        return;
    }
    
    std::string channelName = args[1];
    std::string modeString = args[2];
    
    // Check if channel exists
    if (channels.find(channelName) == channels.end()) {
        clients[fd].response = "403 " + channelName + " :No such channel\r\n";
        return;
    }
    
    // Check if user is in the channel
    if (!channels[channelName].hasUser(&clients[fd])) {
        clients[fd].response = "442 " + channelName + " :You're not on that channel\r\n";
        return;
    }
    
    // Check if user is channel operator
    if (!channels[channelName].isOperator(fd)) {
        clients[fd].response = "482 " + channelName + " :You're not channel operator\r\n";
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
                        clients[fd].response = "461 * MODE +k :Not enough parameters\r\n";
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
                    clients[fd].response = "461 * MODE +o :Not enough parameters\r\n";
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
                        clients[fd].response = "461 * MODE +l :Not enough parameters\r\n";
                        return;
                    }
                    
                    // Convert limit to integer
                    int limit = 0;
                    std::istringstream iss(args[argIndex]);
                    if (!(iss >> limit) || limit < 0) {
                        clients[fd].response = "461 * MODE +l :Invalid limit parameter\r\n";
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