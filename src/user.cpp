#include "../include/server.hpp"

void Server::cmdUser(int fd, std::vector<std::string>& args) {
    if (args.size() < 5) {
        sendError(fd, "461", "USER", "Not enough parameters");
        return;
    }
    
    clients[fd].setUser(args[1]);
    clients[fd].setUserauth();
    
    // If both PASS and NICK are already set, complete authentication
    if (clients[fd].getPassauth() && clients[fd].getNickauth() && !clients[fd].getAuth()) {
        clients[fd].setAuth(true);
        
        // Send welcome messages according to RFC 1459/2812
        std::string nick = clients[fd].getNick();
        std::string user = clients[fd].getUser();
        std::string host = "localhost"; // Would be better to get actual hostname
        
        // 001 RPL_WELCOME
        clients[fd].response = ":" + serverName + " 001 " + nick + " :Welcome to the Internet Relay Network " 
                             + nick + "!" + user + "@" + host + "\r\n";
        clients[fd].sendResponse();
        
        // 002 RPL_YOURHOST
        clients[fd].response = ":" + serverName + " 002 " + nick + " :Your host is " + serverName 
                             + ", running version 1.0\r\n";
        clients[fd].sendResponse();
        
        // 003 RPL_CREATED
        clients[fd].response = ":" + serverName + " 003 " + nick + " :This server was created recently\r\n";
        clients[fd].sendResponse();
        
        // 004 RPL_MYINFO
        clients[fd].response = ":" + serverName + " 004 " + nick + " " + serverName 
                             + " 1.0 itkol\r\n";
        clients[fd].sendResponse();
    }
}