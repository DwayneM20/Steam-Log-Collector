#include "steam-utils.hpp"
#include "logger.hpp"
#include <filesystem>
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cctype>
#include <iomanip>
#include <chrono>
#include <memory>

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
            // Check multiple drives for Steam installation
            std::vector<char> drives = {'C', 'D', 'E', 'F', 'G', 'H'};

            for (char drive : drives)
            {
                std::string driveRoot = std::string(1, drive) + ":";
                paths.push_back(driveRoot + "\\Program Files (x86)\\Steam");
                paths.push_back(driveRoot + "\\Program Files\\Steam");
                paths.push_back(driveRoot + "\\Steam");
                paths.push_back(driveRoot + "\\Games\\Steam");
            }

            if (!home.empty())
            {
                paths.push_back(home + "\\AppData\\Local\\Steam");
                paths.push_back(home + "\\Steam");
            }

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
                if (isValidSteamDirectory(path))
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

    GameInfo parseAcfFile(const std::string &acfFilePath)
    {
        GameInfo game;
        std::ifstream file(acfFilePath);

        if (!file.is_open())
        {
            Logger::log("Failed to open ACF file: " + acfFilePath);
            return game;
        }

        std::string line;
        while (std::getline(file, line))
        {
            line.erase(0, line.find_first_not_of(" \t"));
            line.erase(line.find_last_not_of(" \t") + 1);

            if (line.find("\"appid\"") == 0)
            {
                size_t start = line.find("\"", 8);
                size_t end = line.find("\"", start + 1);
                if (start != std::string::npos && end != std::string::npos)
                {
                    game.appId = line.substr(start + 1, end - start - 1);
                }
            }
            else if (line.find("\"name\"") == 0)
            {
                size_t start = line.find("\"", 7);
                size_t end = line.find("\"", start + 1);
                if (start != std::string::npos && end != std::string::npos)
                {
                    game.name = line.substr(start + 1, end - start - 1);
                }
            }
            else if (line.find("\"installdir\"") == 0)
            {
                size_t start = line.find("\"", 12);
                size_t end = line.find("\"", start + 1);
                if (start != std::string::npos && end != std::string::npos)
                {
                    game.installDir = line.substr(start + 1, end - start - 1);
                }
            }
        }

        file.close();
        return game;
    }

    std::vector<GameInfo> getInstalledGames(const std::string &steamDir)
    {
        std::vector<GameInfo> games;
        std::string steamappsPath = steamDir;
        std::string os = getOperatingSystem();

        if (os == "Windows")
        {
            steamappsPath += "\\steamapps";
        }
        else
        {
            steamappsPath += "/steamapps";
        }

        Logger::log("Scanning for games in: " + steamappsPath);

        if (!directoryExists(steamappsPath))
        {
            Logger::log("Steamapps directory not found: " + steamappsPath);
            return games;
        }

        try
        {
            for (const auto &entry : std::filesystem::directory_iterator(steamappsPath))
            {
                if (entry.is_regular_file())
                {
                    std::string filename = entry.path().filename().string();
                    if (filename.substr(0, 12) == "appmanifest_" &&
                        filename.length() > 4 && filename.substr(filename.length() - 4) == ".acf")
                    {
                        GameInfo game = parseAcfFile(entry.path().string());
                        if (!game.name.empty() && !game.appId.empty())
                        {
                            games.push_back(game);
                            Logger::log("Found game: " + game.name + " (ID: " + game.appId + ")");
                        }
                    }
                }
            }
        }
        catch (const std::filesystem::filesystem_error &e)
        {
            Logger::log("Error scanning steamapps directory: " + std::string(e.what()));
        }

        Logger::log("Found " + std::to_string(games.size()) + " installed games");
        return games;
    }

    const GameInfo *findGameByName(const std::vector<GameInfo> &games, const std::string &gameName)
    {
        std::string lowerGameName = gameName;
        std::transform(lowerGameName.begin(), lowerGameName.end(), lowerGameName.begin(), ::tolower);

        for (const auto &game : games)
        {
            std::string lowerCurrentGame = game.name;
            std::transform(lowerCurrentGame.begin(), lowerCurrentGame.end(), lowerCurrentGame.begin(), ::tolower);

            if (lowerCurrentGame == lowerGameName || lowerCurrentGame.find(lowerGameName) != std::string::npos)
            {
                return &game;
            }
        }
        return nullptr;
    }

    std::vector<std::string> getLogFileExtensions()
    {
        return {".log", ".txt", ".out", ".err", ".crash", ".dmp", ".mdmp", ".rpt",
                ".debug", ".trace", ".console", ".output", ".error"};
    }

    bool isLogFile(const std::string &filename)
    {
        std::string lowerFilename = filename;
        std::transform(lowerFilename.begin(), lowerFilename.end(), lowerFilename.begin(), ::tolower);
        std::vector<std::string> logExtensions = getLogFileExtensions();

        for (const auto &ext : logExtensions)
        {
            if (lowerFilename.length() >= ext.length() &&
                lowerFilename.substr(lowerFilename.length() - ext.length()) == ext)
            {
                return true;
            }
        }

        std::vector<std::string> logPatterns = {
            "log", "crash", "error", "debug", "console", "output",
            "stderr", "stdout", "trace", "dump", "report"};

        for (const auto &pattern : logPatterns)
        {
            if (lowerFilename.find(pattern) != std::string::npos)
            {
                return true;
            }
        }

        return false;
    }

    std::string formatFileSize(std::uintmax_t size)
    {
        const char *units[] = {"B", "KB", "MB", "GB"};
        double fileSize = static_cast<double>(size);
        int unitIndex = 0;

        while (fileSize >= 1024.0 && unitIndex < 3)
        {
            fileSize /= 1024.0;
            unitIndex++;
        }

        std::ostringstream oss;
        oss << std::fixed << std::setprecision(1) << fileSize << " " << units[unitIndex];
        return oss.str();
    }

    std::string formatFileTime(const std::string &filePath)
    {
        try
        {
            auto ftime = std::filesystem::last_write_time(filePath);
            auto sctp = std::chrono::time_point_cast<std::chrono::system_clock::duration>(
                ftime - std::filesystem::file_time_type::clock::now() + std::chrono::system_clock::now());
            auto time_t = std::chrono::system_clock::to_time_t(sctp);

            std::ostringstream oss;
            oss << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S");
            return oss.str();
        }
        catch (const std::exception &e)
        {
            return "Unknown";
        }
    }

    void searchLogsInDirectory(const std::string &directory, std::vector<LogFile> &logFiles,
                               int maxDepth, int currentDepth)
    {
        if (currentDepth >= maxDepth || !directoryExists(directory))
        {
            return;
        }

        try
        {
            for (const auto &entry : std::filesystem::directory_iterator(directory))
            {
                try
                {
                    if (entry.is_regular_file())
                    {
                        std::string filename = entry.path().filename().string();

                        if (isLogFile(filename))
                        {
                            LogFile logFile;
                            logFile.path = entry.path().string();
                            logFile.filename = filename;
                            logFile.size = entry.file_size();
                            logFile.lastModified = formatFileTime(logFile.path);

                            std::string lowerFilename = filename;
                            std::transform(lowerFilename.begin(), lowerFilename.end(),
                                           lowerFilename.begin(), ::tolower);

                            if (lowerFilename.find("crash") != std::string::npos ||
                                lowerFilename.find("dump") != std::string::npos)
                            {
                                logFile.type = "crash_log";
                            }
                            else if (lowerFilename.find("error") != std::string::npos)
                            {
                                logFile.type = "error_log";
                            }
                            else if (lowerFilename.find("debug") != std::string::npos)
                            {
                                logFile.type = "debug_log";
                            }
                            else if (lowerFilename.find("console") != std::string::npos)
                            {
                                logFile.type = "console_log";
                            }
                            else
                            {
                                logFile.type = "game_log";
                            }

                            logFiles.push_back(logFile);
                            Logger::log("Found log file: " + logFile.path + " (" + formatFileSize(logFile.size) + ")");
                        }
                    }
                    else if (entry.is_directory() && currentDepth < maxDepth - 1)
                    {
                        searchLogsInDirectory(entry.path().string(), logFiles, maxDepth, currentDepth + 1);
                    }
                }
                catch (const std::exception &e)
                {
                    continue;
                }
            }
        }
        catch (const std::filesystem::filesystem_error &e)
        {
            Logger::log("Error scanning directory " + directory + ": " + e.what());
        }
    }

    std::vector<LogFile> findGameLogs(const std::string &steamDir, const GameInfo &game)
    {
        std::vector<LogFile> logFiles;
        std::vector<std::string> searchPaths;
        std::string os = getOperatingSystem();
        std::string home = getHomeDirectory();

        Logger::log("Searching for logs for game: " + game.name + " (ID: " + game.appId + ")");

        if (os == "Windows")
        {
            searchPaths.push_back(steamDir + "\\steamapps\\common\\" + game.installDir);
            if (!home.empty())
            {
                searchPaths.push_back(home + "\\AppData\\Local\\" + game.installDir);
                searchPaths.push_back(home + "\\AppData\\Roaming\\" + game.installDir);
                searchPaths.push_back(home + "\\AppData\\Local\\" + game.name);
                searchPaths.push_back(home + "\\AppData\\Roaming\\" + game.name);
                searchPaths.push_back(home + "\\Documents\\My Games\\" + game.installDir);
                searchPaths.push_back(home + "\\Documents\\My Games\\" + game.name);
                searchPaths.push_back(home + "\\Documents\\" + game.name);
                searchPaths.push_back(home + "\\Documents\\" + game.installDir);
            }
        }
        else if (os == "Linux")
        {
            searchPaths.push_back(steamDir + "/steamapps/common/" + game.installDir);

            if (!home.empty())
            {
                searchPaths.push_back(home + "/.local/share/" + game.installDir);
                searchPaths.push_back(home + "/.config/" + game.installDir);
                searchPaths.push_back(home + "/." + game.installDir);
                searchPaths.push_back(home + "/.local/share/" + game.name);
                searchPaths.push_back(home + "/.config/" + game.name);
                searchPaths.push_back(steamDir + "/steamapps/compatdata/" + game.appId + "/pfx/drive_c/users/steamuser/AppData/Local/" + game.installDir);
                searchPaths.push_back(steamDir + "/steamapps/compatdata/" + game.appId + "/pfx/drive_c/users/steamuser/AppData/Roaming/" + game.installDir);
                searchPaths.push_back(steamDir + "/steamapps/compatdata/" + game.appId + "/pfx/drive_c/users/steamuser/Documents/" + game.name);
            }
        }
        else if (os == "macOS")
        {
            searchPaths.push_back(steamDir + "/steamapps/common/" + game.installDir);

            if (!home.empty())
            {
                searchPaths.push_back(home + "/Library/Application Support/" + game.installDir);
                searchPaths.push_back(home + "/Library/Application Support/" + game.name);
                searchPaths.push_back(home + "/Library/Logs/" + game.installDir);
                searchPaths.push_back(home + "/Library/Logs/" + game.name);
                searchPaths.push_back(home + "/Library/Preferences/" + game.installDir);
                searchPaths.push_back(home + "/Library/Preferences/" + game.name);
                searchPaths.push_back(home + "/Documents/" + game.name);
                searchPaths.push_back(home + "/Documents/" + game.installDir);
            }
        }

        std::sort(searchPaths.begin(), searchPaths.end());
        searchPaths.erase(std::unique(searchPaths.begin(), searchPaths.end()), searchPaths.end());

        for (const auto &path : searchPaths)
        {
            Logger::log("Searching in: " + path);
            searchLogsInDirectory(path, logFiles, 3, 0);
        }

        std::sort(logFiles.begin(), logFiles.end(),
                  [](const LogFile &a, const LogFile &b)
                  {
                      return a.lastModified > b.lastModified;
                  });

        Logger::log("Found " + std::to_string(logFiles.size()) + " log files for " + game.name);
        return logFiles;
    }
}