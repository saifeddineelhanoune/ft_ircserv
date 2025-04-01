#include "../include/server.hpp"

Server::Server(std::string passwd, int port) {
    data.passwd = passwd;
    data.port = port;
    data.socket = 0;
    data.poll = 0;
    data.serverInfo.sin_family = AF_INET;
    data.serverInfo.sin_addr.s_addr = INADDR_ANY;
    data.serverInfo.sin_port = htons(port);

    if (IsValidPass(passwd, data.passwd) == false) {
        std::cerr << "Error: Invalid password" << std::endl;
        exit(1);
    }
    if (IsValidPort(port, data.port) == false) {
        std::cerr << "Error: Invalid port" << std::endl;
        exit(1);
    }

    commands["NICK"] = &Server::cmdNick;
    commands["USER"] = &Server::cmdUser;
    commands["PASS"] = &Server::cmdPass;
    commands["JOIN"] = &Server::cmdJoin;
    commands["PRIVMSG"] = &Server::cmdPrivmsg;
    commands["PART"] = &Server::cmdPart;
    commands["QUIT"] = &Server::cmdQuit;
    commands["KICK"] = &Server::cmdKick;
    commands["INVITE"] = &Server::cmdInvite;
    commands["TOPIC"] = &Server::cmdTopic;
    commands["MODE"] = &Server::cmdMode;
}

Server::~Server() {
    close(data.socket);
    std::vector<struct pollfd>::iterator it = pollfds.begin();
    while (it != pollfds.end()) {
        close(it->fd);
        it++;
    }
}

// void Server::sendError(int fd, const std::string& errorCode, const std::string& command, const std::string& error)
// {
//     std::map<int, Client>::iterator it = clients.find(fd);
//     if (it == clients.end())
//     {
//         std::cerr << "Error: Client not found for FD " << fd << std::endl;
//         return;
//     }
//     std::string t = ":";
//     std::string nick = it->second.getNick().empty() ? "*" : it->second.getNick();
//     it->second.response = t + serverName + " " + errorCode + " " + nick + " " + command + " :" + error + "\r\n";
//     it->second.sendResponse();   
// }


void Server::createSocket() {
    data.socket = socket(AF_INET, SOCK_STREAM, 0);
    if (data.socket == -1) {
        std::cerr << "Error creating socket" << std::endl;
        exit(1);
    }

    int opt = 1;
    if (setsockopt(data.socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1) {
        std::cerr << "Error setting socket options" << std::endl;
        exit(1);
    }

    if (bind(data.socket, (struct sockaddr *)&data.serverInfo, sizeof(data.serverInfo)) == -1) {
        std::cerr << "Error binding socket" << std::endl;
        exit(1);
    }

    if (listen(data.socket, 5) == -1) {
        std::cerr << "Error listening on socket" << std::endl;
        exit(1);
    }

    std::cout << "Server created on port " << data.port << std::endl;
}

void Server::startListen() {
    std::cout << "Server started on port " << data.port << std::endl;
    struct pollfd pfd;
    pfd.fd = data.socket;
    pfd.events = POLLIN;
    pollfds.push_back(pfd);
    data.poll = 1;
    while (data.poll) {
        if (poll(&pollfds[0], pollfds.size(), -1) == -1) {
            std::cerr << "Error polling" << std::endl;
            exit(1);
        }
        for (size_t i = 0; i < pollfds.size(); i++) {
            if (pollfds[i].revents & POLLIN) {
                if (pollfds[i].fd == data.socket) {
                    welcomeClient();
                } else {
                    ReadEvent(pollfds[i].fd);
                }
        }
    }
}
}

void Server::startServer() {
    createSocket();
    startListen();
}

