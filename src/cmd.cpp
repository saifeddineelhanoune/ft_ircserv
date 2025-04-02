#include "../include/server.hpp"
#include <netdb.h>
#include <sstream>

void Server::welcomeClient()
{
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    int fd_c = accept(data.socket, (struct sockaddr*)&client_addr, &client_len);
    
    if (fd_c < 0)
    {
        Logger::error("Error accepting client connection");
        Logger::client("Client Error");
        return;
    }
    std::ostringstream oss;
    oss << fd_c;
    Logger::client("Client Accepted" + oss.str());
    clients[fd_c] = Client(fd_c, client_addr,this);
    sockaddr_in addr = clients[fd_c].getAddr();
    char host[NI_MAXHOST];
    char service[NI_MAXSERV];
    getnameinfo((struct sockaddr*)&addr, sizeof(client_addr), 
                host, NI_MAXHOST, service, NI_MAXSERV, 
                NI_NUMERICHOST | NI_NUMERICSERV);

    struct pollfd _fd;
    _fd.fd = fd_c;
    _fd.events = POLLIN;
    // client[fd_c].auth = false;
    pollfds.push_back(_fd);
}

void Server::broadcastToChannel(const std::string& channel, const std::string& message, int excludeFd) {
    std::vector<Client*>& users = channels[channel].getUsers();
    std::vector<Client*>::iterator it;
    for (it = users.begin(); it != users.end(); ++it) {
        if ((*it)->getFd() != excludeFd) {
            (*it)->response = message;
            (*it)->sendResponse();
        }
    }
}

void Server::deleteClient(int fd) {
    std::map<int, Client>::iterator it = clients.find(fd);
    if (it != clients.end()) {
        Client* client = &it->second;

        for (std::map<std::string, Channel>::iterator ch = channels.begin(); ch != channels.end(); ++ch) {
            std::vector<Client*>& users = ch->second.getUsers();
            
            users.erase(std::remove(users.begin(), users.end(), client), users.end());
        }

        clients.erase(it);
    }

    std::vector<struct pollfd>::iterator pollIt = pollfds.begin();
    while (pollIt != pollfds.end()) {
        if (pollIt->fd == fd) {
            pollIt = pollfds.erase(pollIt);
        } else {
            ++pollIt;
        }
    }

    close(fd);
}




