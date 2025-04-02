#include "../include/server.hpp"

void Server::cmdPing(int fd, std::vector<std::string>& args) {
    if (args.size() < 2) {
        sendError(fd, "461", "PING", "Not enough parameters");
        return;
    }
    std::string token = args[1];
    clients[fd].response = ":" + serverName + " PONG " + serverName + " :" + token + "\r\n";
    clients[fd].sendResponse();
    Logger::debug("Responded to PING from client " + clients[fd].getNick() + " with token: " + token);
}