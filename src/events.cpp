#include "../include/server.hpp"
#include <sstream>
#include <algorithm>
#include <cstring>

void Server::ReadEvent(int fd)
{
    char buffer[1024];
    std::memset(buffer, 0, sizeof(buffer));
    
    int readed = read(fd, buffer, sizeof(buffer) - 1);
    
    if (readed <= 0)
    {
        std::ostringstream oss;
        oss << "Closing connection FD: " << fd;
        std::map<int, Client>::iterator it = clients.find(fd);
        if (it != clients.end())
        {
            std::string response = ":" + it->second.getNick() + " QUIT :Quit: Leaving\r\n";
            // broadcastToChannel(response, fd);
            clients.erase(fd);
            close(fd);
        }
        // clients.erase(fd);
        return;
    }
    
    buffer[readed] = '\0';
    std::string input(buffer);
    std::istringstream iss(input);
    std::string line;
    std::cout << "Received: " << input << std::endl;
    while (std::getline(iss, line))
    {
        // Remove \r if present
        // std::cout << "Received: " << line << std::endl;
        if (!line.empty() && line[line.length() - 1] == '\r')
            line.erase(line.length() - 1);
        
        if (line.empty())
            continue;
            
        handleCommands(fd, line);
        // std::cout << "Received: " << line << std::endl;
    }
}

void    Server::WriteEvent(int fd)
{
    std::map<int, Client>::iterator it = clients.find(fd);
    if (it == clients.end())
    {
        std::cerr << "Error: Client not found" << std::endl;
        return;
    }
    int ret = write(fd, it->second.response.c_str(), it->second.response.length());
    if (ret == -1)
    {
        std::cerr << "Error writing to client" << std::endl;
        close(fd);
        return;
    }
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
                clients[fd].response = "464 * :You must authenticate first\r\n";
                clients[fd].sendResponse();
                return;
            }
        }
        else if (clients[fd].getUserauth() == false)
        {
            if (args[0] != "USER")
            {
                clients[fd].response = "464 * :You must authenticate first\r\n";
                clients[fd].sendResponse();
                return;
            }
        }
        else if (clients[fd].getNickauth() == false)
        {
            if (args[0] != "NICK")
            {
                clients[fd].response = "464 * :You must authenticate first\r\n";
                clients[fd].sendResponse();
                return;
            }
        }
    }
    (this->*(it->second))(fd, args);
}
