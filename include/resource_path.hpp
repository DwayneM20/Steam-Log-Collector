#pragma once
#include <filesystem>
#include <string>

#ifdef _WIN32
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

inline std::filesystem::path GetExecutableDir()
{
#ifdef _WIN32
    char buf[MAX_PATH];
    GetModuleFileNameA(nullptr, buf, MAX_PATH);
    return std::filesystem::path(buf).parent_path();
#else
    return std::filesystem::canonical("/proc/self/exe").parent_path();
#endif
}

inline std::filesystem::path GetResourcePath(const std::string &relativePath)
{
    return GetExecutableDir() / relativePath;
}
