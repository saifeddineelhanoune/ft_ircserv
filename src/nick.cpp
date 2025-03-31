#include "../include/server.hpp"

void Server::cmdNick(int fd, std::vector<std::string>& args) {
    if (args.size() < 2) {
        clients[fd].response = "461 * NICK :Not enough parameters\r\n";
        return;
    }
    
    // Check if nickname is already in use
    std::map<int, Client>::iterator it;
    for (it = clients.begin(); it != clients.end(); ++it) {
        if (it->second.getNick() == args[1]) {
            clients[fd].response = "433 * " + args[1] + " :Nickname is already in use\r\n";
            clients[fd].sendResponse();
            return;
        }
    }
    
    clients[fd].setNick(args[1]);
    clients[fd].response = ":" + clients[fd].getNick() + " NICK :" + args[1] + "\r\n";
    if (clients[fd].getAuth() == false)
        clients[fd].setNickauth();
}