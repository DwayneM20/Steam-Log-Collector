#include "logger.hpp"
#include <iostream>
#include <ctime>
#include <string>
#include <sstream>
#include <iomanip>

namespace Logger
{
    static std::string getTimestamp()
    {
        std::time_t now = std::time(nullptr);
        std::tm tmBuf{};
#ifdef _WIN32
        localtime_s(&tmBuf, &now);
#else
        localtime_r(&now, &tmBuf);
#endif
        std::ostringstream oss;
        oss << std::put_time(&tmBuf, "%c");
        return oss.str();
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
