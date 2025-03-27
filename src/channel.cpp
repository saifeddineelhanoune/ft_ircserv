#include "../include/channel.hpp"
#include "../include/client.hpp"

void Channel::addUser(Client* client) {
    users.push_back(client);
}

void Channel::removeUser(Client* client) {
    std::vector<Client*>::iterator it;
    for (it = users.begin(); it != users.end(); ++it) {
        if ((*it)->getFd() == client->getFd()) {
            users.erase(it);
            break;
        }
    }
    operators.erase(client->getFd());
}

bool Channel::hasUser(Client* client) {
    std::vector<Client*>::iterator it;
    for (it = users.begin(); it != users.end(); ++it) {
        if ((*it)->getFd() == client->getFd()) {
            return true;
        }
    }
    return false;
}

bool Channel::isEmpty() const {
    return users.empty();
}

std::vector<Client*>& Channel::getUsers() {
    return users;
} 