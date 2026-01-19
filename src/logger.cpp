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
        case INFO:
            levelStr = "INFO";
            break;
        case WARNING:
            levelStr = "WARNING";
            break;
        case ERROR:
            levelStr = "ERROR";
            break;
        case FATAL:
            levelStr = "FATAL";
            break;
        case DEBUG:
            levelStr = "DEBUG";
            break;
        default:
            levelStr = "UNKNOWN";
            break;
        }

        std::cout << "[" << std::asctime(std::localtime(&now)) << "] [" << levelStr << "] " << message << std::endl;
    }
}