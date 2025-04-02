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
        std::map<int, Client>::iterator it = clients.find(fd);
        if (it != clients.end())
        {
            std::string response = ":" + it->second.getNick() + " QUIT :Quit: Leaving\r\n";
            deleteClient(fd);
            Logger::fatal(response);
            // clients.erase(fd);
            // close(fd);
        }
        Logger::info("Connection closed");
        return;
    }
    Logger::debug("processing read event");
    buffer[readed] = '\0';
    std::string input(buffer);
    std::istringstream iss(input);
    std::string line;
    Logger::debug("recieved data: " + input);
    Logger::info("start parsing data");
    while (std::getline(iss, line))
    {
        if (!line.empty() && line[line.length() - 1] == '\r')
            line.erase(line.length() - 1);
        if (line.empty())
            continue;
        handleCommands(fd, line);
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

void Server::handleCommands(int fd, std::string command)
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
        Logger::error("Invalid Command");
        clients[fd].response = "421 * :Unknown command\r\n";
        clients[fd].sendResponse();
        return;
    }
    Logger::info("start Handling Command " + args[0]);
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
        else if (clients[fd].getUserauth() == false)
        {
            if (args[0] != "USER" && args[0] != "NICK")
            {
                sendError(fd, "451", args[0], "You have not registered");
                return;
            }
        }
        else if (clients[fd].getNickauth() == false)
        {
            if (args[0] != "USER" && args[0] != "NICK") // Fixed: Was checking args[1] instead of args[0]
            {
                sendError(fd, "451", args[0], "You have not registered");
                return;
            }
        }
    }
    (this->*(it->second))(fd, args);
}