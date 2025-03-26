#include "../include/Server.hpp"

void    Server::ReadEvent(int fd)
{
    char    buffer[1024];
    int     ret;

    ret = read(fd, buffer, 1024);
    if (ret == 0)
    {
        std::cerr << "Client disconnected" << std::endl;
        close(fd);
        return;
    }
    if (ret == -1)
    {
        std::cerr << "Error reading from client" << std::endl;
        close(fd);
        return;
    }
    std::string command(buffer);
    handleCommands(fd, command);
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
        return;
    }
    (this->*(it->second))(fd, args);
}

// void    Server::welcomeClient(int fd)
// {
//     std::string welcome = "Welcome to the server!\r\n";
//     int ret = write(fd, welcome.c_str(), welcome.length());
//     if (ret == -1)
//     {
//         std::cerr << "Error writing to client" << std::endl;
//         close(fd);
//         return;
//     }
// }
