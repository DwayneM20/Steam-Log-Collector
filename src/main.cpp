#include <iostream>
#include "logger.hpp"

int main(int argc, char *argv[])
{
    std::cout << "=== Steam Log Collector CLI ===" << std::endl;

    if (argc < 2)
    {
        std::cerr << "Usage: " << argv[0] << "<steam_game_name>" << std::endl;
        return 1;
    }

    std::string gameName = argv[1];
    std::cout << "Target game: " << gameName << std::endl;

    Logger::log("Initialized Steam Log Collector for: " + gameName);

    return 0;
}