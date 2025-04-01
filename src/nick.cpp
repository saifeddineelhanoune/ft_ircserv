#include "../include/server.hpp"

void Server::cmdNick(int fd, std::vector<std::string>& args) {
    if (args.size() < 2) {
        sendError(fd, "431", "NICK", "No nickname given");
        return;
    }
    
    // Check if nickname is already in use
    std::map<int, Client>::iterator it;
    for (it = clients.begin(); it != clients.end(); ++it) {
        if (it->second.getNick() == args[1]) {
            sendError(fd, "433", "NICK", "Nickname is already in use");
            return;
        }
    }
    
    clients[fd].setNick(args[1]);
    clients[fd].response = ":" + clients[fd].getNick() + " NICK :" + args[1] + "\r\n";
    clients[fd].sendResponse();
    if (clients[fd].getAuth() == false)
        clients[fd].setNickauth();
}