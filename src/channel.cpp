#include "../include/channel.hpp"
#include "../include/client.hpp"

Channel::Channel() : topic("Welcome to the channel!"), inviteOnly(false), topicRestricted(false), userLimit(0) {}


void Channel::addUser(Client* client) {
    users.push_back(client);
}
const std::string &Channel::getTopic() const {
    return topic;
}

void Channel::setTopic(const std::string& newTopic) {
    topic = newTopic;
}

bool Channel::isOperator(int clientFd) const {
    return operators.find(clientFd) != operators.end();
}

void Channel::addOperator(int clientFd) {
    operators.insert(clientFd);
}

void Channel::removeOperator(int clientFd) {
    operators.erase(clientFd);
}

bool Channel::isInviteOnly() const {
    return inviteOnly;
}

void Channel::setInviteOnly(bool value) {
    inviteOnly = value;
}

bool Channel::isTopicRestricted() const {
    return topicRestricted;
}

void Channel::setTopicRestricted(bool value) {
    topicRestricted = value;
}

const std::string& Channel::getKey() const {
    return key;
}

void Channel::setKey(const std::string& newKey) {
    key = newKey;
}

int Channel::getUserLimit() const {
    return userLimit;
}

void Channel::setUserLimit(int limit) {
    userLimit = limit;
}

bool Channel::isInvited(int clientFd) const {
    return invitedUsers.find(clientFd) != invitedUsers.end();
}

void Channel::addInvitedUser(int clientFd) {
    invitedUsers.insert(clientFd);
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