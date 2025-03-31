#include "../include/server.hpp"

void Server::cmdQuit(int fd, std::vector<std::string>& args) {
    std::string quitMessage = args.size() > 1 ? args[1] : "Leaving";
    std::string response = ":" + clients[fd].getNick() + " QUIT :Quit: " + quitMessage + "\r\n";
    
    // Notify all channels this user was in
    std::map<std::string, Channel>::iterator it;
    for (it = channels.begin(); it != channels.end(); ++it) {
        if (it->second.hasUser(&clients[fd])) {
            broadcastToChannel(it->first, response, fd);
            it->second.removeUser(&clients[fd]);
        }
    }
    
    // Clean up empty channels
    it = channels.begin();
    while (it != channels.end()) {
        if (it->second.isEmpty()) {
            channels.erase(it++);
        } else {
            ++it;
        }
    }
    
    clients.erase(fd);
    close(fd);
}