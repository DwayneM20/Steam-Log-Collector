#pragma once

#include <filesystem>
#include <string>
#include <vector>

#include "steam-utils.hpp"

enum class Screen
{
    Welcome,
    GameSelection,
    LogFiles
};

struct AppState
{
    Screen currentScreen = Screen::Welcome;

    std::filesystem::path steamDir;
    char manualSteamDir[512] = "";
    std::vector<SteamUtils::GameInfo> games;
    std::vector<SteamUtils::LogFile> logFiles;
    std::vector<bool> selectedLogs;
    int selectedGameIndex = -1;
    int previewLogIndex = -1;

    bool steamDirFound = false;
    bool scanningGames = false;
    bool scanningLogs = false;

    std::string errorMessage;
    std::string statusMessage;
    std::string previewContent;

    bool showAboutPopup = false;
    bool showPreviewWindow = false;
};

constexpr size_t kMaxPreviewBytes = 1024 * 1024; // 1 MB
constexpr size_t kReadBufferSize = 4096;
