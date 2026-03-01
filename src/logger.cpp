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

    void log(const std::string &message, SeverityLevel level)
    {
        std::string levelStr;

        switch (level)
        {
        case SeverityLevel::Info:
            levelStr = "INFO";
            break;
        case SeverityLevel::Warning:
            levelStr = "WARNING";
            break;
        case SeverityLevel::Err:
            levelStr = "ERROR";
            break;
        case SeverityLevel::Fatal:
            levelStr = "FATAL";
            break;
        case SeverityLevel::Debug:
            levelStr = "DEBUG";
            break;
        default:
            levelStr = "UNKNOWN";
            break;
        }

        std::cout << "[" << getTimestamp() << "] [" << levelStr << "] " << message << std::endl;
    }
}
