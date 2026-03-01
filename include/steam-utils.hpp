#pragma once

#include <array>
#include <filesystem>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace SteamUtils
{
    namespace fs = std::filesystem;

    /**
     * @brief Finds the Steam installation directory on the current system
     * @return Path to Steam directory if found, empty path otherwise
     */
    [[nodiscard]] fs::path findSteamDirectory();

    /**
     * @brief Gets all possible Steam directories locations for the current OS
     * @return Vector of paths where Steam might be installed
     */
    [[nodiscard]] std::vector<fs::path> getSteamDirectoryPaths();

    /**
     * @brief Checks if a directory exists and is accessible
     * @param path The directory path to check
     * @return True if the directory exists and is accessible, false otherwise
     */
    [[nodiscard]] bool directoryExists(const fs::path &path);

    /**
     * @brief Validates if a given directory is a valid steam directory
     * @param path The directory path to validate
     * @return True if the directory is a valid Steam directory, false otherwise
     */
    [[nodiscard]] bool isValidSteamDirectory(const fs::path &path);

    /**
     * @brief Gets the user's home directory path
     * @return Path to the user's home directory
     */
    [[nodiscard]] fs::path getHomeDirectory();

    /**
     * @brief Detects the operating system type
     * @return A string representing the OS type (e.g., "Windows", "Linux", "macOS")
     */

    [[nodiscard]] std::string getOperatingSystem();

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
    [[nodiscard]] std::vector<GameInfo> getInstalledGames(const fs::path &steamDir);

    /**
     * @brief Parses an ACF file to extract game information
     * @param acfFilePath Path to the .acf file
     * @return GameInfo structure with game details, empty if parsing fails
     */
    [[nodiscard]] GameInfo parseAcfFile(const fs::path &acfFilePath);

    /**
     * @brief Finds a game by name (case-insensitive search)
     * @param games Vector of games to search through
     * @param gameName Name of the game to find
     * @return GameInfo if found, std::nullopt otherwise
     */
    [[nodiscard]] std::optional<GameInfo> findGameByName(const std::vector<GameInfo> &games, std::string_view gameName);

    struct LogFile
    {
        fs::path path;
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
    [[nodiscard]] std::vector<LogFile> findGameLogs(const fs::path &steamDir, const GameInfo &game);

    /**
     * @brief Common log file extensions
     */
    inline constexpr std::array<std::string_view, 13> logFileExtensions = {
        ".log", ".txt", ".out", ".err", ".crash", ".dmp", ".mdmp", ".rpt",
        ".debug", ".trace", ".console", ".output", ".error"};

    /**
     * @brief Checks if a file is likely a log file based on name and extension
     * @param filename Name of the file to check
     * @return True if the file appears to be a log file
     */
    [[nodiscard]] bool isLogFile(std::string_view filename);

    /**
     * @brief Recursively searches for log files in a directory
     * @param directory Directory to search
     * @param logFiles Vector to store found log files
     * @param maxDepth Maximum recursion depth (default: 3)
     * @param currentDepth Current recursion depth (internal use)
     */
    void searchLogsInDirectory(const fs::path &directory, std::vector<LogFile> &logFiles,
                               int maxDepth = 3, int currentDepth = 0);

    /**
     * @brief Formats file size in human readable format
     * @param size File size in bytes
     * @return Formatted size string (e.g., "1.5 MB")
     */
    [[nodiscard]] std::string formatFileSize(std::uintmax_t size);

    /**
     * @brief Formats file modification time
     * @param filePath Path to the file
     * @return Formatted date/time string
     */
    [[nodiscard]] std::string formatFileTime(const fs::path &filePath);

    /**
     * @brief Creates the output directory for copied logs
     * @param gameName Name of the game
     * @return Path to the created output directory, empty path if creation failed
     */
    [[nodiscard]] fs::path createOutputDirectory(std::string_view gameName);

    /**
     * @brief Copies log files to the output directory
     * @param logFiles Vector of log files to copy
     * @param outputDir Directory to copy files to
     * @param gameName Name of the game
     * @return Number of files successfully copied
     */
    [[nodiscard]] int copyLogsToDirectory(const std::vector<LogFile> &logFiles, const fs::path &outputDir, std::string_view gameName);

    /**
     * @brief Safely copies a single file
     * @param sourcePath source file path
     * @param destPath destination file path
     * @return True if copy was successful, false otherwise
     */
    [[nodiscard]] bool copyFile(const fs::path &sourcePath, const fs::path &destPath);

    /**
     * @brief Creates a directory recursively if it does not exist
     * @param path Directory path to create
     * @return True if the directory was created or already exists, false otherwise
     */
    [[nodiscard]] bool createDirectory(const fs::path &path);

    /**
     * @brief Sanitizes a file name for safe file system usage
     * @param filename The original file name
     * @return Sanitized file name safe for file system usage
     */
    [[nodiscard]] std::string sanitizeFileName(std::string_view filename);
}
