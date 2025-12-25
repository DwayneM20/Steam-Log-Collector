#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <GLFW/glfw3.h>
#include <string>
#include <vector>

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

struct AppState
{
    std::string steamDir;
    std::vector<SteamUtils::GameInfo> games;
    std::vector<SteamUtils::LogFile> logFiles;
    int selectedGameIndex = -1;
    bool steamDirFound = false;
    bool scanningGames = false;
    std::string statusMessage = "Click 'Find Steam Directory' to begin";
    bool showAboutPopup = false;
};

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
    ImGui::SetNextWindowPos(
        center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

    if (ImGui::BeginPopupModal(
            "About Steam Log Collector",
            nullptr,
            ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::PushFont(UIFonts::Large);
        ImGui::TextColored(
            UIColors::LavenderBlue, "Steam Log Collector");
        ImGui::PopFont();

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        ImGui::Text("A tool for collecting game logs from Steam");
        ImGui::Text("Built with ImGui and C++");
        ImGui::Text("© 2024 Steam Log Collector Contributors");
        ImGui::Text("All rights reserved.");
        ImGui::Text("This software is licensed under the MIT License.");
        ImGui::Text("See the LICENSE file for details.");

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        if (UIWidgets::PrimaryButton("Close", ImVec2(120, 0)))
            ImGui::CloseCurrentPopup();

        ImGui::EndPopup();
    }
}

void RenderStatusSection(AppState &state)
{
    ImGui::PushFont(UIFonts::Default);

    if (state.steamDirFound)
        UIWidgets::StatusBadge("Ready", UIColors::Success);
    else if (state.scanningGames)
        UIWidgets::StatusBadge("Scanning...", UIColors::Warning);
    else
        UIWidgets::StatusBadge("Not Ready", UIColors::CoolGray);

    ImGui::SameLine();
    ImGui::TextColored(UIColors::OffWhite, "%s", state.statusMessage.c_str());

    ImGui::PopFont();
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();
}

void RenderSteamDirectorySection(AppState &state)
{
    UIWidgets::SectionHeader("Steam Installation");

    if (UIWidgets::PrimaryButton(
            "Find Steam Directory", ImVec2(200, 40)))
    {
        state.statusMessage = "Searching for Steam...";
        state.steamDir = SteamUtils::findSteamDirectory();

        if (!state.steamDir.empty())
        {
            state.steamDirFound = true;
            state.statusMessage = "Steam directory found!";
            Logger::log("Found Steam directory: " + state.steamDir);
        }
        else
        {
            state.steamDirFound = false;
            state.statusMessage = "Steam directory not found";
        }
    }

    if (!state.steamDir.empty())
    {
        ImGui::Spacing();
        ImGui::BeginChild(
            "SteamDirDisplay",
            ImVec2(0, 60),
            true,
            ImGuiWindowFlags_NoScrollbar);

        ImGui::PushFont(UIFonts::Small);
        ImGui::TextColored(UIColors::CoolGray, "Installation Path:");
        ImGui::TextWrapped("%s", state.steamDir.c_str());
        ImGui::PopFont();

        ImGui::EndChild();
    }

    ImGui::Spacing();
}

void RenderGamesSection(AppState &state)
{
    if (!state.steamDirFound)
        return;

    UIWidgets::SectionHeader("Installed Games");

    if (UIWidgets::PrimaryButton("Scan for Games", ImVec2(200, 40)))
    {
        state.scanningGames = true;
        state.statusMessage = "Scanning for games...";
        state.games = SteamUtils::getInstalledGames(state.steamDir);
        state.statusMessage =
            "Found " + std::to_string(state.games.size()) + " games";
        state.selectedGameIndex = -1;
        state.logFiles.clear();
        state.scanningGames = false;
    }

    if (!state.games.empty())
    {
        ImGui::Spacing();
        ImGui::PushFont(UIFonts::Default);
        ImGui::Text("Found %zu games", state.games.size());
        ImGui::PopFont();

        ImGui::BeginChild(
            "GamesList",
            ImVec2(0, 220),
            true);

        for (int i = 0; i < state.games.size(); i++)
        {
            bool isSelected = (state.selectedGameIndex == i);

            if (isSelected)
                ImGui::PushStyleColor(
                    ImGuiCol_Header, UIColors::LavenderBlue);

            if (ImGui::Selectable(
                    state.games[i].name.c_str(), isSelected))
            {
                state.selectedGameIndex = i;
                state.logFiles.clear();
            }

            if (isSelected)
                ImGui::PopStyleColor();
        }

        ImGui::EndChild();
    }

    ImGui::Spacing();
}

void RenderSelectedGameSection(AppState &state)
{
    if (state.selectedGameIndex < 0 ||
        state.selectedGameIndex >= state.games.size())
        return;

    UIWidgets::SectionHeader("Selected Game");

    const auto &game = state.games[state.selectedGameIndex];

    ImGui::BeginChild("GameInfo", ImVec2(0, 120), true);
    ImGui::PushFont(UIFonts::Large);
    ImGui::TextColored(UIColors::LavenderBlue, "%s", game.name.c_str());
    ImGui::PopFont();

    ImGui::Spacing();
    UIWidgets::InfoText("App ID:", game.appId);
    UIWidgets::InfoText("Install Directory:", game.installDir);
    ImGui::EndChild();

    ImGui::Spacing();

    if (UIWidgets::PrimaryButton("Find Log Files", ImVec2(200, 40)))
    {
        state.statusMessage = "Searching for log files...";
        state.logFiles = SteamUtils::findGameLogs(state.steamDir, game);
        state.statusMessage = "Found " +
                              std::to_string(state.logFiles.size()) +
                              " log files";
    }

    ImGui::Spacing();
}

void RenderLogFilesSection(AppState &state)
{
    if (state.logFiles.empty())
        return;

    UIWidgets::SectionHeader("Log Files");

    ImGui::PushFont(UIFonts::Default);
    ImGui::Text("Found %zu log files", state.logFiles.size());
    ImGui::PopFont();

    ImGui::BeginChild("LogFilesList", ImVec2(0, 280), true);

    if (ImGui::BeginTable(
            "LogFilesTable", 4,
            ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg |
                ImGuiTableFlags_ScrollY))
    {
        ImGui::TableSetupColumn(
            "File Name", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn("Type", ImGuiTableColumnFlags_WidthFixed, 100);
        ImGui::TableSetupColumn("Size", ImGuiTableColumnFlags_WidthFixed, 80);
        ImGui::TableSetupColumn(
            "Modified", ImGuiTableColumnFlags_WidthFixed, 150);
        ImGui::TableHeadersRow();

        for (const auto &log : state.logFiles)
        {
            ImGui::TableNextRow();

            ImGui::TableSetColumnIndex(0);
            ImGui::Text("%s", log.filename.c_str());

            ImGui::TableSetColumnIndex(1);
            ImGui::TextColored(UIColors::LightTeal, "%s", log.type.c_str());

            ImGui::TableSetColumnIndex(2);
            ImGui::Text(
                "%s", SteamUtils::formatFileSize(log.size).c_str());

            ImGui::TableSetColumnIndex(3);
            ImGui::PushFont(UIFonts::Small);
            ImGui::Text("%s", log.lastModified.c_str());
            ImGui::PopFont();
        }

        ImGui::EndTable();
    }

    ImGui::EndChild();

    ImGui::Spacing();

    if (UIWidgets::PrimaryButton(
            "Copy Log Files to ~/steam-logs", ImVec2(250, 40)))
    {
        const auto &game = state.games[state.selectedGameIndex];
        std::string outputDir =
            SteamUtils::createOutputDirectory(game.name);

        if (!outputDir.empty())
        {
            int copied = SteamUtils::copyLogsToDirectory(
                state.logFiles, outputDir, game.name);
            state.statusMessage = "Copied " + std::to_string(copied) +
                                  " files to " + outputDir;
        }
        else
        {
            state.statusMessage = "Failed to create output directory";
        }
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
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit())
        return 1;

    const char *glsl_version = "#version 330";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);

    GLFWwindow *window = glfwCreateWindow(
        1400, 900, "Steam Log Collector", nullptr, nullptr);
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
        ImGui::Begin(
            "Steam Log Collector",
            nullptr,
            ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_MenuBar);

        RenderMenuBar(state, window);
        RenderAboutPopup(state);

        ImGui::Spacing();
        RenderStatusSection(state);
        RenderSteamDirectorySection(state);
        RenderGamesSection(state);
        RenderSelectedGameSection(state);
        RenderLogFilesSection(state);

        ImGui::End();

        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(
            UIColors::DarkBg.x, UIColors::DarkBg.y, UIColors::DarkBg.z, 1.0f);
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