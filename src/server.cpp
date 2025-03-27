#include "../include/server.hpp"

Server::Server(std::string passwd, int port) {
    data.passwd = passwd;
    data.port = port;
    data.socket = 0;
    data.poll = 0;
    data.serverInfo.sin_family = AF_INET;
    data.serverInfo.sin_addr.s_addr = INADDR_ANY;
    data.serverInfo.sin_port = htons(port);
    
    // Set default log level
    Logger::setLogLevel(INFO);
    
    std::ostringstream portStr;
    portStr << port;
    Logger::info("Initializing IRC server on port " + portStr.str());

    if (IsValidPass(passwd, data.passwd) == false) {
        Logger::error("Invalid password");
        exit(1);
    }
    if (IsValidPort(port, data.port) == false) {
        Logger::error("Invalid port");
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
    
    Logger::info("Command handlers registered successfully");
}

Server::~Server() {
    Logger::info("Shutting down server");
    close(data.socket);
    std::vector<struct pollfd>::iterator it = pollfds.begin();
    while (it != pollfds.end()) {
        close(it->fd);
        it++;
    }
}

void Server::createSocket() {
    Logger::debug("Creating server socket");
    data.socket = socket(AF_INET, SOCK_STREAM, 0);
    if (data.socket == -1) {
        Logger::error("Failed to create socket");
        exit(1);
    }
    if (bind(data.socket, (struct sockaddr*)&data.serverInfo, sizeof(data.serverInfo)) == -1) {
        std::ostringstream portStr;
        portStr << data.port;
        Logger::error("Failed to bind socket to port " + portStr.str());
        exit(1);
    }
    if (listen(data.socket, 5) == -1) {
        Logger::error("Failed to listen on socket");
        exit(1);
    }
    std::ostringstream portStr;
    portStr << data.port;
    Logger::info("Server socket created and listening on port " + portStr.str());
}

void Server::startListen() {
    struct pollfd pfd;
    pfd.fd = data.socket;
    pfd.events = POLLIN;
    pollfds.push_back(pfd);
    data.poll = 1;
    
    Logger::info("Server is now accepting connections");
    
    while (data.poll) {
        if (poll(&pollfds[0], pollfds.size(), -1) == -1) {
            Logger::error("Poll failed");
            exit(1);
        }
        std::vector<struct pollfd>::iterator it = pollfds.begin();
        while (it != pollfds.end()) {
            if (it->revents & POLLIN) {
                if (it->fd == data.socket) {
                    int newfd = accept(data.socket, NULL, NULL);
                    if (newfd == -1) {
                        Logger::error("Failed to accept new connection");
                        exit(1);
                    }
                    welcomeClient();
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
    Logger::info("Starting IRC server");
    createSocket();
    startListen();
}

