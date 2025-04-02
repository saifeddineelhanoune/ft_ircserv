#include "../include/server.hpp"

#include "../include/server.hpp"

void Server::cmdKick(int fd, std::vector<std::string>& args) {
    if (args.size() < 3) {
        sendError(fd, "461", "KICK", "Not enough parameters");
        return;
    }
    
    if (!clients[fd].getAuth()) {
        sendError(fd, "451", "KICK", "You have not registered");
        return;
    }
    
    std::string channelName = args[1];
    std::string targetNick = args[2];
    std::string reason = args.size() > 3 ? args[3] : "No reason specified";
    
    // Check if channel exists
    if (channels.find(channelName) == channels.end()) {
        sendError(fd, "403", channelName, "No such channel");
        return;
    }
    
    // Check if user is an operator
    if (!channels[channelName].isOperator(fd)) {
        sendError(fd, "482", channelName, "You're not channel operator");
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
        sendError(fd, "441", targetNick, "They aren't on that channel");
        return;
    }
    
    // Create kick message in proper IRC format
    std::string kickMsg = ":" + clients[fd].getNick() + "!" + clients[fd].getUser() + "@" + 
                         serverName + " KICK " + channelName + " " + targetNick + " :" + reason + "\r\n";
    
    // Send kick message to the kicked user first
    clients[targetFd].response = kickMsg;
    clients[targetFd].sendResponse();
    
    // Broadcast kick message to all other channel members
    broadcastToChannel(channelName, kickMsg, targetFd);
    
    // Use the Channel's removeUser method to properly remove the user
    channels[channelName].removeUser(&clients[targetFd]);
    
    Logger::info("User " + targetNick + " was kicked from " + channelName + " by " + clients[fd].getNick());
}