#include "../include/logger.hpp"

// Initialize static members
bool Logger::isColorEnabled = true;
LogLevel Logger::minLevel = INFO;
std::ofstream Logger::logFile;

// Get current timestamp for log entries
std::string Logger::getTimestamp() {
    time_t now = time(0);
    struct tm *timeinfo = localtime(&now);
    char buffer[80];
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", timeinfo);
    return std::string(buffer);
}

// Convert LogLevel to string
std::string Logger::getLogLevelString(LogLevel level) {
    switch (level) {
        case DEBUG:   return "DEBUG";
        case INFO:    return "INFO";
        case WARNING: return "WARNING";
        case ERROR:   return "ERROR";
        case FATAL:   return "FATAL";
        case MESSAGE: return "MESSAGE";
        case CHANNEL: return "CHANNEL";
        case SERVER:  return "SERVER";
        case CLIENT:  return "CLIENT";
        default:      return "UNKNOWN";
    }
}

// Get ANSI color code for different log levels
std::string Logger::getColorCode(LogLevel level) {
    if (!isColorEnabled) {
        return "";
    }
    
    switch (level) {
        case DEBUG:   return "\033[36m"; // Cyan
        case INFO:    return "\033[32m"; // Green
        case WARNING: return "\033[33m"; // Yellow
        case ERROR:   return "\033[31m"; // Red
        case FATAL:   return "\033[35m"; // Magenta
        case MESSAGE: return "\033[37m"; // White
        case CHANNEL: return "\033[34m"; // Blue
        case SERVER:  return "\033[32m"; // Green
        case CLIENT:  return "\033[36m"; // Cyan
        default:      return "\033[0m";  // Reset
    }
}

// Reset ANSI color
std::string Logger::resetColor() {
    if (!isColorEnabled) {
        return "";
    }
    return "\033[0m";
}

// Initialize the logger
void Logger::init(const std::string& logFilePath, LogLevel level, bool enableColor) {
    minLevel = level;
    isColorEnabled = enableColor;
    
    if (!logFilePath.empty()) {
        logFile.open(logFilePath.c_str(), std::ios::out | std::ios::app);
        if (!logFile.is_open()) {
            std::cerr << "Error: Cannot open log file " << logFilePath << std::endl;
        }
    }
    
    info("Logger initialized");
}

// Set minimum log level
void Logger::setLogLevel(LogLevel level) {
    minLevel = level;
}

// Enable/disable colored output
void Logger::setColorEnabled(bool enabled) {
    isColorEnabled = enabled;
}

// Main log method
void Logger::log(LogLevel level, const std::string& message) {
    if (level < minLevel) {
        return;
    }
    
    std::string timestamp = getTimestamp();
    std::string levelStr = getLogLevelString(level);
    std::string colorCode = getColorCode(level);
    std::string resetColorCode = resetColor();
    
    // Format: [TIME] [LEVEL] Message
    std::stringstream logEntry;
    logEntry << "[" << timestamp << "] [" << levelStr << "] " << message;
    
    // Output to console with color
    std::cout << colorCode << logEntry.str() << resetColorCode << std::endl;
    
    // Output to file without color codes
    if (logFile.is_open()) {
        logFile << logEntry.str() << std::endl;
        logFile.flush();
    }
}

// Convenience methods for different log levels
void Logger::debug(const std::string& message) {
    log(DEBUG, message);
}

void Logger::info(const std::string& message) {
    log(INFO, message);
}

void Logger::warning(const std::string& message) {
    log(WARNING, message);
}

void Logger::error(const std::string& message) {
    log(ERROR, message);
}

void Logger::fatal(const std::string& message) {
    log(FATAL, message);
}

void Logger::message(const std::string& message) {
    log(MESSAGE, message);
}

void Logger::channel(const std::string& message) {
    log(CHANNEL, message);
}

void Logger::server(const std::string& message) {
    log(SERVER, message);
}

void Logger::client(const std::string& message) {
    log(CLIENT, message);
}

// Close log file
void Logger::close() {
    if (logFile.is_open()) {
        logFile.close();
    }
}