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
}

Server::~Server() {
    close(data.socket);
    std::vector<struct pollfd>::iterator it = pollfds.begin();
    while (it != pollfds.end()) {
        close(it->fd);
        it++;
    }
}

void Server::createSocket() {
    data.socket = socket(AF_INET, SOCK_STREAM, 0);
    if (data.socket == -1) {
        std::cerr << "Error creating socket" << std::endl;
        exit(1);
    }
    if (bind(data.socket, (struct sockaddr*)&data.serverInfo, sizeof(data.serverInfo)) == -1) {
        std::cerr << "Error binding socket" << std::endl;
        exit(1);
    }
    if (listen(data.socket, 5) == -1) {
        std::cerr << "Error listening on socket" << std::endl;
        exit(1);
    }
}

void Server::startListen() {
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
        std::vector<struct pollfd>::iterator it = pollfds.begin();
        while (it != pollfds.end()) {
            if (it->revents & POLLIN) {
                if (it->fd == data.socket) {
                    int newfd = accept(data.socket, NULL, NULL);
                    if (newfd == -1) {
                        std::cerr << "Error accepting connection" << std::endl;
                        exit(1);
                    }
                    welcomeClient();
                    // struct pollfd pfd;
                    // pfd.fd = newfd;
                    // pfd.events = POLLIN;
                    // pollfds.push_back(pfd);

                } else {
                    ReadEvent(it->fd);
                }
            }
            if (it->revents & POLLOUT) {
                WriteEvent(it->fd);
            }
            it++;
        }
    }
}

void Server::startServer() {
    createSocket();
    startListen();
}

