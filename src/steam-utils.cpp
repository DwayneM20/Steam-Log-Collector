#include "steam-utils.hpp"
#include "logger.hpp"
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

    fs::path getHomeDirectory()
    {
#ifdef _WIN32
        char *userProfile = nullptr;
        size_t len = 0;
        if (_dupenv_s(&userProfile, &len, "USERPROFILE") == 0 && userProfile != nullptr)
        {
            fs::path home(userProfile);
            free(userProfile);
            return home;
        }
        return {};
#else
        const char *home = getenv("HOME");
        if (home != nullptr)
        {
            return fs::path(home);
        }
        struct passwd *pw = getpwuid(getuid());
        if (pw != nullptr)
        {
            return fs::path(pw->pw_dir);
        }
        return {};
#endif
    }

    bool directoryExists(const fs::path &path)
    {
        try
        {
            return fs::exists(path) && fs::is_directory(path);
        }
        catch (const fs::filesystem_error &e)
        {
            Logger::log("Error checking directory: " + path.string() + " - " + e.what(), SeverityLevel::Err);
            return false;
        }
    }

    bool isValidSteamDirectory(const fs::path &path)
    {
#ifdef _WIN32
        return fs::exists(path / "steam.exe") ||
               fs::exists(path / "Steam.exe");
#elif defined(__APPLE__)
        return fs::exists(path / "Steam") ||
               fs::exists(path / ".." / "Steam") ||
               fs::exists(path / "steamapps");
#elif defined(__linux__)
        return fs::exists(path / "steam") ||
               fs::exists(path / "steam.sh") ||
               fs::exists(path / "steamapps");
#else
        return false;
#endif
    }

    std::vector<fs::path> getSteamDirectoryPaths()
    {
        std::vector<fs::path> paths;
        fs::path home = getHomeDirectory();

#ifdef _WIN32
        std::vector<char> drives = {'C', 'D', 'E', 'F', 'G', 'H'};

        for (char drive : drives)
        {
            fs::path driveRoot(std::string(1, drive) + ":");
            paths.push_back(driveRoot / "Program Files (x86)" / "Steam");
            paths.push_back(driveRoot / "Program Files" / "Steam");
            paths.push_back(driveRoot / "Steam");
            paths.push_back(driveRoot / "Games" / "Steam");
        }

        if (!home.empty())
        {
            paths.push_back(home / "AppData" / "Local" / "Steam");
            paths.push_back(home / "Steam");
        }

        HKEY hKey;
        if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, TEXT("SOFTWARE\\WOW6432Node\\Valve\\Steam"), 0, KEY_READ, &hKey) == ERROR_SUCCESS ||
            RegOpenKeyEx(HKEY_LOCAL_MACHINE, TEXT("SOFTWARE\\Valve\\Steam"), 0, KEY_READ, &hKey) == ERROR_SUCCESS)
        {
            char installPath[MAX_PATH];
            DWORD bufferSize = sizeof(installPath);
            if (RegQueryValueEx(hKey, TEXT("InstallPath"), NULL, NULL, reinterpret_cast<LPBYTE>(installPath), &bufferSize) == ERROR_SUCCESS)
            {
                paths.insert(paths.begin(), fs::path(installPath));
            }
            RegCloseKey(hKey);
        }
#elif defined(__APPLE__)
        if (!home.empty())
        {
            paths.push_back(home / "Library" / "Application Support" / "Steam");
            paths.push_back(home / ".steam");
            paths.push_back(home / ".local" / "share" / "Steam");
        }
        paths.push_back("/Applications/Steam.app/Contents/MacOS");
#elif defined(__linux__)
        if (!home.empty())
        {
            paths.push_back(home / ".steam" / "steam");
            paths.push_back(home / ".steam");
            paths.push_back(home / ".local" / "share" / "Steam");
            paths.push_back(home / "snap" / "steam" / "common" / ".steam");
            paths.push_back(home / ".var" / "app" / "com.valvesoftware.Steam" / ".steam");
        }
        paths.push_back("/usr/share/steam");
        paths.push_back("/opt/steam");
#endif
        return paths;
    }

    fs::path findSteamDirectory()
    {
        Logger::log("Searching for Steam installation directory...", SeverityLevel::Info);

        std::string os = getOperatingSystem();
        Logger::log("Detected operating system: " + os, SeverityLevel::Info);

        std::vector<fs::path> potentialPaths = getSteamDirectoryPaths();

        for (const auto &path : potentialPaths)
        {
            Logger::log("Checking path: " + path.string(), SeverityLevel::Info);

            if (directoryExists(path))
            {
                if (isValidSteamDirectory(path))
                {
                    Logger::log("Found valid Steam directory: " + path.string(), SeverityLevel::Info);
                    return path;
                }
                else
                {
                    Logger::log("Invalid Steam directory: " + path.string(), SeverityLevel::Warning);
                }
            }
        }
        Logger::log("Steam directory not found in any of the common locations.", SeverityLevel::Warning);
        return {};
    }

    GameInfo parseAcfFile(const fs::path &acfFilePath)
    {
        GameInfo game;
        std::ifstream file(acfFilePath);

        if (!file.is_open())
        {
            Logger::log("Failed to open ACF file: " + acfFilePath.string(), SeverityLevel::Err);
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

    std::vector<GameInfo> getInstalledGames(const fs::path &steamDir)
    {
        std::vector<GameInfo> games;
        fs::path steamappsPath = steamDir / "steamapps";

        Logger::log("Scanning for games in: " + steamappsPath.string(), SeverityLevel::Info);

        if (!directoryExists(steamappsPath))
        {
            Logger::log("Steamapps directory not found: " + steamappsPath.string(), SeverityLevel::Warning);
            return games;
        }

        try
        {
            for (const auto &entry : fs::directory_iterator(steamappsPath))
            {
                if (entry.is_regular_file())
                {
                    std::string filename = entry.path().filename().string();
                    if (filename.substr(0, 12) == "appmanifest_" &&
                        filename.length() > 4 && filename.substr(filename.length() - 4) == ".acf")
                    {
                        GameInfo game = parseAcfFile(entry.path());
                        if (!game.name.empty() && !game.appId.empty())
                        {
                            games.push_back(game);
                            Logger::log("Found game: " + game.name + " (ID: " + game.appId + ")", SeverityLevel::Info);
                        }
                    }
                }
            }
        }
        catch (const fs::filesystem_error &e)
        {
            Logger::log("Error scanning steamapps directory: " + std::string(e.what()), SeverityLevel::Err);
        }

        Logger::log("Found " + std::to_string(games.size()) + " installed games", SeverityLevel::Info);
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

    std::string formatFileTime(const fs::path &filePath)
    {
        try
        {
            auto ftime = fs::last_write_time(filePath);
            auto sctp = std::chrono::time_point_cast<std::chrono::system_clock::duration>(
                ftime - fs::file_time_type::clock::now() + std::chrono::system_clock::now());
            auto time_t = std::chrono::system_clock::to_time_t(sctp);

            std::tm tmBuf{};
#ifdef _WIN32
            localtime_s(&tmBuf, &time_t);
#else
            localtime_r(&time_t, &tmBuf);
#endif
            std::ostringstream oss;
            oss << std::put_time(&tmBuf, "%Y-%m-%d %H:%M:%S");
            return oss.str();
        }
        catch (const std::exception &e)
        {
            return "Unknown";
        }
    }

    void searchLogsInDirectory(const fs::path &directory, std::vector<LogFile> &logFiles,
                               int maxDepth, int currentDepth)
    {
        if (currentDepth >= maxDepth || !directoryExists(directory))
        {
            return;
        }

        try
        {
            for (const auto &entry : fs::directory_iterator(directory))
            {
                try
                {
                    if (entry.is_regular_file())
                    {
                        std::string filename = entry.path().filename().string();

                        if (isLogFile(filename))
                        {
                            LogFile logFile;
                            logFile.path = entry.path();
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
                            Logger::log("Found log file: " + logFile.path.string() + " (" + formatFileSize(logFile.size) + ")", SeverityLevel::Debug);
                        }
                    }
                    else if (entry.is_directory() && currentDepth < maxDepth - 1)
                    {
                        searchLogsInDirectory(entry.path(), logFiles, maxDepth, currentDepth + 1);
                    }
                }
                catch (const std::exception &e)
                {
                    continue;
                }
            }
        }
        catch (const fs::filesystem_error &e)
        {
            Logger::log("Error scanning directory " + directory.string() + ": " + e.what(), SeverityLevel::Err);
        }
    }

    std::vector<LogFile> findGameLogs(const fs::path &steamDir, const GameInfo &game)
    {
        std::vector<LogFile> logFiles;
        std::vector<fs::path> searchPaths;
        fs::path home = getHomeDirectory();

        Logger::log("Searching for logs for game: " + game.name + " (ID: " + game.appId + ")", SeverityLevel::Info);

        searchPaths.push_back(steamDir / "steamapps" / "common" / game.installDir);

#ifdef _WIN32
        if (!home.empty())
        {
            searchPaths.push_back(home / "AppData" / "Local" / game.installDir);
            searchPaths.push_back(home / "AppData" / "Roaming" / game.installDir);
            searchPaths.push_back(home / "AppData" / "Local" / game.name);
            searchPaths.push_back(home / "AppData" / "Roaming" / game.name);
            searchPaths.push_back(home / "Documents" / "My Games" / game.installDir);
            searchPaths.push_back(home / "Documents" / "My Games" / game.name);
            searchPaths.push_back(home / "Documents" / game.name);
            searchPaths.push_back(home / "Documents" / game.installDir);
        }
#elif defined(__linux__)
        if (!home.empty())
        {
            searchPaths.push_back(home / ".local" / "share" / game.installDir);
            searchPaths.push_back(home / ".config" / game.installDir);
            searchPaths.push_back(home / ("." + game.installDir));
            searchPaths.push_back(home / ".local" / "share" / game.name);
            searchPaths.push_back(home / ".config" / game.name);
            fs::path compatdata = steamDir / "steamapps" / "compatdata" / game.appId / "pfx" / "drive_c" / "users" / "steamuser";
            searchPaths.push_back(compatdata / "AppData" / "Local" / game.installDir);
            searchPaths.push_back(compatdata / "AppData" / "Roaming" / game.installDir);
            searchPaths.push_back(compatdata / "Documents" / game.name);
        }
#elif defined(__APPLE__)
        if (!home.empty())
        {
            searchPaths.push_back(home / "Library" / "Application Support" / game.installDir);
            searchPaths.push_back(home / "Library" / "Application Support" / game.name);
            searchPaths.push_back(home / "Library" / "Logs" / game.installDir);
            searchPaths.push_back(home / "Library" / "Logs" / game.name);
            searchPaths.push_back(home / "Library" / "Preferences" / game.installDir);
            searchPaths.push_back(home / "Library" / "Preferences" / game.name);
            searchPaths.push_back(home / "Documents" / game.name);
            searchPaths.push_back(home / "Documents" / game.installDir);
        }
#endif

        std::sort(searchPaths.begin(), searchPaths.end());
        searchPaths.erase(std::unique(searchPaths.begin(), searchPaths.end()), searchPaths.end());

        for (const auto &path : searchPaths)
        {
            Logger::log("Searching in: " + path.string(), SeverityLevel::Info);
            searchLogsInDirectory(path, logFiles, 3, 0);
        }

        std::sort(logFiles.begin(), logFiles.end(),
                  [](const LogFile &a, const LogFile &b)
                  {
                      return a.lastModified > b.lastModified;
                  });

        Logger::log("Found " + std::to_string(logFiles.size()) + " log files for " + game.name, SeverityLevel::Info);
        return logFiles;
    }

    bool createDirectory(const fs::path &path)
    {
        try
        {
            if (fs::exists(path))
            {
                bool isDir = fs::is_directory(path);
                Logger::log("Directory already exists: " + path.string() + " (is_directory: " + (isDir ? "true" : "false") + ")", SeverityLevel::Warning);
                return isDir;
            }

            Logger::log("Attempting to create directory: " + path.string(), SeverityLevel::Info);
            bool success = fs::create_directories(path);

            if (success)
            {
                Logger::log("Successfully created directory: " + path.string(), SeverityLevel::Info);

                // Verify the directory was actually created
                if (fs::exists(path) && fs::is_directory(path))
                {
                    Logger::log("Verified directory creation: " + path.string(), SeverityLevel::Info);
                    return true;
                }
                else
                {
                    Logger::log("Directory creation reported success but verification failed: " + path.string(), SeverityLevel::Warning);
                    return false;
                }
            }
            else
            {
                Logger::log("create_directories returned false for: " + path.string(), SeverityLevel::Warning);
                return false;
            }
        }
        catch (const fs::filesystem_error &e)
        {
            Logger::log("Filesystem error creating directory " + path.string() + ": " + e.what(), SeverityLevel::Err);
            Logger::log("Error code: " + std::to_string(e.code().value()) + " - " + e.code().message(), SeverityLevel::Err);
            return false;
        }
        catch (const std::exception &e)
        {
            Logger::log("General exception creating directory " + path.string() + ": " + e.what(), SeverityLevel::Err);
            return false;
        }
    }

    std::string sanitizeFileName(const std::string &filename)
    {
        std::string sanitized = filename;
        std::string invalidChars = "<>:\"/\\|?*";
        for (char &c : sanitized)
        {
            if (invalidChars.find(c) != std::string::npos || c < 32)
            {
                c = '_';
            }
        }
        sanitized.erase(0, sanitized.find_first_not_of(" ."));
        sanitized.erase(sanitized.find_last_not_of(" .") + 1);

        if (sanitized.empty())
        {
            sanitized = "untitled";
        }
        return sanitized;
    }

    fs::path createOutputDirectory(const std::string &gameName)
    {
        fs::path home = getHomeDirectory();
        if (home.empty())
        {
            Logger::log("Could not determine home directory", SeverityLevel::Err);
            return {};
        }

        Logger::log("Home directory: " + home.string(), SeverityLevel::Info);
        fs::path steamLogDir = home / "steam-logs";

        Logger::log("Attempting to create steam-logs directory: " + steamLogDir.string(), SeverityLevel::Info);

        if (!createDirectory(steamLogDir))
        {
            Logger::log("Standard method failed, trying alternative approach...", SeverityLevel::Warning);
#ifdef _WIN32
            if (CreateDirectoryA(steamLogDir.string().c_str(), NULL) || GetLastError() == ERROR_ALREADY_EXISTS)
            {
                Logger::log("Windows API successfully created/found directory: " + steamLogDir.string(), SeverityLevel::Info);
            }
            else
            {
                DWORD error = GetLastError();
                Logger::log("Windows API failed with error: " + std::to_string(error), SeverityLevel::Err);

                fs::path documentsDir = home / "Documents" / "steam-logs";
                Logger::log("Trying fallback location: " + documentsDir.string(), SeverityLevel::Info);

                if (CreateDirectoryA(documentsDir.string().c_str(), NULL) || GetLastError() == ERROR_ALREADY_EXISTS)
                {
                    steamLogDir = documentsDir;
                    Logger::log("Successfully created fallback directory: " + steamLogDir.string(), SeverityLevel::Info);
                }
                else
                {
                    Logger::log("All directory creation attempts failed", SeverityLevel::Err);
                    return {};
                }
            }
#else
            Logger::log("Failed to create steam-logs directory", SeverityLevel::Err);
            return {};
#endif
        }

        if (!directoryExists(steamLogDir))
        {
            Logger::log("Directory verification failed: " + steamLogDir.string(), SeverityLevel::Err);
            return {};
        }

        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);
        std::tm tmBuf{};
#ifdef _WIN32
        localtime_s(&tmBuf, &time_t);
#else
        localtime_r(&time_t, &tmBuf);
#endif
        std::ostringstream timestamp;
        timestamp << std::put_time(&tmBuf, "%Y%m%d_%H%M%S");

        std::string sanitizedGameName = sanitizeFileName(gameName);
        fs::path gameDir = steamLogDir / (sanitizedGameName + "_" + timestamp.str());

        Logger::log("Creating game-specific directory: " + gameDir.string(), SeverityLevel::Info);

        if (!createDirectory(gameDir))
        {
            Logger::log("Failed to create game directory: " + gameDir.string(), SeverityLevel::Err);
            return {};
        }

        if (!directoryExists(gameDir))
        {
            Logger::log("Game directory verification failed: " + gameDir.string(), SeverityLevel::Err);
            return {};
        }

        Logger::log("Successfully created output directory: " + gameDir.string(), SeverityLevel::Info);
        return gameDir;
    }

    bool copyFile(const fs::path &sourcePath, const fs::path &destPath)
    {
        try
        {
            fs::copy_file(sourcePath, destPath, fs::copy_options::overwrite_existing);
            return true;
        }
        catch (const fs::filesystem_error &e)
        {
            Logger::log("Error copying file from " + sourcePath.string() + " to " + destPath.string() + ": " + e.what(), SeverityLevel::Err);
            return false;
        }
    }

    int copyLogsToDirectory(const std::vector<LogFile> &logFiles, const fs::path &outputDir, const std::string &gameName)
    {
        if (logFiles.empty())
        {
            Logger::log("No log files to copy for game: " + gameName, SeverityLevel::Info);
            return 0;
        }

        Logger::log("Starting to copy " + std::to_string(logFiles.size()) + " log files to: " + outputDir.string(), SeverityLevel::Info);

        int copiedCount = 0;
        fs::path summaryPath = outputDir / "log_summary.txt";

        std::ofstream summaryFile(summaryPath);
        if (summaryFile.is_open())
        {
            auto now = std::chrono::system_clock::now();
            auto time_t = std::chrono::system_clock::to_time_t(now);
            std::tm tmBuf{};
#ifdef _WIN32
            localtime_s(&tmBuf, &time_t);
#else
            localtime_r(&time_t, &tmBuf);
#endif

            summaryFile << "Steam Log Collection Summary\n";
            summaryFile << "============================\n";
            summaryFile << "Game: " << gameName << "\n";
            summaryFile << "Collection Date: " << std::put_time(&tmBuf, "%Y-%m-%d %H:%M:%S") << "\n";
            summaryFile << "Total Files Found: " << logFiles.size() << "\n\n";
            summaryFile << "Files Collected:\n";
            summaryFile << "-----------------\n";
        }

        for (size_t i = 0; i < logFiles.size(); ++i)
        {
            const auto &logFile = logFiles[i];

            std::string destFileName = std::to_string(i + 1) + "_" + sanitizeFileName(logFile.filename);
            destFileName = sanitizeFileName(destFileName);

            fs::path destPath = outputDir / destFileName;

            if (copyFile(logFile.path, destPath))
            {
                copiedCount++;
                Logger::log("Copied: " + logFile.filename + "-->" + destFileName, SeverityLevel::Info);
                if (summaryFile.is_open())
                {
                    summaryFile << "[" << (i + 1) << "] " << destFileName << "\n";
                    summaryFile << "Original: " << logFile.path.string() << "\n";
                    summaryFile << "Type: " << logFile.type << "\n";
                    summaryFile << "Size: " << formatFileSize(logFile.size) << "\n";
                    summaryFile << "Last Modified: " << logFile.lastModified << "\n\n";
                }
            }
            else
            {
                Logger::log("Failed to copy: " + logFile.path.string(), SeverityLevel::Err);
            }
        }

        if (summaryFile.is_open())
        {
            summaryFile << "Successfully copied " << copiedCount << "/" << logFiles.size() << " files\n";
            summaryFile.close();
            Logger::log("Log summary file created: " + summaryPath.string(), SeverityLevel::Info);
        }
        Logger::log("Copy operation completed. " + std::to_string(copiedCount) + " files copied successfully.", SeverityLevel::Info);
        return copiedCount;
    }
}
