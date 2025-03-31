#include "../include/server.hpp"

void Server::cmdInvite(int fd, std::vector<std::string>& args) {
    if (args.size() < 3) {
        sendError(fd, "461", "INVITE", "Not enough parameters");
        return;
    }
    
    if (!clients[fd].getAuth()) {
        sendError(fd, "451", "INVITE", "You have not registered");
        return;
    }
    
    std::string targetNick = args[1];
    std::string channelName = args[2];
    
    // Check if channel exists
    if (channels.find(channelName) == channels.end()) {
        sendError(fd, "403", channelName, "No such channel");
        return;
    }
    
    // Check if inviteOnly mode is set and user is not an operator
    if (channels[channelName].isInviteOnly() && !channels[channelName].isOperator(fd)) {
        sendError(fd, "482", channelName, "You're not channel operator");
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
        sendError(fd, "401", targetNick, "No such nick");
        return;
    }
    
    // Check if user is already in the channel
    if (channels[channelName].hasUser(&clients[targetFd])) {
        sendError(fd, "443", targetNick, "is already on channel");
        return;
    }
    
    // Add user to invited list
    channels[channelName].addInvitedUser(targetFd);
    
    // Send invite notification to target user
    std::string inviteMsg = ":" + clients[fd].getNick() + " INVITE " + targetNick + " :" + channelName + "\r\n";
    clients[targetFd].response = inviteMsg;
    clients[targetFd].sendResponse();
    
    // Confirm to inviter
    clients[fd].response = "341 " + clients[fd].getNick() + " " + targetNick + " " + channelName + "\r\n";
    clients[fd].sendResponse();
}
