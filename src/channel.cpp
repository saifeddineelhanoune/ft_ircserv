#include "../include/channel.hpp"
#include "../include/client.hpp"

Channel::Channel() : topic("Welcome to the channel!"), inviteOnly(false), topicRestricted(false), userLimit(0) {}

void Channel::addUser(Client* client) {
    users.push_back(client);
}

void Channel::removeInvitedUser(int fd) {
    // Find and remove the user from the invited list
    std::set<int>::iterator it = std::find(invitedUsers.begin(), invitedUsers.end(), fd);
    if (it != invitedUsers.end()) {
        invitedUsers.erase(it);
    }
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