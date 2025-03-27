#ifndef SERVER_HPP
#define SERVER_HPP

#include <netinet/in.h> 
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <iostream>
#include <sys/poll.h>
#include <string>
#include <sstream>
#include <vector>
#include <map>
#include <cstdlib>
#include "client.hpp"
#include "channel.hpp"
#include "logger.hpp"

struct servData
{
    std::string passwd;
    int port;
    int socket;
    int poll;
    struct sockaddr_in serverInfo;
};
class Server;
typedef void (Server::*CommandHandler)(int, std::vector<std::string>&);

class Server {
    private:
        servData data;
        std::vector<struct pollfd> pollfds;
        std::map<int, Client> clients;
        std::map<std::string, Channel> channels;
        std::map<std::string, CommandHandler> commands;
        void createSocket();
        void startListen();
        void broadcastToChannel(const std::string& channel, const std::string& message, int excludeFd);

        void handleCommands(int fd, std::string command);
        void cmdNick(int fd, std::vector<std::string>& args);
        void cmdUser(int fd, std::vector<std::string>& args);
        void cmdPass(int fd, std::vector<std::string>& args);
        void cmdJoin(int fd, std::vector<std::string>& args);
        void cmdPrivmsg(int fd, std::vector<std::string>& args);
        void cmdPart(int fd, std::vector<std::string>& args);
        void cmdQuit(int fd, std::vector<std::string>& args);
        // Channel operator commands
        void cmdKick(int fd, std::vector<std::string>& args);
        void cmdInvite(int fd, std::vector<std::string>& args);
        void cmdTopic(int fd, std::vector<std::string>& args);
        void cmdMode(int fd, std::vector<std::string>& args);
    public:
        void startServer();
        Server(std::string passwd, int port);
        ~Server();
        void welcomeClient();
        void WriteEvent(int fd);
        void ReadEvent(int fd);
};

bool isStringDigits(std::string str);
bool IsValidPort(long port, int& _tport);
bool IsValidPass(std::string _pass, std::string& passwd);

#endif