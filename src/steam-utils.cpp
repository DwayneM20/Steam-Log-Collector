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
            Logger::log("Error checking directory: " + path + " - " + e.what(), SEVERITY_LEVEL::ERR);
            return false;
        }
    }

    bool isValidSteamDirectory(const std::string &path)
    {
        namespace fs = std::filesystem;
        fs::path dir(path);

#ifdef _WIN32
        return fs::exists(dir / "steam.exe") ||
               fs::exists(dir / "Steam.exe");
#elif defined(__APPLE__)
        return fs::exists(dir / "Steam") ||
               fs::exists(dir / ".." / "Steam") ||
               fs::exists(dir / "steamapps");
#elif defined(__linux__)
        return fs::exists(dir / "steam") ||
               fs::exists(dir / "steam.sh") ||
               fs::exists(dir / "steamapps");
#else
        return false;
#endif
    }

    std::vector<std::string> getSteamDirectoryPaths()
    {
        namespace fs = std::filesystem;
        std::vector<std::string> paths;
        std::string home = getHomeDirectory();

#ifdef _WIN32
        std::vector<char> drives = {'C', 'D', 'E', 'F', 'G', 'H'};

        for (char drive : drives)
        {
            fs::path driveRoot = fs::path(std::string(1, drive) + ":");
            paths.push_back((driveRoot / "Program Files (x86)" / "Steam").string());
            paths.push_back((driveRoot / "Program Files" / "Steam").string());
            paths.push_back((driveRoot / "Steam").string());
            paths.push_back((driveRoot / "Games" / "Steam").string());
        }

        if (!home.empty())
        {
            fs::path h(home);
            paths.push_back((h / "AppData" / "Local" / "Steam").string());
            paths.push_back((h / "Steam").string());
        }

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
#elif defined(__APPLE__)
        if (!home.empty())
        {
            fs::path h(home);
            paths.push_back((h / "Library" / "Application Support" / "Steam").string());
            paths.push_back((h / ".steam").string());
            paths.push_back((h / ".local" / "share" / "Steam").string());
        }
        paths.push_back("/Applications/Steam.app/Contents/MacOS");
#elif defined(__linux__)
        if (!home.empty())
        {
            fs::path h(home);
            paths.push_back((h / ".steam" / "steam").string());
            paths.push_back((h / ".steam").string());
            paths.push_back((h / ".local" / "share" / "Steam").string());
            paths.push_back((h / "snap" / "steam" / "common" / ".steam").string());
            paths.push_back((h / ".var" / "app" / "com.valvesoftware.Steam" / ".steam").string());
        }
        paths.push_back("/usr/share/steam");
        paths.push_back("/opt/steam");
#endif
        return paths;
    }

    std::string findSteamDirectory()
    {
        Logger::log("Searching for Steam installation directory...", SEVERITY_LEVEL::INFO);

        std::string os = getOperatingSystem();
        Logger::log("Detected operating system: " + os, SEVERITY_LEVEL::INFO);

        std::vector<std::string> potentialPaths = getSteamDirectoryPaths();

        for (const auto &path : potentialPaths)
        {
            Logger::log("Checking path: " + path, SEVERITY_LEVEL::INFO);

            if (directoryExists(path))
            {
                if (isValidSteamDirectory(path))
                {
                    Logger::log("Found valid Steam directory: " + path, SEVERITY_LEVEL::INFO);
                    return path;
                }
                else
                {
                    Logger::log("Invalid Steam directory: " + path, SEVERITY_LEVEL::WARNING);
                }
            }
        }
        Logger::log("Steam directory not found in any of the common locations.", SEVERITY_LEVEL::WARNING);
        return "";
    }

    GameInfo parseAcfFile(const std::string &acfFilePath)
    {
        GameInfo game;
        std::ifstream file(acfFilePath);

        if (!file.is_open())
        {
            Logger::log("Failed to open ACF file: " + acfFilePath, SEVERITY_LEVEL::ERR);
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
        std::string steamappsPath = (std::filesystem::path(steamDir) / "steamapps").string();

        Logger::log("Scanning for games in: " + steamappsPath, SEVERITY_LEVEL::INFO);

        if (!directoryExists(steamappsPath))
        {
            Logger::log("Steamapps directory not found: " + steamappsPath, SEVERITY_LEVEL::WARNING);
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
                            Logger::log("Found game: " + game.name + " (ID: " + game.appId + ")", SEVERITY_LEVEL::INFO);
                        }
                    }
                }
            }
        }
        catch (const std::filesystem::filesystem_error &e)
        {
            Logger::log("Error scanning steamapps directory: " + std::string(e.what()), SEVERITY_LEVEL::ERR);
        }

        Logger::log("Found " + std::to_string(games.size()) + " installed games", SEVERITY_LEVEL::INFO);
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
                            Logger::log("Found log file: " + logFile.path + " (" + formatFileSize(logFile.size) + ")", SEVERITY_LEVEL::DEBUG);
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
            Logger::log("Error scanning directory " + directory + ": " + e.what(), SEVERITY_LEVEL::ERR);
        }
    }

    std::vector<LogFile> findGameLogs(const std::string &steamDir, const GameInfo &game)
    {
        namespace fs = std::filesystem;
        std::vector<LogFile> logFiles;
        std::vector<std::string> searchPaths;
        std::string home = getHomeDirectory();

        Logger::log("Searching for logs for game: " + game.name + " (ID: " + game.appId + ")", SEVERITY_LEVEL::INFO);

        fs::path steam(steamDir);
        searchPaths.push_back((steam / "steamapps" / "common" / game.installDir).string());

#ifdef _WIN32
        if (!home.empty())
        {
            fs::path h(home);
            searchPaths.push_back((h / "AppData" / "Local" / game.installDir).string());
            searchPaths.push_back((h / "AppData" / "Roaming" / game.installDir).string());
            searchPaths.push_back((h / "AppData" / "Local" / game.name).string());
            searchPaths.push_back((h / "AppData" / "Roaming" / game.name).string());
            searchPaths.push_back((h / "Documents" / "My Games" / game.installDir).string());
            searchPaths.push_back((h / "Documents" / "My Games" / game.name).string());
            searchPaths.push_back((h / "Documents" / game.name).string());
            searchPaths.push_back((h / "Documents" / game.installDir).string());
        }
#elif defined(__linux__)
        if (!home.empty())
        {
            fs::path h(home);
            searchPaths.push_back((h / ".local" / "share" / game.installDir).string());
            searchPaths.push_back((h / ".config" / game.installDir).string());
            fs::path dotGame = h / ("." + game.installDir);
            searchPaths.push_back(dotGame.string());
            searchPaths.push_back((h / ".local" / "share" / game.name).string());
            searchPaths.push_back((h / ".config" / game.name).string());
            fs::path compatdata = steam / "steamapps" / "compatdata" / game.appId / "pfx" / "drive_c" / "users" / "steamuser";
            searchPaths.push_back((compatdata / "AppData" / "Local" / game.installDir).string());
            searchPaths.push_back((compatdata / "AppData" / "Roaming" / game.installDir).string());
            searchPaths.push_back((compatdata / "Documents" / game.name).string());
        }
#elif defined(__APPLE__)
        if (!home.empty())
        {
            fs::path h(home);
            searchPaths.push_back((h / "Library" / "Application Support" / game.installDir).string());
            searchPaths.push_back((h / "Library" / "Application Support" / game.name).string());
            searchPaths.push_back((h / "Library" / "Logs" / game.installDir).string());
            searchPaths.push_back((h / "Library" / "Logs" / game.name).string());
            searchPaths.push_back((h / "Library" / "Preferences" / game.installDir).string());
            searchPaths.push_back((h / "Library" / "Preferences" / game.name).string());
            searchPaths.push_back((h / "Documents" / game.name).string());
            searchPaths.push_back((h / "Documents" / game.installDir).string());
        }
#endif

        std::sort(searchPaths.begin(), searchPaths.end());
        searchPaths.erase(std::unique(searchPaths.begin(), searchPaths.end()), searchPaths.end());

        for (const auto &path : searchPaths)
        {
            Logger::log("Searching in: " + path, SEVERITY_LEVEL::INFO);
            searchLogsInDirectory(path, logFiles, 3, 0);
        }

        std::sort(logFiles.begin(), logFiles.end(),
                  [](const LogFile &a, const LogFile &b)
                  {
                      return a.lastModified > b.lastModified;
                  });

        Logger::log("Found " + std::to_string(logFiles.size()) + " log files for " + game.name, SEVERITY_LEVEL::INFO);
        return logFiles;
    }
    bool createDirectory(const std::string &path)
    {
        try
        {
            if (std::filesystem::exists(path))
            {
                bool isDir = std::filesystem::is_directory(path);
                Logger::log("Directory already exists: " + path + " (is_directory: " + (isDir ? "true" : "false") + ")", SEVERITY_LEVEL::WARNING);
                return isDir;
            }

            Logger::log("Attempting to create directory: " + path, SEVERITY_LEVEL::INFO);
            bool success = std::filesystem::create_directories(path);

            if (success)
            {
                Logger::log("Successfully created directory: " + path, SEVERITY_LEVEL::INFO);

                // Verify the directory was actually created
                if (std::filesystem::exists(path) && std::filesystem::is_directory(path))
                {
                    Logger::log("Verified directory creation: " + path, SEVERITY_LEVEL::INFO);
                    return true;
                }
                else
                {
                    Logger::log("Directory creation reported success but verification failed: " + path, SEVERITY_LEVEL::WARNING);
                    return false;
                }
            }
            else
            {
                Logger::log("create_directories returned false for: " + path, SEVERITY_LEVEL::WARNING);
                return false;
            }
        }
        catch (const std::filesystem::filesystem_error &e)
        {
            Logger::log("Filesystem error creating directory " + path + ": " + e.what(), SEVERITY_LEVEL::ERR);
            Logger::log("Error code: " + std::to_string(e.code().value()) + " - " + e.code().message(), SEVERITY_LEVEL::ERR);
            return false;
        }
        catch (const std::exception &e)
        {
            Logger::log("General exception creating directory " + path + ": " + e.what(), SEVERITY_LEVEL::ERR);
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

    std::string createOutputDirectory(const std::string &gameName)
    {
        std::string home = getHomeDirectory();
        if (home.empty())
        {
            Logger::log("Could not determine home directory", SEVERITY_LEVEL::ERR);
            return "";
        }

        namespace fs = std::filesystem;
        Logger::log("Home directory: " + home, SEVERITY_LEVEL::INFO);
        std::string steamLogDir = (fs::path(home) / "steam-logs").string();

        Logger::log("Attempting to create steam-logs directory: " + steamLogDir, SEVERITY_LEVEL::INFO);

        if (!createDirectory(steamLogDir))
        {
            Logger::log("Standard method failed, trying alternative approach...", SEVERITY_LEVEL::WARNING);
#ifdef _WIN32
            if (CreateDirectoryA(steamLogDir.c_str(), NULL) || GetLastError() == ERROR_ALREADY_EXISTS)
            {
                Logger::log("Windows API successfully created/found directory: " + steamLogDir, SEVERITY_LEVEL::INFO);
            }
            else
            {
                DWORD error = GetLastError();
                Logger::log("Windows API failed with error: " + std::to_string(error), SEVERITY_LEVEL::ERR);

                std::string documentsDir = (fs::path(home) / "Documents" / "steam-logs").string();
                Logger::log("Trying fallback location: " + documentsDir, SEVERITY_LEVEL::INFO);

                if (CreateDirectoryA(documentsDir.c_str(), NULL) || GetLastError() == ERROR_ALREADY_EXISTS)
                {
                    steamLogDir = documentsDir;
                    Logger::log("Successfully created fallback directory: " + steamLogDir, SEVERITY_LEVEL::INFO);
                }
                else
                {
                    Logger::log("All directory creation attempts failed", SEVERITY_LEVEL::ERR);
                    return "";
                }
            }
#else
            Logger::log("Failed to create steam-logs directory", SEVERITY_LEVEL::ERR);
            return "";
#endif
        }

        if (!directoryExists(steamLogDir))
        {
            Logger::log("Directory verification failed: " + steamLogDir, SEVERITY_LEVEL::ERR);
            return "";
        }

        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);
        std::ostringstream timestamp;
        timestamp << std::put_time(std::localtime(&time_t), "%Y%m%d_%H%M%S");

        std::string sanitizedGameName = sanitizeFileName(gameName);
        std::string gameDir = (fs::path(steamLogDir) / (sanitizedGameName + "_" + timestamp.str())).string();

        Logger::log("Creating game-specific directory: " + gameDir, SEVERITY_LEVEL::INFO);

        if (!createDirectory(gameDir))
        {
            Logger::log("Failed to create game directory: " + gameDir, SEVERITY_LEVEL::ERR);
            return "";
        }

        if (!directoryExists(gameDir))
        {
            Logger::log("Game directory verification failed: " + gameDir, SEVERITY_LEVEL::ERR);
            return "";
        }

        Logger::log("Successfully created output directory: " + gameDir, SEVERITY_LEVEL::INFO);
        return gameDir;
    }

    bool copyFile(const std::string &sourcePath, const std::string &destPath)
    {
        try
        {
            std::filesystem::copy_file(sourcePath, destPath,
                                       std::filesystem::copy_options::overwrite_existing);
            return true;
        }
        catch (const std::filesystem::filesystem_error &e)
        {
            Logger::log("Error copying file from " + sourcePath + " to " + destPath + ": " + e.what(), SEVERITY_LEVEL::ERR);
            return false;
        }
    }

    int copyLogsToDirectory(const std::vector<LogFile> &logFiles, const std::string &outputDir, const std::string &gameName)
    {
        if (logFiles.empty())
        {
            Logger::log("No log files to copy for game: " + gameName, SEVERITY_LEVEL::INFO);
            return 0;
        }

        Logger::log("Starting to copy " + std::to_string(logFiles.size()) + " log files to: " + outputDir, SEVERITY_LEVEL::INFO);

        namespace fs = std::filesystem;
        int copiedCount = 0;
        std::string summaryPath = (fs::path(outputDir) / "log_summary.txt").string();

        std::ofstream summaryFile(summaryPath);
        if (summaryFile.is_open())
        {
            auto now = std::chrono::system_clock::now();
            auto time_t = std::chrono::system_clock::to_time_t(now);

            summaryFile << "Steam Log Collection Summary\n";
            summaryFile << "============================\n";
            summaryFile << "Game: " << gameName << "\n";
            summaryFile << "Collection Date: " << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S") << "\n";
            summaryFile << "Total Files Found: " << logFiles.size() << "\n\n";
            summaryFile << "Files Collected:\n";
            summaryFile << "-----------------\n";
        }

        for (size_t i = 0; i < logFiles.size(); ++i)
        {
            const auto &logFile = logFiles[i];

            std::string destFileName = std::to_string(i + 1) + "_" + sanitizeFileName(logFile.filename);
            destFileName = sanitizeFileName(destFileName);

            std::string destPath = (fs::path(outputDir) / destFileName).string();

            if (copyFile(logFile.path, destPath))
            {
                copiedCount++;
                Logger::log("Copied: " + logFile.filename + "-->" + destFileName, SEVERITY_LEVEL::INFO);
                if (summaryFile.is_open())
                {
                    summaryFile << "[" << (i + 1) << "] " << destFileName << "\n";
                    summaryFile << "Original: " << logFile.path << "\n";
                    summaryFile << "Type: " << logFile.type << "\n";
                    summaryFile << "Size: " << formatFileSize(logFile.size) << "\n";
                    summaryFile << "Last Modified: " << logFile.lastModified << "\n\n";
                }
            }
            else
            {
                Logger::log("Failed to copy: " + logFile.path, SEVERITY_LEVEL::ERR);
            }
        }

        if (summaryFile.is_open())
        {
            summaryFile << "Successfully copied " << copiedCount << "/" << logFiles.size() << " files\n";
            summaryFile.close();
            Logger::log("Log summary file created: " + summaryPath, SEVERITY_LEVEL::INFO);
        }
        Logger::log("Copy operation completed. " + std::to_string(copiedCount) + " files copied successfully.", SEVERITY_LEVEL::INFO);
        return copiedCount;
    }
}