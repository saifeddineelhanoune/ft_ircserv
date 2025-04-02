#include "../include/server.hpp"
#include "../include/Logger.hpp"
#include <sstream>
#include <algorithm>
#include <cstring>

void Server::ReadEvent(int fd)
{
    char buffer[1024];
    std::memset(buffer, 0, sizeof(buffer));
    Logger::debug("starting read event");

    int readed = read(fd, buffer, sizeof(buffer) - 1);

    if (readed <= 0)
    {
        std::map<int,Client>::iterator it = clients.find(fd);
        if (it != clients.end())
        {
            std::string response = ":" + it->second.getNick() + " QUIT :Quit: Leaving\r\n";
            Logger::fatal(response);
            deleteClient(fd);
        }
        Logger::info("Connection closed");
        return;
    }

    buffer[readed] = '\0';
    Logger::debug("received data: " + std::string(buffer));
    clients[fd].getbuff().append(buffer);
    std::string& clientBuffer = clients[fd].getbuff();

    std::size_t pos;
    while ((pos = clientBuffer.find_first_of("\r\n")) != std::string::npos) 
    {
        std::string command = clientBuffer.substr(0, pos);
        clientBuffer.erase(0, pos + 1); 

        if (!command.empty() && command.back() == '\r')
            command.pop_back();

        if (!command.empty()) 
        {
            Logger::debug("Processing command: " + command);
            handleCommands(fd, command);
        }
    }

    Logger::info("end of read event");
}



void    Server::WriteEvent(int fd)
{
    Logger::info("start write event");
    Logger::client("searching for matching client");
    std::map<int, Client>::iterator it = clients.find(fd);
    if (it == clients.end())
    {
        Logger::error("Client not found");
        return;
    }
    std::cout << fd;
    Logger::client("writing to client ");
    int ret = write(fd, it->second.response.c_str(), it->second.response.length());
    if (ret == -1)
    {
        Logger::fatal("Error writing to client");
        close(fd);
        return;
    }
    Logger::message("clearing buffer");
    it->second.response.clear();
}

void    Server::handleCommands(int fd, std::string command)
{
    std::vector<std::string> args;
    std::string::size_type pos = 0, lastPos = 0;
    while ((pos = command.find(" ", lastPos)) != std::string::npos)
    {
        args.push_back(command.substr(lastPos, pos - lastPos));
        lastPos = pos + 1;
    }
    args.push_back(command.substr(lastPos));
    std::map<std::string, CommandHandler>::iterator it = commands.find(args[0]);
    if (it == commands.end())
    {
        std::cerr << "Error: Invalid command" << std::endl;
        clients[fd].response = "421 * :Unknown command\r\n";
        clients[fd].sendResponse();
        return;
    }
    std::cout << "Command: " << args[0] << std::endl;
    if (clients[fd].getAuth() == false)
    {
        if (clients[fd].getPassauth() == false)
        {
            if (args[0] != "PASS")
            {
                sendError(fd, "451", args[0], "You have not registered");
                return;
            }
        }
        else if (clients[fd].getNickauth() == false)
        {
            if (args[0] != "NICK")
            {
                sendError(fd, "451", args[0], "You have not registered");
                // clients[fd].response = "464 * :You must authenticate first\r\n";
                // clients[fd].sendResponse();
                return;
            }
        }
        else if (clients[fd].getUserauth() == false)
        {
            if (args[0] != "USER")
            {
                sendError(fd, "451", args[0], "You have not registered");
                return;
            }
        }
    }
    (this->*(it->second))(fd, args);
    if (clients[fd].getAuth() == false && clients[fd].getPassauth() && clients[fd].getUserauth() && clients[fd].getNickauth())
    {
        clients[fd].setAuth(true);
        clients[fd].response = "001 " + clients[fd].getNick() + " :Welcome to the IRC server\r\n";
        clients[fd].sendResponse();
        clients[fd].response = "002 " + clients[fd].getNick() + " :Your host is " + inet_ntoa(data.serverInfo.sin_addr) + "\r\n";
        clients[fd].sendResponse();
        clients[fd].response = "003 " + clients[fd].getNick() + " :This server was created " __DATE__ "\r\n";
        clients[fd].sendResponse();
        clients[fd].response = "004 " + clients[fd].getNick() + " :This server is running " __DATE__ "\r\n";
        clients[fd].sendResponse();
    }
}
