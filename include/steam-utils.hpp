#ifndef STEAM_UTILS_HPP
#define STEAM_UTILS_HPP

#include <string>
#include <vector>

namespace SteamUtils
{
    /**
     * @brief Finds the Steam installation directory on the current system
     * @return Path to Steam directory if found, empty string otherwise
     */
    std::string findSteamDirectory();

    /**
     * @brief Gets all possible Steam directories locations for the current OS
     * @return Vector of paths where Steam might be installed
     */
    std::vector<std::string> getSteamDirectoryPaths();

    /**
     * @brief Checks if a directory exists and is accessible
     * @param path The directory path to check
     * @return True if the directory exists and is accessible, false otherwise
     */
    bool directoryExists(const std::string &path);

    /**
     * @brief Gets the user's home directory path
     * @return Path to the user's home directory
     */
    std::string getHomeDirectory();

    /**
     * @brief Detects the operating system type
     * @return A string representing the OS type (e.g., "Windows", "Linux", "macOS")
     */

    std::string getOperatingSystem();

}
#endif