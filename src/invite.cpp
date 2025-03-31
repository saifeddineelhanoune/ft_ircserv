#include "../include/server.hpp"

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
