#include "logger.hpp"
#include <iostream>
#include <ctime>
#include <string>

namespace Logger
{
    static std::string getTimestamp()
    {
        std::time_t now = std::time(nullptr);
        std::string time = std::asctime(std::localtime(&now));
        if (!time.empty() && time.back() == '\n')
            time.pop_back();
        return time;
    }

    void log(const std::string &message)
    {
        std::cout << "[" << getTimestamp() << "] " << message << std::endl;
    }

    void log(const std::string &message, SEVERITY_LEVEL level)
    {
        std::string levelStr;

        switch (level)
        {
        case SEVERITY_LEVEL::INFO:
            levelStr = "INFO";
            break;
        case SEVERITY_LEVEL::WARNING:
            levelStr = "WARNING";
            break;
        case SEVERITY_LEVEL::ERR:
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

        std::cout << "[" << getTimestamp() << "] [" << levelStr << "] " << message << std::endl;
    }
}