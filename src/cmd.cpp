#include "../include/Server.hpp"

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
            return;
        }
    }
    
    clients[fd].setNick(args[1]);
    clients[fd].response = ":" + clients[fd].getNick() + " NICK :" + args[1] + "\r\n";
}

void Server::cmdUser(int fd, std::vector<std::string>& args) {
    if (args.size() < 5) {
        clients[fd].response = "461 * USER :Not enough parameters\r\n";
        return;
    }
    
    clients[fd].setUser(args[1]);
    if (clients[fd].getNick() != "" && !clients[fd].getAuth()) {
        std::string response = "001 " + clients[fd].getNick() + " :Welcome to the IRC server\r\n";
        clients[fd].response = response;
    }
}

void Server::cmdPass(int fd, std::vector<std::string>& args) {
    if (args.size() < 2) {
        clients[fd].response = "461 * PASS :Not enough parameters\r\n";
        return;
    }
    
    if (args[1] == data.passwd) {
        clients[fd].setAuth(true);
        clients[fd].response = "001 " + clients[fd].getNick() + " :Welcome to the IRC server\r\n";
    } else {
        clients[fd].response = "464 * :Password incorrect\r\n";
        // Consider disconnecting the client after multiple failed attempts
    }
}

void Server::cmdJoin(int fd, std::vector<std::string>& args) {
    if (args.size() < 2) {
        clients[fd].response = "461 * JOIN :Not enough parameters\r\n";
        return;
    }
    
    if (!clients[fd].getAuth()) {
        clients[fd].response = "464 * :You must authenticate first\r\n";
        return;
    }
    
    std::string channelName = args[1];
    if (channelName[0] != '#') {
        channelName = "#" + channelName;
    }
    
    // Create channel if it doesn't exist
    if (channels.find(channelName) == channels.end()) {
        Channel newChannel;
        channels[channelName] = newChannel;
    }
    
    channels[channelName].addUser(clients[fd]);
    std::string response = ":" + clients[fd].getNick() + " JOIN " + channelName + "\r\n";
    broadcastToChannel(channelName, response, fd);
}

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

void Server::cmdPart(int fd, std::vector<std::string>& args) {
    if (args.size() < 2) {
        clients[fd].response = "461 * PART :Not enough parameters\r\n";
        return;
    }
    
    std::string channelName = args[1];
    if (channels.find(channelName) == channels.end()) {
        clients[fd].response = "403 " + channelName + " :No such channel\r\n";
        return;
    }
    
    std::string response = ":" + clients[fd].getNick() + " PART " + channelName + "\r\n";
    broadcastToChannel(channelName, response, fd);
    channels[channelName].removeUser(clients[fd]);
    
    // Remove channel if empty
    if (channels[channelName].isEmpty()) {
        channels.erase(channelName);
    }
}

void Server::cmdQuit(int fd, std::vector<std::string>& args) {
    std::string quitMessage = args.size() > 1 ? args[1] : "Leaving";
    std::string response = ":" + clients[fd].getNick() + " QUIT :Quit: " + quitMessage + "\r\n";
    
    // Notify all channels this user was in
    std::map<std::string, Channel>::iterator it;
    for (it = channels.begin(); it != channels.end(); ++it) {
        if (it->second.hasUser(clients[fd])) {
            broadcastToChannel(it->first, response, fd);
            it->second.removeUser(clients[fd]);
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

void Server::welcomeClient()
{
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    
    int fd_c = accept(data.socket, (struct sockaddr*)&client_addr, &client_len);
    
    if (fd_c < 0)
    { 
        std::cerr << "Error accepting client" << std::endl;
        return;
    }
    
    std::ostringstream oss;
    oss << "Client accepted FD: " << fd_c;
    
    clients[fd_c] = Client(fd_c, client_addr);

    char host[NI_MAXHOST];
    char service[NI_MAXSERV];
    getnameinfo((struct sockaddr*)&clients[fd_c].getClientInfos(), sizeof(client_addr), 
                host, NI_MAXHOST, service, NI_MAXSERV, 
                NI_NUMERICHOST | NI_NUMERICSERV);

    struct pollfd _fd;
    _fd.fd = fd_c;
    _fd.events = POLLIN;
    pollfds.push_back(_fd);
}

void    Server::broadcastToChannel(const std::string& channel, const std::string& message, int excludeFd) {
    std::vector<Client>& users = channels[channel].getUsers();
    std::vector<Client>::iterator it;
    for (it = users.begin(); it != users.end(); ++it) {
        if (it->getFd() != excludeFd) {
            it->response = message;
        }
    }
}