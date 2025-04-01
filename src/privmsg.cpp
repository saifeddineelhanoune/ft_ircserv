#include "../include/server.hpp"

void Server::cmdPrivmsg(int fd, std::vector<std::string>& args) {
    if (args.size() < 3) {
        clients[fd].response = "461 * PRIVMSG :Not enough parameters\r\n";
        clients[fd].sendResponse();
        return;
    }
    
    if (!clients[fd].getAuth()) {
        clients[fd].response = "464 * :You must authenticate first\r\n";
        clients[fd].sendResponse();
        return;
    }
    
    std::string target = args[1];
    std::string message;

    for (size_t i = 2; i < args.size(); i++) {
        if (i == 2 && args[i][0] == ':') {
            message = args[i].substr(1);
        } else {
            if (!message.empty())
                message += " ";
            message += args[i];
        }
    }

    if (message.empty()) {
        clients[fd].response = "412 :No text to send\r\n";
        clients[fd].sendResponse();
        return;
    }

    if (target[0] == '#') {
        if (channels.find(target) != channels.end()) {
            std::string response = ":" + clients[fd].getNick() + " PRIVMSG " + target + " :" + message + "\r\n";
            if (!channels[target].hasUser(&clients[fd])) {
                clients[fd].response = "442" + clients[fd].getNick() + " not found in channel " + target;
                clients[fd].sendResponse();
                // sendError(fd, "442", target, "You're not on that channel");
                Logger::warning(clients[fd].getNick() + " not found in channel " + target);
                return ;
            }
            broadcastToChannel(target, response, fd);
        } else {
            clients[fd].response = "403 " + target + " :No such channel\r\n";
            clients[fd].sendResponse();
        }
    } else {
        bool found = false;
        std::map<int, Client>::iterator it;
        for (it = clients.begin(); it != clients.end(); ++it) {
            if (it->second.getNick() == target) {
                std::string response = ":" + clients[fd].getNick() + " PRIVMSG " + target + " :" + message + "\r\n";
                it->second.response = response;
                it->second.sendResponse();
                found = true;
                break;
            }
        }
        if (!found) {
            clients[fd].response = "401 " + target + " :No such nick\r\n";
            clients[fd].sendResponse();
        }
    }
}