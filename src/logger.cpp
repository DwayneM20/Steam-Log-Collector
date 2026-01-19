#include "logger.hpp"
#include <iostream>
#include <ctime>

namespace Logger
{
    void log(const std::string &message)
    {
        std::time_t now = std::time(nullptr);
        std::cout << "[" << std::asctime(std::localtime(&now)) << "] " << message << std::endl;
    }

    void log(const std::string &message, SEVERITY_LEVEL level)
    {
        std::time_t now = std::time(nullptr);
        std::string levelStr;

        switch (level)
        {
        case SEVERITY_LEVEL::INFO:
            levelStr = "INFO";
            break;
        case SEVERITY_LEVEL::WARNING:
            levelStr = "WARNING";
            break;
        case SEVERITY_LEVEL::ERRORS:
            levelStr = "ERROR";
            break;
        case SEVERITY_LEVEL::FATAL:
            levelStr = "FATAL";
            break;
        case SEVERITY_LEVEL::DEBUG:
            levelStr = "DEBUG";
            break;
        default:
            levelStr = "UNKNOWN";
            break;
        }

        std::cout << "[" << std::asctime(std::localtime(&now)) << "] [" << levelStr << "] " << message << std::endl;
    }
}