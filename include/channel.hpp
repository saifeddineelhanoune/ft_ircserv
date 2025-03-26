#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include <vector>
#include "client.hpp"

class Channel {
private:
    std::vector<Client> users;
    std::string topic;
    
public:
    Channel() : topic("Welcome to the channel!") {}
    
    void addUser(const Client& client) {
        users.push_back(client);
    }
    
    void removeUser(const Client& client) {
        std::vector<Client>::iterator it;
        for (it = users.begin(); it != users.end(); ++it) {
            if (it->getFd() == client.getFd()) {
                users.erase(it);
                break;
            }
        }
    }
    
    bool hasUser(const Client& client) {
        std::vector<Client>::iterator it;
        for (it = users.begin(); it != users.end(); ++it) {
            if (it->getFd() == client.getFd()) {
                return true;
            }
        }
        return false;
    }
    
    bool isEmpty() const {
        return users.empty();
    }
    
    std::vector<Client>& getUsers() {
        return users;
    }
    
    const std::string& getTopic() const {
        return topic;
    }
    
    void setTopic(const std::string& newTopic) {
        topic = newTopic;
    }
};

#endif