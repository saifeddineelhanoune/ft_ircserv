#include "../include/logger.hpp"

// Initialize static members
LogLevel Logger::currentLevel = INFO;

// ANSI color codes
const std::string Logger::RESET   = "\033[0m";
const std::string Logger::RED     = "\033[31m";
const std::string Logger::GREEN   = "\033[32m";
const std::string Logger::YELLOW  = "\033[33m";
const std::string Logger::BLUE    = "\033[34m";
const std::string Logger::MAGENTA = "\033[35m";
const std::string Logger::CYAN    = "\033[36m";
const std::string Logger::WHITE   = "\033[37m"; 