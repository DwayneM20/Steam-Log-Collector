#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <GLFW/glfw3.h>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <algorithm>

#include "logger.hpp"
#include "steam-utils.hpp"
#include "colors.hpp"
#include "theme.hpp"
#include "fonts.hpp"
#include "ui_widgets.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

static void glfw_error_callback(int error, const char *description)
{
    fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

enum class Screen
{
    Welcome,
    GameSelection,
    LogFiles
};

struct AppState
{
    Screen currentScreen = Screen::Welcome;

    std::string steamDir;
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

std::string ReadFileContent(const std::string &filePath,
                            size_t maxBytes = 1024 * 1024)
{
    std::ifstream file(filePath, std::ios::binary);
    if (!file.is_open())
    {
        return "Error: Could not open file for reading.";
    }

    std::ostringstream content;
    char buffer[4096];
    size_t bytesRead = 0;

    while (file.read(buffer, sizeof(buffer)) && bytesRead < maxBytes)
    {
        size_t count = file.gcount();
        content.write(buffer, count);
        bytesRead += count;
    }

    if (file.gcount() > 0 && bytesRead < maxBytes)
    {
        content.write(buffer, file.gcount());
        bytesRead += file.gcount();
    }

    file.close();

    std::string result = content.str();
    if (bytesRead >= maxBytes)
    {
        result += "\n\n[Preview truncated - file is larger than 1MB]";
    }

    return result;
}

void RenderMenuBar(AppState &state, GLFWwindow *window)
{
    if (ImGui::BeginMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("Exit", "Alt+F4"))
                glfwSetWindowShouldClose(window, true);
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Help"))
        {
            if (ImGui::MenuItem("About"))
                state.showAboutPopup = true;
            ImGui::EndMenu();
        }
        ImGui::EndMenuBar();
    }
}

void RenderAboutPopup(AppState &state)
{
    if (state.showAboutPopup)
    {
        ImGui::OpenPopup("About Steam Log Collector");
        state.showAboutPopup = false;
    }

    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

    if (ImGui::BeginPopupModal("About Steam Log Collector", nullptr,
                               ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::PushFont(UIFonts::Large);
        ImGui::TextColored(UIColors::LavenderBlue, "Steam Log Collector");
        ImGui::PopFont();

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        ImGui::Text("A tool for collecting game logs from Steam");
        ImGui::Text("Built with ImGui and C++");
        ImGui::Text("© 2024 Steam Log Collector Contributors");
        ImGui::Text("All rights reserved.");
        ImGui::Text("This software is licensed under the MIT License.");

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        if (UIWidgets::PrimaryButton("Close", ImVec2(120, 0)))
            ImGui::CloseCurrentPopup();

        ImGui::EndPopup();
    }
}

void RenderPreviewWindow(AppState &state)
{
    if (!state.showPreviewWindow)
        return;

    ImVec2 displaySize = ImGui::GetIO().DisplaySize;
    ImGui::SetNextWindowSize(
        ImVec2(displaySize.x * 0.7f, displaySize.y * 0.8f),
        ImGuiCond_FirstUseEver);
    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_FirstUseEver, ImVec2(0.5f, 0.5f));

    if (ImGui::Begin("Log File Preview", &state.showPreviewWindow,
                     ImGuiWindowFlags_NoCollapse))
    {
        if (state.previewLogIndex >= 0 &&
            state.previewLogIndex < static_cast<int>(state.logFiles.size()))
        {
            const auto &log = state.logFiles[state.previewLogIndex];

            ImGui::PushFont(UIFonts::Large);
            ImGui::TextColored(UIColors::LavenderBlue, "%s",
                               log.filename.c_str());
            ImGui::PopFont();

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            UIWidgets::InfoText("Path:", log.path);
            UIWidgets::InfoText("Size:",
                                SteamUtils::formatFileSize(log.size));
            UIWidgets::InfoText("Type:", log.type);
            UIWidgets::InfoText("Modified:", log.lastModified);

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            ImGui::BeginChild("PreviewContent", ImVec2(0, -50), true,
                              ImGuiWindowFlags_HorizontalScrollbar);
            ImGui::PushFont(UIFonts::Medium);
            ImGui::TextUnformatted(state.previewContent.c_str());
            ImGui::PopFont();
            ImGui::EndChild();

            ImGui::Spacing();

            if (UIWidgets::PrimaryButton("Close", ImVec2(120, 0)))
            {
                state.showPreviewWindow = false;
            }
        }
        else
        {
            ImGui::Text("No file selected for preview.");
        }
    }
    ImGui::End();
}

void RenderWelcomeScreen(AppState &state)
{
    ImVec2 windowSize = ImGui::GetContentRegionAvail();
    float padding = windowSize.x * 0.05f;

    // Calculate content width - use more space on larger screens
    float contentWidth = std::min(windowSize.x * 0.6f, 800.0f);
    contentWidth = std::max(contentWidth, 400.0f);
    float startX = (windowSize.x - contentWidth) / 2.0f;

    // Vertical centering - place content in upper-middle area
    float totalContentHeight = 500.0f;
    float startY = std::max((windowSize.y - totalContentHeight) * 0.35f, 20.0f);
    ImGui::SetCursorPosY(startY);

    // Title section
    ImGui::PushFont(UIFonts::Title);
    float titleWidth = ImGui::CalcTextSize("Steam Log Collector").x;
    ImGui::SetCursorPosX((windowSize.x - titleWidth) / 2.0f);
    ImGui::TextColored(UIColors::LavenderBlue, "Steam Log Collector");
    ImGui::PopFont();

    ImGui::Spacing();

    ImGui::PushFont(UIFonts::Default);
    const char *subtitle = "Collect and organize game log files from Steam";
    float subtitleWidth = ImGui::CalcTextSize(subtitle).x;
    ImGui::SetCursorPosX((windowSize.x - subtitleWidth) / 2.0f);
    ImGui::TextColored(UIColors::CoolGray, "%s", subtitle);
    ImGui::PopFont();

    ImGui::Spacing();
    ImGui::Spacing();
    ImGui::Spacing();

    // Main content card
    float cardHeight = std::min(windowSize.y * 0.5f, 600.0f);
    cardHeight = std::max(cardHeight, 300.0f);

    ImGui::SetCursorPosX(startX);
    ImGui::BeginChild("WelcomeCard", ImVec2(contentWidth, cardHeight), true);

    float innerPadding = contentWidth * 0.04f;
    ImGui::SetCursorPos(ImVec2(innerPadding, innerPadding));

    ImGui::BeginGroup();

    UIWidgets::SectionHeader("Get Started");

    ImGui::PushTextWrapPos(ImGui::GetCursorPosX() + contentWidth -
                           innerPadding * 2);
    ImGui::TextWrapped(
        "To begin collecting log files, we need to locate your Steam "
        "installation directory. You can either auto-detect it or "
        "manually specify the path.");
    ImGui::PopTextWrapPos();

    ImGui::Spacing();
    ImGui::Spacing();

    // Auto-detect button
    float buttonWidth = contentWidth - innerPadding * 2;
    float buttonHeight = std::max(windowSize.y * 0.06f, 45.0f);

    if (UIWidgets::PrimaryButton("Auto-Detect Steam Directory",
                                 ImVec2(buttonWidth, buttonHeight)))
    {
        state.errorMessage.clear();
        state.statusMessage = "Searching for Steam...";
        state.steamDir = SteamUtils::findSteamDirectory();

        if (!state.steamDir.empty())
        {
            state.steamDirFound = true;
            Logger::log("Found Steam directory: " + state.steamDir, SEVERITY_LEVEL::INFO);
            state.games = SteamUtils::getInstalledGames(state.steamDir);
            state.currentScreen = Screen::GameSelection;
        }
        else
        {
            state.errorMessage =
                "Could not auto-detect Steam directory. "
                "Please enter the path manually.";
        }
    }

    ImGui::Spacing();
    ImGui::Spacing();

    // Divider with "or"
    ImVec2 cursorPos = ImGui::GetCursorPos();
    float dividerY = cursorPos.y + 8;
    float dividerWidth = (buttonWidth - 40) / 2.0f;

    ImDrawList *drawList = ImGui::GetWindowDrawList();
    ImVec2 windowPos = ImGui::GetWindowPos();

    drawList->AddLine(
        ImVec2(windowPos.x + innerPadding, windowPos.y + dividerY),
        ImVec2(windowPos.x + innerPadding + dividerWidth, windowPos.y + dividerY),
        ImGui::ColorConvertFloat4ToU32(UIColors::CoolGray), 1.0f);

    ImGui::SetCursorPosX(innerPadding + dividerWidth + 5);
    ImGui::TextColored(UIColors::CoolGray, " or ");

    drawList->AddLine(
        ImVec2(windowPos.x + innerPadding + dividerWidth + 40,
               windowPos.y + dividerY),
        ImVec2(windowPos.x + innerPadding + buttonWidth,
               windowPos.y + dividerY),
        ImGui::ColorConvertFloat4ToU32(UIColors::CoolGray), 1.0f);

    ImGui::Spacing();
    ImGui::Spacing();

    // Manual input
    ImGui::Text("Enter Steam directory path:");
    ImGui::SetNextItemWidth(buttonWidth);
    ImGui::InputText("##steamdir", state.manualSteamDir,
                     sizeof(state.manualSteamDir));

    ImGui::Spacing();

    if (UIWidgets::SecondaryButton("Use Manual Path",
                                   ImVec2(buttonWidth, buttonHeight * 0.85f)))
    {
        state.errorMessage.clear();
        std::string manualPath = state.manualSteamDir;

        if (manualPath.empty())
        {
            state.errorMessage = "Please enter a directory path.";
        }
        else if (!SteamUtils::directoryExists(manualPath))
        {
            state.errorMessage = "Directory does not exist: " + manualPath;
        }
        else if (!SteamUtils::isValidSteamDirectory(manualPath))
        {
            state.errorMessage =
                "This does not appear to be a valid Steam directory.";
        }
        else
        {
            state.steamDir = manualPath;
            state.steamDirFound = true;
            Logger::log("Using manual Steam directory: " + state.steamDir, SEVERITY_LEVEL::INFO);
            state.games = SteamUtils::getInstalledGames(state.steamDir);
            state.currentScreen = Screen::GameSelection;
        }
    }

    ImGui::EndGroup();
    ImGui::EndChild();

    // Error message
    if (!state.errorMessage.empty())
    {
        ImGui::Spacing();
        ImGui::SetCursorPosX(startX);
        ImGui::PushStyleColor(ImGuiCol_ChildBg,
                              ImVec4(UIColors::Error.x, UIColors::Error.y,
                                     UIColors::Error.z, 0.2f));
        ImGui::BeginChild("ErrorBox", ImVec2(contentWidth, 60), true);
        ImGui::SetCursorPos(ImVec2(15, 15));
        ImGui::TextColored(UIColors::Error, "%s", state.errorMessage.c_str());
        ImGui::EndChild();
        ImGui::PopStyleColor();
    }
}

void RenderGameSelectionScreen(AppState &state)
{
    ImVec2 windowSize = ImGui::GetContentRegionAvail();
    float padding = std::max(windowSize.x * 0.03f, 20.0f);
    float contentWidth = windowSize.x - padding * 2;

    ImGui::SetCursorPosX(padding);

    // Header row
    ImGui::BeginGroup();

    ImGui::PushFont(UIFonts::Title);
    ImGui::TextColored(UIColors::LavenderBlue, "Select a Game");
    ImGui::PopFont();

    ImGui::SameLine(contentWidth - 80);
    if (UIWidgets::SecondaryButton("Back", ImVec2(100, 35)))
    {
        state.currentScreen = Screen::Welcome;
        state.steamDir.clear();
        state.steamDirFound = false;
        state.games.clear();
        state.errorMessage.clear();
    }

    ImGui::EndGroup();

    ImGui::Spacing();
    ImGui::Spacing();

    // Steam directory info bar
    ImGui::SetCursorPosX(padding);
    float infoBarHeight = std::max(windowSize.y * 0.08f, 70.0f);
    ImGui::BeginChild("SteamInfo", ImVec2(contentWidth, infoBarHeight), true);

    float infoPadding = 15.0f;
    ImGui::SetCursorPos(ImVec2(infoPadding, infoPadding));

    ImGui::BeginGroup();
    ImGui::PushFont(UIFonts::Default);
    ImGui::TextColored(UIColors::CoolGray, "Steam Directory:");
    ImGui::SameLine();
    ImGui::TextColored(UIColors::OffWhite, "%s", state.steamDir.c_str());

    ImGui::TextColored(UIColors::CoolGray, "Games Found:");
    ImGui::SameLine();
    ImGui::TextColored(UIColors::OffWhite, "%zu", state.games.size());
    ImGui::PopFont();
    ImGui::EndGroup();

    ImGui::EndChild();

    ImGui::Spacing();
    ImGui::Spacing();

    // Games list or empty state
    ImGui::SetCursorPosX(padding);
    float listHeight = windowSize.y - ImGui::GetCursorPosY() - padding;

    if (state.games.empty())
    {
        ImGui::BeginChild("NoGames", ImVec2(contentWidth, listHeight), true);

        float centerY = listHeight / 2.0f - 60.0f;
        ImGui::SetCursorPosY(centerY);

        ImGui::PushFont(UIFonts::Large);
        const char *noGamesText = "No Games Found";
        float textWidth = ImGui::CalcTextSize(noGamesText).x;
        ImGui::SetCursorPosX((contentWidth - textWidth) / 2.0f);
        ImGui::TextColored(UIColors::CoolGray, "%s", noGamesText);
        ImGui::PopFont();

        ImGui::Spacing();
        ImGui::Spacing();

        const char *helpText =
            "No Steam games were detected in this directory.";
        float helpWidth = ImGui::CalcTextSize(helpText).x;
        ImGui::SetCursorPosX((contentWidth - helpWidth) / 2.0f);
        ImGui::TextColored(UIColors::CoolGray, "%s", helpText);

        ImGui::Spacing();
        ImGui::Spacing();
        ImGui::Spacing();

        float buttonWidth = 220.0f;
        ImGui::SetCursorPosX((contentWidth - buttonWidth) / 2.0f);
        if (UIWidgets::PrimaryButton("Try Different Directory",
                                     ImVec2(buttonWidth, 50)))
        {
            state.currentScreen = Screen::Welcome;
            state.steamDir.clear();
            state.steamDirFound = false;
        }

        ImGui::EndChild();
    }
    else
    {
        ImGui::BeginChild("GamesList", ImVec2(contentWidth, listHeight), true);

        float innerPadding = 15.0f;
        ImGui::SetCursorPos(ImVec2(innerPadding, innerPadding));

        ImGui::PushFont(UIFonts::Default);
        ImGui::TextColored(UIColors::CoolGray,
                           "Click on a game to view its log files:");
        ImGui::PopFont();

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        float itemWidth = contentWidth - innerPadding * 2 - 15;
        float itemHeight = std::max(windowSize.y * 0.08f, 70.0f);

        for (size_t i = 0; i < state.games.size(); i++)
        {
            const auto &game = state.games[i];

            ImGui::PushID(static_cast<int>(i));

            ImVec2 cursorPos = ImGui::GetCursorScreenPos();
            ImVec2 itemMax = ImVec2(cursorPos.x + itemWidth,
                                    cursorPos.y + itemHeight);

            bool hovered = ImGui::IsMouseHoveringRect(cursorPos, itemMax);

            if (hovered)
            {
                ImGui::PushStyleColor(
                    ImGuiCol_ChildBg,
                    ImVec4(UIColors::LavenderBlue.x, UIColors::LavenderBlue.y,
                           UIColors::LavenderBlue.z, 0.15f));
            }

            ImGui::BeginChild("GameItem", ImVec2(itemWidth, itemHeight), true,
                              ImGuiWindowFlags_NoScrollbar);

            ImGui::SetCursorPos(ImVec2(15, 12));

            ImGui::PushFont(UIFonts::Large);
            ImGui::TextColored(
                hovered ? UIColors::LavenderBlue : UIColors::OffWhite, "%s",
                game.name.c_str());
            ImGui::PopFont();

            ImGui::SetCursorPosX(15);
            ImGui::PushFont(UIFonts::Small);
            ImGui::TextColored(UIColors::CoolGray, "App ID: %s  |  %s",
                               game.appId.c_str(), game.installDir.c_str());
            ImGui::PopFont();

            ImGui::EndChild();

            if (hovered)
            {
                ImGui::PopStyleColor();

                if (ImGui::IsMouseClicked(0))
                {
                    state.selectedGameIndex = static_cast<int>(i);
                    state.logFiles.clear();
                    state.selectedLogs.clear();
                    state.previewLogIndex = -1;
                    state.statusMessage.clear();
                    state.errorMessage.clear();

                    state.logFiles =
                        SteamUtils::findGameLogs(state.steamDir, game);
                    state.selectedLogs.resize(state.logFiles.size(), false);

                    state.currentScreen = Screen::LogFiles;
                }
            }

            ImGui::PopID();
            ImGui::Spacing();
        }

        ImGui::EndChild();
    }
}

void RenderLogFilesScreen(AppState &state)
{
    if (state.selectedGameIndex < 0 ||
        state.selectedGameIndex >= static_cast<int>(state.games.size()))
    {
        state.currentScreen = Screen::GameSelection;
        return;
    }

    const auto &game = state.games[state.selectedGameIndex];

    ImVec2 windowSize = ImGui::GetContentRegionAvail();
    float padding = std::max(windowSize.x * 0.025f, 20.0f);
    float contentWidth = windowSize.x - padding * 2;

    ImGui::SetCursorPosX(padding);

    // Back button
    if (UIWidgets::SecondaryButton("< Back to Games", ImVec2(170, 35)))
    {
        state.currentScreen = Screen::GameSelection;
        state.logFiles.clear();
        state.selectedLogs.clear();
        state.selectedGameIndex = -1;
        state.previewLogIndex = -1;
        state.statusMessage.clear();
        state.errorMessage.clear();
    }

    ImGui::Spacing();
    ImGui::Spacing();

    // Game info card
    ImGui::SetCursorPosX(padding);
    float infoCardHeight = std::max(windowSize.y * 0.12f, 140.0f);
    ImGui::BeginChild("GameInfoCard", ImVec2(contentWidth, infoCardHeight),
                      true);

    float cardPadding = 20.0f;
    ImGui::SetCursorPos(ImVec2(cardPadding, cardPadding));

    ImGui::PushFont(UIFonts::Title);
    ImGui::TextColored(UIColors::LavenderBlue, "%s", game.name.c_str());
    ImGui::PopFont();

    ImGui::Spacing();

    // Info columns
    float colWidth = (contentWidth - cardPadding * 2) / 3.0f;
    ImGui::Columns(3, nullptr, false);
    ImGui::SetColumnWidth(0, colWidth);
    ImGui::SetColumnWidth(1, colWidth);
    ImGui::SetColumnWidth(2, colWidth);

    ImGui::PushFont(UIFonts::Medium);
    ImGui::TextColored(UIColors::CoolGray, "APP ID");
    ImGui::PopFont();
    ImGui::PushFont(UIFonts::Default);
    ImGui::Text("%s", game.appId.c_str());
    ImGui::PopFont();

    ImGui::NextColumn();

    ImGui::PushFont(UIFonts::Medium);
    ImGui::TextColored(UIColors::CoolGray, "INSTALL DIRECTORY");
    ImGui::PopFont();
    ImGui::PushFont(UIFonts::Default);
    ImGui::TextWrapped("%s", game.installDir.c_str());
    ImGui::PopFont();

    ImGui::NextColumn();

    ImGui::PushFont(UIFonts::Medium);
    ImGui::TextColored(UIColors::CoolGray, "LOG FILES FOUND");
    ImGui::PopFont();
    ImGui::PushFont(UIFonts::Default);
    ImGui::Text("%zu", state.logFiles.size());
    ImGui::PopFont();

    ImGui::Columns(1);

    ImGui::EndChild();

    ImGui::Spacing();
    ImGui::Spacing();

    // Log files section
    ImGui::SetCursorPosX(padding);

    if (state.logFiles.empty())
    {
        float emptyHeight = windowSize.y - ImGui::GetCursorPosY() - padding;
        ImGui::BeginChild("NoLogs", ImVec2(contentWidth, emptyHeight), true);

        float centerY = emptyHeight / 2.0f - 50.0f;
        ImGui::SetCursorPosY(centerY);

        ImGui::PushFont(UIFonts::Large);
        const char *noLogsText = "No Log Files Found";
        float textWidth = ImGui::CalcTextSize(noLogsText).x;
        ImGui::SetCursorPosX((contentWidth - textWidth) / 2.0f);
        ImGui::TextColored(UIColors::CoolGray, "%s", noLogsText);
        ImGui::PopFont();

        ImGui::Spacing();
        ImGui::Spacing();

        const char *helpText = "No log files were detected for this game.";
        float helpWidth = ImGui::CalcTextSize(helpText).x;
        ImGui::SetCursorPosX((contentWidth - helpWidth) / 2.0f);
        ImGui::TextColored(UIColors::CoolGray, "%s", helpText);

        ImGui::EndChild();
    }
    else
    {
        // Action buttons row
        ImGui::BeginGroup();

        float buttonHeight = 40.0f;

        if (UIWidgets::SecondaryButton("Select All", ImVec2(130, buttonHeight)))
        {
            for (auto &selected : state.selectedLogs)
                selected = true;
        }

        ImGui::SameLine();

        if (UIWidgets::SecondaryButton("Deselect All",
                                       ImVec2(150, buttonHeight)))
        {
            for (auto &selected : state.selectedLogs)
                selected = false;
        }

        ImGui::SameLine();

        bool canPreview =
            state.previewLogIndex >= 0 &&
            state.previewLogIndex < static_cast<int>(state.logFiles.size());

        if (!canPreview)
            ImGui::BeginDisabled();

        if (UIWidgets::SecondaryButton("Preview Selected",
                                       ImVec2(200, buttonHeight)))
        {
            state.previewContent =
                ReadFileContent(state.logFiles[state.previewLogIndex].path);
            state.showPreviewWindow = true;
        }

        if (!canPreview)
            ImGui::EndDisabled();

        // Copy button on the right
        int selectedCount = 0;
        for (bool selected : state.selectedLogs)
        {
            if (selected)
                selectedCount++;
        }

        float copyButtonWidth = 220.0f;
        ImGui::SameLine(contentWidth - copyButtonWidth);

        if (selectedCount == 0)
            ImGui::BeginDisabled();

        std::string copyText =
            "Copy Selected (" + std::to_string(selectedCount) + ")";
        if (UIWidgets::PrimaryButton(copyText.c_str(),
                                     ImVec2(copyButtonWidth, buttonHeight)))
        {
            state.errorMessage.clear();
            std::string outputDir =
                SteamUtils::createOutputDirectory(game.name);

            if (!outputDir.empty())
            {
                std::vector<SteamUtils::LogFile> selectedLogFiles;
                for (size_t i = 0; i < state.logFiles.size(); ++i)
                {
                    if (state.selectedLogs[i])
                    {
                        selectedLogFiles.push_back(state.logFiles[i]);
                    }
                }

                int copied = SteamUtils::copyLogsToDirectory(
                    selectedLogFiles, outputDir, game.name);
                state.statusMessage = "Successfully copied " +
                                      std::to_string(copied) + " file(s) to: " +
                                      outputDir;
            }
            else
            {
                state.errorMessage = "Failed to create output directory";
            }
        }

        if (selectedCount == 0)
            ImGui::EndDisabled();

        ImGui::EndGroup();

        ImGui::Spacing();

        // Status/error messages
        if (!state.statusMessage.empty())
        {
            ImGui::PushStyleColor(
                ImGuiCol_ChildBg,
                ImVec4(UIColors::Success.x, UIColors::Success.y,
                       UIColors::Success.z, 0.15f));
            ImGui::BeginChild("StatusBox", ImVec2(contentWidth, 45), true);
            ImGui::SetCursorPos(ImVec2(15, 12));
            ImGui::TextColored(UIColors::Success, "%s",
                               state.statusMessage.c_str());
            ImGui::EndChild();
            ImGui::PopStyleColor();
            ImGui::Spacing();
        }

        if (!state.errorMessage.empty())
        {
            ImGui::PushStyleColor(ImGuiCol_ChildBg,
                                  ImVec4(UIColors::Error.x, UIColors::Error.y,
                                         UIColors::Error.z, 0.15f));
            ImGui::BeginChild("ErrorBox", ImVec2(contentWidth, 45), true);
            ImGui::SetCursorPos(ImVec2(15, 12));
            ImGui::TextColored(UIColors::Error, "%s",
                               state.errorMessage.c_str());
            ImGui::EndChild();
            ImGui::PopStyleColor();
            ImGui::Spacing();
        }

        // Log files table
        float tableHeight = windowSize.y - ImGui::GetCursorPosY() - padding;
        ImGui::BeginChild("LogFilesTable", ImVec2(contentWidth, tableHeight),
                          true);

        if (ImGui::BeginTable("LogsTable", 5,
                              ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg |
                                  ImGuiTableFlags_ScrollY |
                                  ImGuiTableFlags_Resizable |
                                  ImGuiTableFlags_SizingStretchProp))
        {
            ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed, 40);
            ImGui::TableSetupColumn("File Name",
                                    ImGuiTableColumnFlags_WidthStretch, 3.0f);
            ImGui::TableSetupColumn("Type", ImGuiTableColumnFlags_WidthStretch,
                                    1.0f);
            ImGui::TableSetupColumn("Size", ImGuiTableColumnFlags_WidthStretch,
                                    0.8f);
            ImGui::TableSetupColumn("Modified",
                                    ImGuiTableColumnFlags_WidthStretch, 1.2f);
            ImGui::TableHeadersRow();

            for (size_t i = 0; i < state.logFiles.size(); ++i)
            {
                const auto &log = state.logFiles[i];
                ImGui::TableNextRow();

                ImGui::TableSetColumnIndex(0);
                ImGui::PushID(static_cast<int>(i));
                bool selected = state.selectedLogs[i];
                if (ImGui::Checkbox("##select", &selected))
                {
                    state.selectedLogs[i] = selected;
                    if (selected)
                    {
                        state.previewLogIndex = static_cast<int>(i);
                    }
                }
                ImGui::PopID();

                ImGui::TableSetColumnIndex(1);
                bool isSelected =
                    state.previewLogIndex == static_cast<int>(i);

                if (ImGui::Selectable(log.filename.c_str(), isSelected,
                                      ImGuiSelectableFlags_SpanAllColumns |
                                          ImGuiSelectableFlags_AllowOverlap))
                {
                    state.previewLogIndex = static_cast<int>(i);
                }

                ImGui::TableSetColumnIndex(2);
                ImGui::TextColored(UIColors::LightTeal, "%s",
                                   log.type.c_str());

                ImGui::TableSetColumnIndex(3);
                ImGui::Text("%s",
                            SteamUtils::formatFileSize(log.size).c_str());

                ImGui::TableSetColumnIndex(4);
                ImGui::PushFont(UIFonts::Small);
                ImGui::Text("%s", log.lastModified.c_str());
                ImGui::PopFont();
            }

            ImGui::EndTable();
        }

        ImGui::EndChild();
    }
}

bool SetWindowIcon(GLFWwindow *window, const char *iconPath)
{
    GLFWimage icon;
    icon.pixels = stbi_load(iconPath, &icon.width, &icon.height, 0, 4);

    if (!icon.pixels)
    {
        fprintf(stderr, "Failed to load icon: %s\n", iconPath);
        return false;
    }

    glfwSetWindowIcon(window, 1, &icon);
    stbi_image_free(icon.pixels);
    return true;
}

int main(int argc, char *argv[])
{
    (void)argc;
    (void)argv;

    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit())
        return 1;

    const char *glsl_version = "#version 330";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);

    // Get primary monitor for fullscreen dimensions
    GLFWmonitor *primaryMonitor = glfwGetPrimaryMonitor();
    const GLFWvidmode *videoMode = glfwGetVideoMode(primaryMonitor);

    GLFWwindow *window = glfwCreateWindow(
        videoMode->width, videoMode->height, "Steam Log Collector",
        nullptr, nullptr);

    if (window == nullptr)
        return 1;

    SetWindowIcon(window, "resources/SLC-logo.png");

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    UIFonts::LoadFonts(io);
    UITheme::ApplyModernStyle();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    AppState state;

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(io.DisplaySize);
        ImGui::Begin("Steam Log Collector", nullptr,
                     ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                         ImGuiWindowFlags_NoCollapse |
                         ImGuiWindowFlags_MenuBar |
                         ImGuiWindowFlags_NoTitleBar);

        RenderMenuBar(state, window);
        RenderAboutPopup(state);

        ImGui::Spacing();

        switch (state.currentScreen)
        {
        case Screen::Welcome:
            RenderWelcomeScreen(state);
            break;
        case Screen::GameSelection:
            RenderGameSelectionScreen(state);
            break;
        case Screen::LogFiles:
            RenderLogFilesScreen(state);
            break;
        }

        ImGui::End();

        RenderPreviewWindow(state);

        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(UIColors::DarkBg.x, UIColors::DarkBg.y, UIColors::DarkBg.z,
                     1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}