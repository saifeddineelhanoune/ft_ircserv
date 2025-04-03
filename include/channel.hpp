#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include <vector>
#include <set>
#include <string>

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
        
        const std::string& getTopic() const;
        void setTopic(const std::string& newTopic);
        bool isOperator(int clientFd) const;

        void addOperator(int clientFd);

        void removeOperator(int clientFd);

        bool isInviteOnly() const ;

        void setInviteOnly(bool value);

        bool isTopicRestricted() const ;

        void setTopicRestricted(bool value) ;

        const std::string& getKey() const ;

        void setKey(const std::string& newKey) ;

        int getUserLimit() const ;

        void setUserLimit(int limit);

        bool isInvited(int clientFd) const ;

        void addInvitedUser(int clientFd);
};

#endif