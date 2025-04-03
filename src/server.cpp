#include "../include/server.hpp"

Server::Server(std::string passwd, int port) {
    Logger::init("irc_server.log", DEBUG, true);
    Logger::server("Server initializing");
    data.passwd = passwd;
    data.port = port;
    data.socket = 0;
    data.poll = 0;
    data.serverInfo.sin_family = AF_INET;
    data.serverInfo.sin_addr.s_addr = INADDR_ANY;
    data.serverInfo.sin_port = htons(port);

    if (IsValidPass(passwd, data.passwd) == false) {
        Logger::error("Error: Invalid password");
        exit(1);
    }
    if (IsValidPort(port, data.port) == false) {
        Logger::error("Error: Invalid port");
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
    commands["PING"] = &Server::cmdPing;
    commands["PONG"] = &Server::cmdPong;

    Logger::info("Server initialized");
}

Server::~Server() {
    Logger::server("Server shutting down");
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
        Logger::error("Error creating socket");
        exit(1);
    }

    int opt = 1;
    if (setsockopt(data.socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1) {
        Logger::error("Error setting socket options");
        exit(1);
    }

    if (bind(data.socket, (struct sockaddr *)&data.serverInfo, sizeof(data.serverInfo)) == -1) {
        Logger::error("Error binding socket");
        exit(1);
    }

    if (listen(data.socket, 5) == -1) {
        Logger::error("Error listening on socket");
        exit(1);
    }
    std::ostringstream oss;
    oss << "Server created on port " << data.port;
    Logger::server(oss.str());
}

void Server::startListen() {
    std::ostringstream oss;
    oss << "Server listening on port " << data.port;
    Logger::info(oss.str());
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

