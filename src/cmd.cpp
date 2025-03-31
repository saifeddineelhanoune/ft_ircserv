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
        std::cerr << "Error accepting client" << std::endl;
        return;
    }
    
    std::cout << "Client accepted" << std::endl;
    std::ostringstream oss;
    oss << "Client accepted FD: " << fd_c;
    std::cout << oss.str() << std::endl;
    clients[fd_c] = Client(fd_c, client_addr);
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
        }
    }
}



