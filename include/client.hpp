#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <netinet/in.h>
#include "server.hpp"

class Server;

class Client
{
    private :
        int         fd;
        sockaddr_in     _addr;
        bool        Auth;
        bool    pass;
        bool    user;
        bool    nick;
        std::string Nick;
        std::string User;
        Server *serv;
        std::string buff;
    public :
        Client() : fd(0), Auth(false) ,serv(NULL){};
        Client (int _fd, sockaddr_in addr, Server *ss);
        ~Client();
        void    setNick(std::string _nick);
        void    setUser(std::string _user);
        void    setAuth(bool _auth);
        std::string getNick();
        std::string getUser();
        bool    getAuth();
        int     getFd() const;
        bool    getPassauth();
        bool    getUserauth();
        bool    getNickauth();
        void    setPassauth();
        void    setUserauth();
        void    setNickauth();
        std::string &getbuff() {return buff;}
        sockaddr_in getAddr();
        void    sendResponse();
        void    setAddr(sockaddr_in addr);
        std::string response;
    };

#endif