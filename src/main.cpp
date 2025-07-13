#include <iostream>
#include "logger.hpp"
#include "steam-utils.hpp"

int main(int argc, char *argv[])
{
    std::cout << "=== Steam Log Collector CLI ===" << std::endl;

    if (argc < 2)
    {
        std::cerr << "Usage: " << argv[0] << "<steam_game_name>  [steam_directory]" << std::endl;
        return 1;
    }

    std::string steamDir;
    std::string gameName = argv[1];

    if (argc >= 3)
    {
        steamDir = argv[2];
        std::cout << "Using provided Steam directory: " << steamDir << std::endl;
        if (!SteamUtils::directoryExists(steamDir))
        {
            std::cerr << "Error: Provided Steam directory does not exist or is not accessible." << std::endl;
            return 1;
        }
        if (!SteamUtils::isValidSteamDirectory(steamDir))
        {
            std::cerr << "Error: Provided directory is not a valid Steam directory." << std::endl;
            return 1;
        }
    }
    else
    {
        std::cout << "No Steam directory provided, searching for default installation..." << std::endl;
        steamDir = SteamUtils::findSteamDirectory();
        if (steamDir.empty())
        {
            Logger::log("Steam directory not found. Please ensure Steam is installed.");
            std::cerr << "Error: Steam directory not found. Please ensure Steam is installed." << std::endl;
            std::cout << "You can provide the Steam directory as an argument." << std::endl;
            std::cout << "Usage: " << argv[0] << " <steam_game_name> [steam_directory]" << std::endl;
            return 1;
        }
    }
    Logger::log("Found steam directory: " + steamDir);
    std::cout << "Target game: " << gameName << std::endl;
    Logger::log("Initializing log collection for game: " + gameName);

    return 0;
}