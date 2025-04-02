#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include <vector>
#include <set>
#include <string>

// Forward declaration
class Client;

class Channel {
    private:
        std::string topic;
        std::string key;
        std::vector<Client*> users;
        std::set<int> operators;
        std::set<int> invitedUsers;
        bool inviteOnly;
        bool topicRestricted;
        int userLimit;
        
    public:
        Channel();
        void removeInvitedUser(int fd);
        void addUser(Client* client);
        void removeUser(Client* client);
        bool hasUser(Client* client);
        bool isEmpty() const;
        std::vector<Client*>& getUsers();
        
        const std::string& getTopic() const {
            return topic;
        }
        
        void setTopic(const std::string& newTopic) {
            topic = newTopic;
        }

        bool isOperator(int clientFd) const {
            return operators.find(clientFd) != operators.end();
        }

        void addOperator(int clientFd) {
            operators.insert(clientFd);
        }

        void removeOperator(int clientFd) {
            operators.erase(clientFd);
        }

        bool isInviteOnly() const {
            return inviteOnly;
        }

        void setInviteOnly(bool value) {
            inviteOnly = value;
        }

        bool isTopicRestricted() const {
            return topicRestricted;
        }

        void setTopicRestricted(bool value) {
            topicRestricted = value;
        }

        const std::string& getKey() const {
            return key;
        }

        void setKey(const std::string& newKey) {
            key = newKey;
        }

        int getUserLimit() const {
            return userLimit;
        }

        void setUserLimit(int limit) {
            userLimit = limit;
        }

        bool isInvited(int clientFd) const {
            return invitedUsers.find(clientFd) != invitedUsers.end();
        }

        void addInvitedUser(int clientFd) {
            invitedUsers.insert(clientFd);
        }
};

#endif