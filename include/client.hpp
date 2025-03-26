#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <netinet/in.h>
#include"server.hpp"

class Client
{
    private :
    int         fd;
    sockaddr_in     _addr;
    bool        Auth;
        std::string Nick;
        std::string User;
    public :
        Client() : fd(0), Auth(false) {};
        Client (int _fd, sockaddr_in addr);
        ~Client();
        void    setNick(std::string _nick);
        void    setUser(std::string _user);
        void    setAuth(bool _auth);
        std::string getNick();
        std::string getUser();
        bool    getAuth();
        int     getFd() const;
        sockaddr_in getAddr();
        void    setAddr(sockaddr_in addr);
        std::string response;
    };

#endif