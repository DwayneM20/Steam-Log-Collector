#include <iostream>
#include <filesystem>
#include <optional>
#include <string>
#include <iomanip>
#include "logger.hpp"
#include "steam-utils.hpp"

namespace fs = std::filesystem;

int main(int argc, char *argv[])
{
    std::cout << "=== Steam Log Collector CLI ===" << '\n';

    if (argc < 2)
    {
        std::cerr << "Usage: " << argv[0] << " <steam_game_name> [steam_directory]" << '\n';
        std::cerr << "   or: " << argv[0] << " --list [steam_directory]" << '\n';
        return 1;
    }

    fs::path steamDir;
    bool listMode = (std::string(argv[1]) == "--list");

    if (argc > 2)
    {
        steamDir = argv[2];
        std::cout << "Using provided Steam directory: " << steamDir.string() << '\n';

        if (!SteamUtils::directoryExists(steamDir))
        {
            std::cerr << "Error: Provided Steam directory does not exist: " << steamDir.string() << '\n';
            return 1;
        }

        if (!SteamUtils::isValidSteamDirectory(steamDir))
        {
            std::cerr << "Error: Provided directory is not a valid Steam installation: " << steamDir.string() << '\n';
            return 1;
        }
    }
    else
    {
        std::cout << "Trying to find Steam directory..." << '\n';
        steamDir = SteamUtils::findSteamDirectory();

        if (steamDir.empty())
        {
            Logger::log("Steam directory not found. Please ensure Steam is installed.", SeverityLevel::Err);
            std::cerr << "Error: Steam directory not found. Please ensure Steam is installed." << '\n';
            std::cout << "You can also specify the Steam directory manually:" << '\n';
            std::cout << "Usage: " << argv[0] << " <steam_game_name> <steam_directory>" << '\n';
            return 1;
        }
    }

    Logger::log("Found Steam directory: " + steamDir.string(), SeverityLevel::Info);

    std::cout << "Scanning for installed games..." << '\n';
    std::vector<SteamUtils::GameInfo> games = SteamUtils::getInstalledGames(steamDir);

    if (games.empty())
    {
        std::cerr << "No games found in Steam directory." << '\n';
        return 1;
    }

    std::cout << "\n=== Installed Steam Games ===" << '\n';
    for (const auto &game : games)
    {
        std::cout << game.name << '\n';
    }
    std::cout << "\nTotal games found: " << games.size() << '\n';

    if (listMode)
    {
        return 0;
    }

    std::string gameName = argv[1];

    std::optional<SteamUtils::GameInfo> foundGame = SteamUtils::findGameByName(games, gameName);

    if (!foundGame)
    {
        std::cerr << "Game not found: " << gameName << '\n';
        std::cout << "Please make sure the game name matches one from the list above." << '\n';
        return 1;
    }

    std::cout << "\n=== Selected Game ===" << '\n';
    std::cout << "Name: " << foundGame->name << '\n';
    std::cout << "App ID: " << foundGame->appId << '\n';
    std::cout << "Install Directory: " << foundGame->installDir << '\n';

    Logger::log("Initialized Steam Log Collector for: " + foundGame->name + " (ID: " + foundGame->appId + ")", SeverityLevel::Info);

    std::cout << "\nSearching for log files..." << '\n';
    std::vector<SteamUtils::LogFile> logFiles = SteamUtils::findGameLogs(steamDir, *foundGame);

    if (logFiles.empty())
    {
        std::cout << "No log files found for " << foundGame->name << '\n';
        Logger::log("No log files found for " + foundGame->name, SeverityLevel::Warning);
        return 0;
    }

    std::cout << "\n=== Found Log Files ===" << '\n';
    std::cout << std::left << std::setw(50) << "File Name"
              << std::setw(15) << "Type"
              << std::setw(12) << "Size"
              << std::setw(20) << "Last Modified" << '\n';
    std::cout << std::string(97, '-') << '\n';

    for (size_t i = 0; i < logFiles.size(); ++i)
    {
        const auto &logFile = logFiles[i];
        std::cout << std::left << std::setw(50) << logFile.filename
                  << std::setw(15) << logFile.type
                  << std::setw(12) << SteamUtils::formatFileSize(logFile.size)
                  << std::setw(20) << logFile.lastModified << '\n';
    }

    std::cout << "\nTotal log files found: " << logFiles.size() << '\n';

    std::cout << "\n=== Full Paths ===" << '\n';
    for (size_t i = 0; i < logFiles.size(); ++i)
    {
        std::cout << "[" << (i + 1) << "] " << logFiles[i].path.string() << '\n';
    }

    std::cout << "\nDo you want to copy these log files to ~/steam-logs? (y/n): ";
    std::string response;
    std::getline(std::cin, response);

    if (response == "y" || response == "Y" || response == "yes" || response == "Yes")
    {
        std::cout << "\nCreating output directory..." << '\n';
        fs::path outputDir = SteamUtils::createOutputDirectory(foundGame->name);

        if (outputDir.empty())
        {
            std::cerr << "Failed to create output directory. Cannot proceed with copying log files." << '\n';
            return 1;
        }

        std::cout << "Copying Log Files..." << '\n';
        int copiedFiles = SteamUtils::copyLogsToDirectory(logFiles, outputDir, foundGame->name);

        if (copiedFiles > 0)
        {
            std::cout << "\n=== Copy Complete ===" << '\n';
            std::cout << "Successfully copied " << copiedFiles << " out of " << logFiles.size() << " log files" << '\n';
            std::cout << "Output Directory: " << outputDir.string() << '\n';
            std::cout << "A summary file (log_summary.txt) has been created with details of all copied files." << '\n';
        }
        else
        {
            std::cerr << "Failed to copy any log files" << '\n';
            return 1;
        }
    }
    else
    {
        std::cout << "Log files were not copied." << '\n';
    }

    return 0;
}
