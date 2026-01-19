#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <string>

enum SEVERITY_LEVEL
{
    INFO,
    WARNING,
    ERROR,
    FATAL,
    DEBUG
};

namespace Logger
{
    void log(const std::string &message);
    void log(const std::string &message, SEVERITY_LEVEL level);

}

#endif