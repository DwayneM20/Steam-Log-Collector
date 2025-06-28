#include <iostream>
#include "logger.hpp"
#include "steam-utils.hpp"

int main(int argc, char *argv[])
{
    std::cout << "=== Steam Log Collector CLI ===" << std::endl;

    if (argc < 2)
    {
        std::cerr << "Usage: " << argv[0] << "<steam_game_name>" << std::endl;
        return 1;
    }

    std::cout << "Trying to find Steam directory..." << std::endl;

    std::string steamDir = SteamUtils::findSteamDirectory();
    if (!steamDir.empty())
    {
        Logger::log("Found Steam directory: " + steamDir);
        // log collection goes here
        std::string gameName = argv[1];
        std::cout << "Target game: " << gameName << std::endl;

        Logger::log("Initialized Steam Log Collector for: " + gameName);
    }
    else
    {
        Logger::log("Steam directory not found. Please ensure Steam is installed.");
        std::cerr << "Error: Steam directory not found. Please ensure Steam is installed." << std::endl;
        return 1;
    }

    return 0;
}