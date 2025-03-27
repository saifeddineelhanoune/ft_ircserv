#include"include/server.hpp"
#include"include/client.hpp"
#include"include/channel.hpp"
#include"include/logger.hpp"

#include <sstream>
#include <string>
#include <iostream>

int stringToInt(const std::string& str) {
    std::stringstream ss(str);
    int result;
    ss >> result;
    if (ss.fail()) {
        throw std::invalid_argument("Invalid input string");
    }
    return result;
}

void printUsage() {
    std::cout << "Usage: ./ircServer <port> <password> [log_level]" << std::endl;
    std::cout << "  port:      Port number to listen on (1-65535)" << std::endl;
    std::cout << "  password:  Server password for user authentication" << std::endl;
    std::cout << "  log_level: Optional, logging level (0-4)" << std::endl;
    std::cout << "             0: DEBUG - All messages" << std::endl;
    std::cout << "             1: INFO - Informational, warnings, and errors" << std::endl;
    std::cout << "             2: WARNING - Warnings and errors only" << std::endl;
    std::cout << "             3: ERROR - Errors only" << std::endl;
    std::cout << "             4: NONE - No logging" << std::endl;
}

int main(int argc, char **argv) {
    if (argc < 3 || argc > 4) {
        printUsage();
        return 1;
    }
    
    // Set log level if provided
    if (argc == 4) {
        int logLevel = atoi(argv[3]);
        if (logLevel < 0 || logLevel > 4) {
            std::cout << "Invalid log level: " << logLevel << std::endl;
            printUsage();
            return 1;
        }
        Logger::setLogLevel(static_cast<LogLevel>(logLevel));
    } else {
        Logger::setLogLevel(INFO); // Default log level
    }
    
    // Validate port
    if (!isStringDigits(argv[1])) {
        Logger::error("Port is not a number");
        return 1;
    }
    
    try {
        int port = stringToInt(argv[1]);
        std::string password = argv[2];
        
        std::ostringstream portStr;
        portStr << port;
        
        Logger::info("Starting IRC server on port " + portStr.str());
        Server server(password, port);
        server.startServer();
    } catch (const std::exception& e) {
        Logger::error("Error starting server: " + std::string(e.what()));
        return 1;
    }
    
    return 0;
}