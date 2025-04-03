#include "../include/client.hpp"

Client::Client(int _fd, sockaddr_in addr,Server *ss) : fd(_fd), _addr(addr), Auth(false), pass(false), user(false), nick(false) ,serv(ss),response(""){}

Client::~Client() {}

void Client::setNick(std::string _nick) {
    Nick = _nick;
}

void Client::setUser(std::string _user) {
    User = _user;
}

void Client::setAuth(bool _auth) {
    Auth = _auth;
}

std::string Client::getNick() {
    return Nick;
}

std::string Client::getUser() {
    return User;
}

bool Client::getAuth() {
    return Auth;
}

int Client::getFd() const{
    return this->fd;
}

sockaddr_in Client::getAddr() {
    return _addr;
}

void Client::setAddr(sockaddr_in addr) {
    _addr = addr;
}

bool Client::getPassauth() {
    return pass;
}

bool Client::getUserauth() {
    return user;
}

bool Client::getNickauth() {
    return nick;
}

void Client::setPassauth() {
    pass = true;
}

void Client::setUserauth() {
    user = true;
}

void Client::setNickauth() {
    nick = true;
}

void    Client::sendResponse() {
    
    int ret = write(fd, response.c_str(), response.length());
    if (ret <= 0) {
        std::cerr << "Error writing to client" << std::endl;
        // (void)serv;

    // response.clear();
        serv->deleteClient(fd);
        // close(fd);
    }
    response.clear();
}
