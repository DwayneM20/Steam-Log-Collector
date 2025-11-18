#ifndef STEAM_CORE_HPP
#define STEAM_CORE_HPP

#include "steam-utils.hpp"
#include <functional>
#include <thread>
#include <future>

class SteamLogCollector
{
public:
    using ProgressCallback = std::function<void(double progress, const std::string &message)>;
    using CompletionCallback = std::function<void(bool success, const std::string &message)>;

    SteamLogCollector();
    ~SteamLogCollector();

    void findSteamDirectoryAsync(CompletionCallback callback);
    void scanForGamesAsync(ProgressCallback progress_cb,
                           std::function<void(std::vector<SteamUtils::GameInfo>)> completion_cb);
    void findLogsForGameAsync(const SteamUtils::GameInfo &game,
                              ProgressCallback progress_cb,
                              std::function<void(std::vector<SteamUtils::LogFile>)> completion_cb);
    void copyLogsAsync(const std::vector<SteamUtils::LogFile> &log_files,
                       const std::string &game_name,
                       ProgressCallback progress_cb,
                       std::function<void(int, std::string)> completion_cb);

    std::string findSteamDirectory();
    std::vector<SteamUtils::GameInfo> scanForGames();
    std::vector<SteamUtils::LogFile> findLogsForGame(const SteamUtils::GameInfo &game);

    const std::string &getSteamDirectory() const { return m_steam_directory; }

private:
    std::string m_steam_directory;
    std::vector<std::thread> m_worker_threads;

    void cleanup_threads();
};

#endif