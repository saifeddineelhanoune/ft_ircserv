#include "../include/server.hpp"

void Server::cmdKick(int fd, std::vector<std::string>& args) {
    if (args.size() < 3) {
        clients[fd].response = "461 * KICK :Not enough parameters\r\n";
        return;
    }
    
    if (!clients[fd].getAuth()) {
        clients[fd].response = "464 * :You must authenticate first\r\n";
        return;
    }
    
    std::string channelName = args[1];
    std::string targetNick = args[2];
    std::string reason = args.size() > 3 ? args[3] : "No reason specified";
    
    // Check if channel exists
    if (channels.find(channelName) == channels.end()) {
        clients[fd].response = "403 " + channelName + " :No such channel\r\n";
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
        clients[fd].response = "441 " + targetNick + " " + channelName + " :They aren't on that channel\r\n";
        clients[fd].sendResponse();
        return;
    }
    
    // Broadcast kick message to channel
    std::string kickMsg = ":" + clients[fd].getNick() + " KICK " + channelName + " " + targetNick + " :" + reason + "\r\n";
    broadcastToChannel(channelName, kickMsg, -1);
    
    // Remove the user from the channel
    channels[channelName].removeUser(&clients[targetFd]);
}