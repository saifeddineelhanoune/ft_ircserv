#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <iostream>
#include <string>
#include <ctime>
#include <sstream>

enum LogLevel {
    DEBUG,
    INFO,
    WARNING,
    ERROR,
    NONE
};

class Logger {
private:
    static LogLevel currentLevel;
    
    // Colors
    static const std::string RESET;
    static const std::string RED;
    static const std::string GREEN;
    static const std::string YELLOW;
    static const std::string BLUE;
    static const std::string MAGENTA;
    static const std::string CYAN;
    static const std::string WHITE;
    
    static std::string getTimestamp() {
        std::time_t now = std::time(0);
        char buffer[80];
        std::strftime(buffer, 80, "[%Y-%m-%d %H:%M:%S]", std::localtime(&now));
        return std::string(buffer);
    }
    
    static std::string getLevelString(LogLevel level) {
        switch (level) {
            case DEBUG:   return BLUE + "[DEBUG]" + RESET;
            case INFO:    return GREEN + "[INFO]" + RESET;
            case WARNING: return YELLOW + "[WARNING]" + RESET;
            case ERROR:   return RED + "[ERROR]" + RESET;
            default:      return "[UNKNOWN]";
        }
    }
    
public:
    static void setLogLevel(LogLevel level) {
        currentLevel = level;
    }
    
    static LogLevel getLogLevel() {
        return currentLevel;
    }
    
    // Standard logging
    static void log(LogLevel level, const std::string& message) {
        if (level >= currentLevel) {
            std::cout << getTimestamp() << " " << getLevelString(level) << " " << message << std::endl;
        }
    }
    
    static void debug(const std::string& message) {
        log(DEBUG, message);
    }
    
    static void info(const std::string& message) {
        log(INFO, message);
    }
    
    static void warning(const std::string& message) {
        log(WARNING, message);
    }
    
    static void error(const std::string& message) {
        log(ERROR, message);
    }
    
    // Client-specific logging
    static void log(LogLevel level, int clientId, const std::string& message) {
        if (level >= currentLevel) {
            std::ostringstream client;
            client << "[Client " << clientId << "]";
            std::cout << getTimestamp() << " " << getLevelString(level) << " " 
                      << MAGENTA << client.str() << RESET 
                      << " " << message << std::endl;
        }
    }
    
    static void debug(int clientId, const std::string& message) {
        log(DEBUG, clientId, message);
    }
    
    static void info(int clientId, const std::string& message) {
        log(INFO, clientId, message);
    }
    
    static void warning(int clientId, const std::string& message) {
        log(WARNING, clientId, message);
    }
    
    static void error(int clientId, const std::string& message) {
        log(ERROR, clientId, message);
    }
    
    // Channel-specific logging
    static void log(LogLevel level, const std::string& channel, const std::string& message) {
        if (level >= currentLevel) {
            std::cout << getTimestamp() << " " << getLevelString(level) << " " 
                      << CYAN << "[Channel " << channel << "]" << RESET 
                      << " " << message << std::endl;
        }
    }
    
    static void debug(const std::string& channel, const std::string& message) {
        log(DEBUG, channel, message);
    }
    
    static void info(const std::string& channel, const std::string& message) {
        log(INFO, channel, message);
    }
    
    static void warning(const std::string& channel, const std::string& message) {
        log(WARNING, channel, message);
    }
    
    static void error(const std::string& channel, const std::string& message) {
        log(ERROR, channel, message);
    }
};

#endif 