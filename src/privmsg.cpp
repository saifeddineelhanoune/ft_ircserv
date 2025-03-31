#include "../include/server.hpp"

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