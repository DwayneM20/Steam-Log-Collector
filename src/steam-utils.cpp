#include "steam-utils.hpp"
#include "logger.hpp"
#include <filesystem>
#include <iostream>

#ifdef _WIN32
#include <windows.h>
#include <shlobj.h>
#elif defined(__APPLE__)
#include <unistd.h>
#include <pwd.h>
#elif defined(__linux__)
#include <unistd.h>
#include <pwd.h>
#endif

namespace SteamUtils
{
    std::string getOperatingSystem()
    {
#ifdef _WIN32
        return "Windows";
#elif defined(__APPLE__)
        return "macOS";
#elif defined(__linux__)
        return "Linux";
#else
        return "Unknown OS";
#endif
    }
    std::string getHomeDirectory()
    {
#ifdef _WIN32
        char *userProfile = nullptr;
        size_t len = 0;
        if (_dupenv_s(&userProfile, &len, "USERPROFILE") == 0 && userProfile != nullptr)
        {
            std::string home(userProfile);
            free(userProfile);
            return home;
        }
        return "";
#else
        const char *home = getenv("HOME");
        if (home != nullptr)
        {
            return std::string(home);
        }

        // Fallback to passwd entry

        struct passwd *pw = getpwuid(getuid());
        if (pw != nullptr)
        {
            return std::string(pw->pw_dir);
        }
        return "";
#endif
    }

    bool directoryExists(const std::string &path)
    {
        try
        {
            return std::filesystem::exists(path) && std::filesystem::is_directory(path);
        }
        catch (const std::filesystem::filesystem_error &e)
        {
            Logger::log("Error checking directory: " + path + " - " + e.what());
            return false;
        }
    }

    bool isValidSteamDirectory(const std::string &path)
    {
        std::string os = getOperatingSystem();
        if (os == "Windows")
        {
            return std::filesystem::exists(path + "\\steam.exe") ||
                   std::filesystem::exists(path + "\\Steam.exe");
        }
        else if (os == "macOS")
        {
            return std::filesystem::exists(path + "/Steam") ||
                   std::filesystem::exists(path + "/../Steam") ||
                   std::filesystem::exists(path + "/steamapps");
        }
        else if (os == "Linux")
        {
            return std::filesystem::exists(path + "/steam") ||
                   std::filesystem::exists(path + "/steam.sh") ||
                   std::filesystem::exists(path + "/steamapps");
        }
        return false;
    }

    std::vector<std::string> getSteamDirectoryPaths()
    {
        std::vector<std::string> paths;
        std::string os = getOperatingSystem();
        std::string home = getHomeDirectory();

        if (os == "Windows")
        {
            paths.push_back("C:\\Program Files (x86)\\Steam");
            paths.push_back("C:\\Program Files\\Steam");

            if (!home.empty())
            {
                paths.push_back(home + "\\AppData\\Local\\Steam");
                paths.push_back(home + "\\Steam");
            }
// Check for custom installation paths
#ifdef _WIN32
            HKEY hKey;
            if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, TEXT("SOFTWARE\\WOW6432Node\\Valve\\Steam"), 0, KEY_READ, &hKey) == ERROR_SUCCESS ||
                RegOpenKeyEx(HKEY_LOCAL_MACHINE, TEXT("SOFTWARE\\Valve\\Steam"), 0, KEY_READ, &hKey) == ERROR_SUCCESS)
            {
                char installPath[MAX_PATH];
                DWORD bufferSize = sizeof(installPath);
                if (RegQueryValueEx(hKey, TEXT("InstallPath"), NULL, NULL, (LPBYTE)installPath, &bufferSize) == ERROR_SUCCESS)
                {
                    paths.insert(paths.begin(), std::string(installPath));
                }
                RegCloseKey(hKey);
            }
#endif
        }
        else if (os == "macOS")
        {
            if (!home.empty())
            {
                paths.push_back(home + "/Library/Application Support/Steam");
                paths.push_back(home + "/.steam");
                paths.push_back(home + "/.local/share/Steam");
            }
            paths.push_back("/Applications/Steam.app/Contents/MacOS");
        }
        else if (os == "Linux")
        {
            if (!home.empty())
            {
                paths.push_back(home + "/.steam/steam");
                paths.push_back(home + "/.steam");
                paths.push_back(home + "/.local/share/Steam");
                paths.push_back(home + "/snap/steam/common/.steam");
                paths.push_back(home + "/.var/app/com.valvesoftware.Steam/.steam");
            }
            paths.push_back("/usr/share/steam");
            paths.push_back("/opt/steam");
        }
        return paths;
    }
    std::string findSteamDirectory()
    {
        Logger::log("Searching for Steam installation directory...");

        std::string os = getOperatingSystem();
        Logger::log("Detected operating system: " + os);

        std::vector<std::string> potentialPaths = getSteamDirectoryPaths();

        for (const auto &path : potentialPaths)
        {
            Logger::log("Checking path: " + path);

            if (directoryExists(path))
            {
                bool isValidSteamDir = false;

                if (os == "Windows")
                {
                    isValidSteamDir = std::filesystem::exists(path + "\\steam.exe") ||
                                      std::filesystem::exists(path + "\\Steam.exe");
                }
                else if (os == "macOS")
                {
                    isValidSteamDir = std::filesystem::exists(path + "/Steam") ||
                                      std::filesystem::exists(path + "/../Steam") ||
                                      std::filesystem::exists(path + "/steamapps");
                }
                else if (os == "Linux")
                {
                    isValidSteamDir = std::filesystem::exists(path + "/steam") ||
                                      std::filesystem::exists(path + "/steam.sh") ||
                                      std::filesystem::exists(path + "/steamapps");
                }
                if (isValidSteamDir)
                {
                    Logger::log("Found valid Steam directory: " + path);
                    return path;
                }
                else
                {
                    Logger::log("Invalid Steam directory: " + path);
                }
            }
        }
        Logger::log("Steam directory not found in any of the common locations.");
        return "";
    }
}