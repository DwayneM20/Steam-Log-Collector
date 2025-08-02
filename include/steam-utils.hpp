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
     * @brief Validates if a given directory is a valid steam directory
     * @param path The directory path to validate
     * @return True if the directory is a valid Steam directory, false otherwise
     */
    bool isValidSteamDirectory(const std::string &path);

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

    /**
     * @brief Structure to hold game information
     */

    struct GameInfo
    {
        std::string name;
        std::string appId;
        std::string installDir;
    };
    /**
     * @brief Gets list of all installed Steam games
     * @param steamDir Path to the Steam installation directory
     * @return Vector of GameInfo structures containing game details
     */
    std::vector<GameInfo> getInstalledGames(const std::string &steamDir);

    /**
     * @brief Parses an ACF file to extract game information
     * @param acfFilePath Path to the .acf file
     * @return GameInfo structure with game details, empty if parsing fails
     */
    GameInfo parseAcfFile(const std::string &acfFilePath);

    /**
     * @brief Finds a game by name (case-insensitive search)
     * @param games Vector of games to search through
     * @param gameName Name of the game to find
     * @return Pointer to GameInfo if found, nullptr otherwise
     */
    const GameInfo *findGameByName(const std::vector<GameInfo> &games, const std::string &gameName);

    struct LogFile
    {
        std::string path;
        std::string filename;
        std::uintmax_t size;
        std::string lastModified;
        std::string type;
    };

    /**
     * @brief Finds all log files for a specific game
     * @param steamDir Path to Steam installation directory
     * @param game GameInfo structure for the target game
     * @return Vector of LogFile structures containing log file details
     */
    std::vector<LogFile> findGameLogs(const std::string &steamDir, const GameInfo &game);

    /**
     * @brief Gets common log file extensions
     * @return Vector of common log file extensions
     */
    std::vector<std::string> getLogFileExtensions();

    /**
     * @brief Checks if a file is likely a log file based on name and extension
     * @param filename Name of the file to check
     * @return True if the file appears to be a log file
     */
    bool isLogFile(const std::string &filename);

    /**
     * @brief Recursively searches for log files in a directory
     * @param directory Directory to search
     * @param logFiles Vector to store found log files
     * @param maxDepth Maximum recursion depth (default: 3)
     * @param currentDepth Current recursion depth (internal use)
     */
    void searchLogsInDirectory(const std::string &directory, std::vector<LogFile> &logFiles,
                               int maxDepth = 3, int currentDepth = 0);

    /**
     * @brief Formats file size in human readable format
     * @param size File size in bytes
     * @return Formatted size string (e.g., "1.5 MB")
     */
    std::string formatFileSize(std::uintmax_t size);

    /**
     * @brief Formats file modification time
     * @param filePath Path to the file
     * @return Formatted date/time string
     */
    std::string formatFileTime(const std::string &filePath);

    /**
     * @brief Creates the output directory for copied logs
     * @param gameName Name of the game
     * @return Path to the created output directory, empty string if creation failed
     */
    std::string createOutputDirectory(const std::string &gameName);

    /**
     * @brief Copies log files to the output directory
     * @param logFiles Vector of log files to copy
     * @param outputDir Directory to copy files to
     * @param gameName Name of the game
     * @return Number of files successfully copied
     */
    int copyLogsToDirectory(const std::vector<LogFile> &logFiles, const std::string &outputDir, const std::string &gameName);

    /**
     * @brief Safely copies a single file
     * @param sourcePath source file path
     * @param destPath destination file path
     * @return True if copy was successful, false otherwise
     */
    bool copyFile(const std::string &sourcePath, const std::string &destPath);

    /**
     * @brief Creates a directory recursively if it does not exist
     * @param path Directory path to create
     * @return True if the directory was created or already exists, false otherwise
     */
    bool createDirectory(const std::string &path);

    /**
     * @brief Sanitizes a file name for safe file system usage
     * @param filename The original file name
     * @return Sanitized file name safe for file system usage
     */
    std::string sanitizeFileName(const std::string &filename);
}
#endif