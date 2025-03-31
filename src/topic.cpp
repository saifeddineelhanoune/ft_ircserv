#include "../include/server.hpp"

void Server::cmdTopic(int fd, std::vector<std::string>& args) {
    if (args.size() < 2) {
        sendError(fd, "461", "TOPIC", "Not enough parameters");
        return;
    }
    
    if (!clients[fd].getAuth()) {
        sendError(fd, "464", "TOPIC", "You must authenticate first");
        return;
    }
    
    std::string channelName = args[1];
    
    // Check if channel exists
    if (channels.find(channelName) == channels.end()) {
        sendError(fd, "403", channelName, "No such channel");
        return;
    }
    
    // Check if user is in the channel
    if (!channels[channelName].hasUser(&clients[fd])) {
        sendError(fd, "442", channelName, "You're not on that channel");
        return;
    }
    
    // If no topic parameter is provided, return the current topic
    if (args.size() == 2) {
        std::string topic = channels[channelName].getTopic();
        if (topic.empty()) {
            clients[fd].response = "331 " + clients[fd].getNick() + " " + channelName + " :No topic is set\r\n";
            clients[fd].sendResponse();
        } else {
            clients[fd].response = "332 " + clients[fd].getNick() + " " + channelName + " :" + topic + "\r\n";
            clients[fd].sendResponse();
        }
        return;
    }
    
    // Setting a new topic
    // Check if topic is restricted to operators
    if (channels[channelName].isTopicRestricted() && !channels[channelName].isOperator(fd)) {
        sendError(fd, "482", channelName, "You're not channel operator");
        return;
    }
    
    std::string newTopic = args[2];
    channels[channelName].setTopic(newTopic);
    
    // Notify all channel users about the topic change
    std::string topicMsg = ":" + clients[fd].getNick() + " TOPIC " + channelName + " :" + newTopic + "\r\n";
    broadcastToChannel(channelName, topicMsg, -1);
}
