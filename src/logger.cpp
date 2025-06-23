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
}