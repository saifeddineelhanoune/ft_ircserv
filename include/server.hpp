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
#include "Logger.hpp"

const std::string serverName = "IRCServer-v1.0";

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
        void sendChannelNames(int fd, const std::string& channelName);
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
        //mode helpers
        void handleInviteOnlyMode(const std::string& channelName, bool addMode);
        void handleTopicRestrictionMode(const std::string& channelName, bool addMode);
        void handleChannelKeyMode(int fd, const std::string& channelName, bool addMode, 
                                    std::vector<std::string>& args, int& argIndex, 
                                    std::string& modeParams, bool* modeSuccess);
        bool handleOperatorMode(int fd, const std::string& channelName, bool addMode, 
                                std::vector<std::string>& args, int& argIndex, 
                                std::string& modeParams);
        bool handleUserLimitMode(int fd, const std::string& channelName, bool addMode, 
                                std::vector<std::string>& args, int& argIndex, 
                                std::string& modeParams);
        void displayChannelModes(int fd, const std::string& channelName);
        void processChannelModes(int fd, const std::string& target, std::vector<std::string>& args);
        void processUserModes(int fd, const std::string& target, std::vector<std::string>& args);
        void cmdMode(int fd, std::vector<std::string>& args);
       
        void sendError(int client_fd, const std::string &code, const std::string &target, const std::string &message) {
            std::string response = std::string(":") + serverName + " " + code + " " + target + " :" + message + "\r\n";
            send(client_fd, response.c_str(), response.length(), 0);
        }
        
    public:
        void    deleteClient(int fd);
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