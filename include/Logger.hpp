#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <iostream>
#include <string>
#include <ctime>
#include <fstream>
#include <sstream>

enum LogLevel {
    DEBUG,
    INFO,
    WARNING,
    ERROR,
    FATAL,
    MESSAGE,
    CHANNEL,
    SERVER,
    CLIENT
};

class Logger {
private:
    static bool isColorEnabled;
    static LogLevel minLevel;
    static std::ofstream logFile;
    
    static std::string getTimestamp();
    static std::string getLogLevelString(LogLevel level);
    static std::string getColorCode(LogLevel level);
    static std::string resetColor();

public:
    static void init(const std::string& logFilePath = "", LogLevel level = INFO, bool enableColor = true);
    static void setLogLevel(LogLevel level);
    static void setColorEnabled(bool enabled);
    
    static void log(LogLevel level, const std::string& message);
    
    static void debug(const std::string& message);
    static void info(const std::string& message);
    static void warning(const std::string& message);
    static void error(const std::string& message);
    static void fatal(const std::string& message);
    static void message(const std::string& message);
    static void channel(const std::string& message);
    static void server(const std::string& message);
    static void client(const std::string& message);
    
    static void close();
};

#endif // LOGGER_HPP