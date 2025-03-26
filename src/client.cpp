#include "../include/client.hpp"

Client::Client(int _fd, sockaddr_in addr) : fd(_fd), _addr(addr), Auth(false) {}

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
