#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <GLFW/glfw3.h>
#include <iostream>

#include "app_state.hpp"
#include "colors.hpp"
#include "fonts.hpp"
#include "theme.hpp"
#include "ui_widgets.hpp"
#include "toast.hpp"
#include "resource_path.hpp"

#include "welcome_screen.hpp"
#include "game_selection_screen.hpp"
#include "log_files_screen.hpp"
#include "file_preview.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

static void glfw_error_callback(int error, const char *description)
{
    fprintf(stderr, "GLFW Error %d: %s\n", error, description);
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
        ImGui::PushFont(UIFonts::GetLarge());
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

int main([[maybe_unused]] int argc, [[maybe_unused]] char *argv[])
{

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

    SetWindowIcon(window, GetResourcePath("resources/SLC-logo.png").string().c_str());

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
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
        UIToast::Render();

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
