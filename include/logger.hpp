#pragma once

#include <string>
#include <string_view>

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
    void log(std::string_view message);
    void log(std::string_view message, SeverityLevel level);

}
