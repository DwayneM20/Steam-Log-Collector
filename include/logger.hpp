#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <string>

enum class SeverityLevel
{
    Info,
    Warning,
    Err, // Named Err to avoid collision with Windows ERROR macro
    Fatal,
    Debug
};

namespace Logger
{
    void log(const std::string &message);
    void log(const std::string &message, SeverityLevel level);

}

#endif
