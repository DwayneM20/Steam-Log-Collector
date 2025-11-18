#include "steam-core.hpp"
#include "logger.hpp"
#include <algorithm>

SteamLogCollector::SteamLogCollector() = default;

SteamLogCollector::~SteamLogCollector()
{
    cleanup_threads();
}

void SteamLogCollector::cleanup_threads()
{
    for (auto &thread : m_worker_threads)
    {
        if (thread.joinable())
        {
            thread.join();
        }
    }
    m_worker_threads.clear();
}

std::string SteamLogCollector::findSteamDirectory()
{
    if (m_steam_directory.empty())
    {
        m_steam_directory = SteamUtils::findSteamDirectory();
    }
    return m_steam_directory;
}

std::vector<SteamUtils::GameInfo> SteamLogCollector::scanForGames()
{
    if (m_steam_directory.empty())
    {
        findSteamDirectory();
    }

    if (m_steam_directory.empty())
    {
        return {};
    }

    return SteamUtils::getInstalledGames(m_steam_directory);
}

std::vector<SteamUtils::LogFile> SteamLogCollector::findLogsForGame(const SteamUtils::GameInfo &game)
{
    if (m_steam_directory.empty())
    {
        return {};
    }

    return SteamUtils::findGameLogs(m_steam_directory, game);
}

void SteamLogCollector::findSteamDirectoryAsync(CompletionCallback callback)
{
    m_worker_threads.emplace_back([this, callback]()
                                  {
        try {
            std::string steam_dir = findSteamDirectory();
            callback(!steam_dir.empty(), steam_dir.empty() ? "Steam directory not found" : steam_dir);
        } catch (const std::exception& e) {
            callback(false, std::string("Error: ") + e.what());
        } });
}

void SteamLogCollector::scanForGamesAsync(ProgressCallback progress_cb,
                                          std::function<void(std::vector<SteamUtils::GameInfo>)> completion_cb)
{
    m_worker_threads.emplace_back([this, progress_cb, completion_cb]()
                                  {
        try {
            progress_cb(0.1, "Finding Steam directory...");
            
            if (m_steam_directory.empty()) {
                m_steam_directory = SteamUtils::findSteamDirectory();
            }
            
            if (m_steam_directory.empty()) {
                completion_cb({});
                return;
            }
            
            progress_cb(0.5, "Scanning for games...");
            auto games = SteamUtils::getInstalledGames(m_steam_directory);
            
            progress_cb(1.0, "Scan complete");
            completion_cb(games);
        } catch (const std::exception& e) {
            Logger::log("Error in scanForGamesAsync: " + std::string(e.what()));
            completion_cb({});
        } });
}

void SteamLogCollector::findLogsForGameAsync(const SteamUtils::GameInfo &game,
                                             ProgressCallback progress_cb,
                                             std::function<void(std::vector<SteamUtils::LogFile>)> completion_cb)
{
    m_worker_threads.emplace_back([this, game, progress_cb, completion_cb]()
                                  {
        try {
            progress_cb(0.1, "Searching for log files...");
            auto log_files = SteamUtils::findGameLogs(m_steam_directory, game);
            
            progress_cb(1.0, "Search complete");
            completion_cb(log_files);
        } catch (const std::exception& e) {
            Logger::log("Error in findLogsForGameAsync: " + std::string(e.what()));
            completion_cb({});
        } });
}

void SteamLogCollector::copyLogsAsync(const std::vector<SteamUtils::LogFile> &log_files,
                                      const std::string &game_name,
                                      ProgressCallback progress_cb,
                                      std::function<void(int, std::string)> completion_cb)
{
    m_worker_threads.emplace_back([this, log_files, game_name, progress_cb, completion_cb]()
                                  {
        try {
            progress_cb(0.1, "Creating output directory...");
            
            std::string output_dir = SteamUtils::createOutputDirectory(game_name);
            if (output_dir.empty()) {
                completion_cb(0, "Failed to create output directory");
                return;
            }
            
            progress_cb(0.2, "Copying files...");
            
            for (size_t i = 0; i < log_files.size(); ++i) {
                double progress = 0.2 + (0.7 * i / log_files.size());
                progress_cb(progress, "Copying " + log_files[i].filename + "...");
            }
            
            int total_copied = SteamUtils::copyLogsToDirectory(log_files, output_dir, game_name);
            
            progress_cb(1.0, "Copy complete");
            completion_cb(total_copied, output_dir);
        } catch (const std::exception& e) {
            Logger::log("Error in copyLogsAsync: " + std::string(e.what()));
            completion_cb(0, std::string("Error: ") + e.what());
        } });
}