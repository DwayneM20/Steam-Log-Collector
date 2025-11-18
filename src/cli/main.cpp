#include <iostream>
#include <string>
#include <iomanip>
#include "logger.hpp"
#include "steam-utils.hpp"

int main(int argc, char *argv[])
{
    std::cout << "=== Steam Log Collector CLI ===" << std::endl;

    if (argc < 2)
    {
        std::cerr << "Usage: " << argv[0] << " <steam_game_name> [steam_directory]" << std::endl;
        std::cerr << "   or: " << argv[0] << " --list [steam_directory]" << std::endl;
        return 1;
    }

    std::string steamDir;
    bool listMode = (std::string(argv[1]) == "--list");

    int steamDirArgIndex = listMode ? 2 : 2;
    if (argc > steamDirArgIndex)
    {
        steamDir = argv[steamDirArgIndex];
        std::cout << "Using provided Steam directory: " << steamDir << std::endl;

        if (!SteamUtils::directoryExists(steamDir))
        {
            std::cerr << "Error: Provided Steam directory does not exist: " << steamDir << std::endl;
            return 1;
        }

        if (!SteamUtils::isValidSteamDirectory(steamDir))
        {
            std::cerr << "Error: Provided directory is not a valid Steam installation: " << steamDir << std::endl;
            return 1;
        }
    }
    else
    {
        std::cout << "Trying to find Steam directory..." << std::endl;
        steamDir = SteamUtils::findSteamDirectory();

        if (steamDir.empty())
        {
            Logger::log("Steam directory not found. Please ensure Steam is installed.");
            std::cerr << "Error: Steam directory not found. Please ensure Steam is installed." << std::endl;
            std::cout << "You can also specify the Steam directory manually:" << std::endl;
            std::cout << "Usage: " << argv[0] << " <steam_game_name> <steam_directory>" << std::endl;
            return 1;
        }
    }

    Logger::log("Found Steam directory: " + steamDir);

    std::cout << "Scanning for installed games..." << std::endl;
    std::vector<SteamUtils::GameInfo> games = SteamUtils::getInstalledGames(steamDir);

    if (games.empty())
    {
        std::cerr << "No games found in Steam directory." << std::endl;
        return 1;
    }

    std::cout << "\n=== Installed Steam Games ===" << std::endl;
    for (const auto &game : games)
    {
        std::cout << game.name << std::endl;
    }
    std::cout << "\nTotal games found: " << games.size() << std::endl;

    if (listMode)
    {
        return 0;
    }

    std::string gameName;
    if (!listMode)
    {
        gameName = argv[1];
    }
    else
    {
        std::cout << "\nEnter the name of the game you want to collect logs for: ";
        std::getline(std::cin, gameName);
    }

    const SteamUtils::GameInfo *foundGame = SteamUtils::findGameByName(games, gameName);

    if (foundGame == nullptr)
    {
        std::cerr << "Game not found: " << gameName << std::endl;
        std::cout << "Please make sure the game name matches one from the list above." << std::endl;
        return 1;
    }

    std::cout << "\n=== Selected Game ===" << std::endl;
    std::cout << "Name: " << foundGame->name << std::endl;
    std::cout << "App ID: " << foundGame->appId << std::endl;
    std::cout << "Install Directory: " << foundGame->installDir << std::endl;

    Logger::log("Initialized Steam Log Collector for: " + foundGame->name + " (ID: " + foundGame->appId + ")");

    std::cout << "\nSearching for log files..." << std::endl;
    std::vector<SteamUtils::LogFile> logFiles = SteamUtils::findGameLogs(steamDir, *foundGame);

    if (logFiles.empty())
    {
        std::cout << "No log files found for " << foundGame->name << std::endl;
        Logger::log("No log files found for " + foundGame->name);
        return 0;
    }

    std::cout << "\n=== Found Log Files ===" << std::endl;
    std::cout << std::left << std::setw(50) << "File Name"
              << std::setw(15) << "Type"
              << std::setw(12) << "Size"
              << std::setw(20) << "Last Modified" << std::endl;
    std::cout << std::string(97, '-') << std::endl;

    for (size_t i = 0; i < logFiles.size(); ++i)
    {
        const auto &logFile = logFiles[i];
        std::cout << std::left << std::setw(50) << logFile.filename
                  << std::setw(15) << logFile.type
                  << std::setw(12) << SteamUtils::formatFileSize(logFile.size)
                  << std::setw(20) << logFile.lastModified << std::endl;
    }

    std::cout << "\nTotal log files found: " << logFiles.size() << std::endl;

    std::cout << "\n=== Full Paths ===" << std::endl;
    for (size_t i = 0; i < logFiles.size(); ++i)
    {
        std::cout << "[" << (i + 1) << "] " << logFiles[i].path << std::endl;
    }

    std::cout << "\nDo you want to copy these log files to ~/steam-logs? (y/n): ";
    std::string response;
    std::getline(std::cin, response);

    if (response == "y" || response == "Y" || response == "yes" || response == "Yes")
    {
        std::cout << "\nCreating output directory..." << std::endl;
        std::string outputDir = SteamUtils::createOutputDirectory(foundGame->name);

        if (outputDir.empty())
        {
            std::cerr << "Failed to create output directory. Cannot proceed with copying log files." << std::endl;
            return 1;
        }

        std::cout << "Copying Log Files..." << std::endl;
        int copiedFiles = SteamUtils::copyLogsToDirectory(logFiles, outputDir, foundGame->name);

        if (copiedFiles > 0)
        {
            std::cout << "\n=== Copy Complete ===" << std::endl;
            std::cout << "Successfully copied " << copiedFiles << " out of " << logFiles.size() << " log files" << std::endl;
            std::cout << "Output Directory: " << outputDir << std::endl;
            std::cout << "A summary file (log_summary.txt) has been created with details of all copied files." << std::endl;
        }
        else
        {
            std::cerr << "Failed to copy any log files" << std::endl;
            return 1;
        }
    }
    else
    {
        std::cout << "Log files were not copied." << std::endl;
    }

    return 0;
}