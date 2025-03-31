#include "../include/server.hpp"

void Server::cmdUser(int fd, std::vector<std::string>& args) {
    if (args.size() < 5) {
        clients[fd].response = "461 * USER :Not enough parameters\r\n";
        clients[fd].sendResponse();
        return;
    }
    
    clients[fd].setUser(args[1]);
    if (clients[fd].getNick() != "" && !clients[fd].getAuth()) {
        std::string response = "001 " + clients[fd].getNick() + " :Welcome to the IRC server\r\n";
        clients[fd].response = response;
    }
    if (clients[fd].getAuth() == false)
    {
        clients[fd].setUserauth();
    }
}