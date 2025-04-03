#include "../include/server.hpp"

void Server::cmdUser(int fd, std::vector<std::string>& args) {
    if (args.size() < 5) {
        sendError(fd, "461", "USER", "Not enough parameters");
        return;
    }
    clients[fd].setUser(args[1]);
    clients[fd].setUserauth();
    if (clients[fd].getPassauth() && clients[fd].getNickauth() && !clients[fd].getAuth()) {
        clients[fd].setAuth(true);
        
        std::string nick = clients[fd].getNick();
        std::string user = clients[fd].getUser();
        std::string host = "localhost";
        
        clients[fd].response = "001 " + clients[fd].getNick() + " :Welcome to the IRC server\r\n";
        clients[fd].sendResponse();
        clients[fd].response = ":" + serverName + " 002 " + nick + " :Your host is " + serverName 
                             + ", running version 1.0\r\n";
        clients[fd].sendResponse();

        clients[fd].response = ":" + serverName + " 003 " + nick + " :This server was created recently\r\n";
        clients[fd].sendResponse();

        clients[fd].response = ":" + serverName + " 004 " + nick + " " + serverName 
                             + " 1.0 itkol\r\n";
        clients[fd].sendResponse();
    }
}