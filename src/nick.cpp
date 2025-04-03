#include "../include/server.hpp"

void Server::cmdNick(int fd, std::vector<std::string>& args) {
    if (args.size() < 2) {
        sendError(fd, "431", "NICK", "No nickname given");
        return;
    }
    
    std::string newNick = args[1];

    std::map<int, Client>::iterator it;
    for (it = clients.begin(); it != clients.end(); ++it) {
        if (strcasecmp(it->second.getNick().c_str(), newNick.c_str()) == 0) {
            sendError(fd, "433", "NICK", "Nickname is already in use");
            return;
        }
    }

    std::string oldNick = clients[fd].getNick();
    clients[fd].setNick(newNick);

    std::string response = ":" + oldNick + " NICK :" + newNick + "\r\n";
    if (clients[fd].getAuth()){
        clients[fd].response = response;
        clients[fd].sendResponse();
    }

    for (std::map<std::string, Channel>::iterator itc = channels.begin(); itc != channels.end(); itc++) {
        if (itc->second.hasUser(&clients[fd])) {
            broadcastToChannel(itc->first, response, fd);
        }
    }

    if (!clients[fd].getAuth()) {
        clients[fd].setNickauth();
    }
}
