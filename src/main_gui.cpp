#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <GLFW/glfw3.h>
#include <string>
#include <vector>
#include "logger.hpp"
#include "steam-utils.hpp"

static void glfw_error_callback(int error, const char *description)
{
    fprintf(stderr, "GLFW Error %d: %s\n", error, description);
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
        1280, 720, "Steam Log Collector", nullptr, nullptr);
    if (window == nullptr)
        return 1;

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    /*
        float font_size = 32.0f;
        io.Fonts->AddFontDefault();
        ImFont *font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\Roboto-Medium.ttf", font_size);

        if (font == nullptr)
        {
            ImFontConfig config;
            config.SizePixels = font_size;
            io.Fonts->AddFontDefault(&config);
        }
    */
    ImGui::StyleColorsDark();
    /*
        ImGuiStyle &style = ImGui::GetStyle();
        style.ScaleAllSizes(1.2f);
    */
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    std::string steamDir;
    std::vector<SteamUtils::GameInfo> games;
    std::vector<SteamUtils::LogFile> logFiles;
    int selectedGameIndex = -1;
    bool steamDirFound = false;
    bool scanningGames = false;
    std::string statusMessage = "Click 'Find Steam Directory' to begin";

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

        if (ImGui::BeginMenuBar())
        {
            if (ImGui::BeginMenu("File"))
            {
                if (ImGui::MenuItem("Exit"))
                    glfwSetWindowShouldClose(window, true);
                ImGui::EndMenu();
            }
            ImGui::EndMenuBar();
        }

        ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Status: %s",
                           statusMessage.c_str());
        ImGui::Separator();

        ImGui::Text("Steam Directory:");
        ImGui::SameLine();
        if (ImGui::Button("Find Steam Directory"))
        {
            statusMessage = "Searching for Steam...";
            steamDir = SteamUtils::findSteamDirectory();
            if (!steamDir.empty())
            {
                steamDirFound = true;
                statusMessage = "Steam directory found!";
                Logger::log("Found Steam directory: " + steamDir);
            }
            else
            {
                steamDirFound = false;
                statusMessage = "Steam directory not found";
            }
        }

        if (!steamDir.empty())
        {
            ImGui::TextWrapped("%s", steamDir.c_str());
        }

        ImGui::Separator();

        if (steamDirFound)
        {
            if (ImGui::Button("Scan for Games"))
            {
                statusMessage = "Scanning for games...";
                games = SteamUtils::getInstalledGames(steamDir);
                statusMessage =
                    "Found " + std::to_string(games.size()) + " games";
                selectedGameIndex = -1;
                logFiles.clear();
            }
        }

        if (!games.empty())
        {
            ImGui::Separator();
            ImGui::Text("Installed Games (%zu):", games.size());

            ImGui::BeginChild("GamesList",
                              ImVec2(0, 200),
                              true,
                              ImGuiWindowFlags_HorizontalScrollbar);

            for (int i = 0; i < games.size(); i++)
            {
                if (ImGui::Selectable(games[i].name.c_str(),
                                      selectedGameIndex == i))
                {
                    selectedGameIndex = i;
                    logFiles.clear();
                }
            }

            ImGui::EndChild();

            if (selectedGameIndex >= 0 &&
                selectedGameIndex < games.size())
            {
                ImGui::Separator();
                const auto &game = games[selectedGameIndex];
                ImGui::Text("Selected Game: %s", game.name.c_str());
                ImGui::Text("App ID: %s", game.appId.c_str());
                ImGui::Text("Install Dir: %s", game.installDir.c_str());

                if (ImGui::Button("Find Log Files"))
                {
                    statusMessage = "Searching for log files...";
                    logFiles = SteamUtils::findGameLogs(steamDir, game);
                    statusMessage =
                        "Found " + std::to_string(logFiles.size()) +
                        " log files";
                }
            }
        }

        if (!logFiles.empty())
        {
            ImGui::Separator();
            ImGui::Text("Found Log Files (%zu):", logFiles.size());

            ImGui::BeginChild("LogFilesList", ImVec2(0, 250), true);

            ImGui::Columns(4, "logfiles");
            ImGui::Separator();
            ImGui::Text("File Name");
            ImGui::NextColumn();
            ImGui::Text("Type");
            ImGui::NextColumn();
            ImGui::Text("Size");
            ImGui::NextColumn();
            ImGui::Text("Modified");
            ImGui::NextColumn();
            ImGui::Separator();

            for (const auto &log : logFiles)
            {
                ImGui::Text("%s", log.filename.c_str());
                ImGui::NextColumn();
                ImGui::Text("%s", log.type.c_str());
                ImGui::NextColumn();
                ImGui::Text(
                    "%s", SteamUtils::formatFileSize(log.size).c_str());
                ImGui::NextColumn();
                ImGui::Text("%s", log.lastModified.c_str());
                ImGui::NextColumn();
            }

            ImGui::Columns(1);
            ImGui::EndChild();

            if (ImGui::Button("Copy Log Files to ~/steam-logs"))
            {
                const auto &game = games[selectedGameIndex];
                std::string outputDir =
                    SteamUtils::createOutputDirectory(game.name);

                if (!outputDir.empty())
                {
                    int copied = SteamUtils::copyLogsToDirectory(
                        logFiles, outputDir, game.name);
                    statusMessage = "Copied " + std::to_string(copied) +
                                    " files to " + outputDir;
                }
                else
                {
                    statusMessage = "Failed to create output directory";
                }
            }
        }

        ImGui::End();

        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.45f, 0.55f, 0.60f, 1.00f);
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
