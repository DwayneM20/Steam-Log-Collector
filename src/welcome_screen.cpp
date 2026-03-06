#include "welcome_screen.hpp"

#include <imgui.h>
#include <algorithm>

#include "colors.hpp"
#include "fonts.hpp"
#include "ui_widgets.hpp"
#include "toast.hpp"
#include "logger.hpp"
#include "steam-utils.hpp"

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
    ImGui::PushFont(UIFonts::GetTitle());
    float titleWidth = ImGui::CalcTextSize("Steam Log Collector").x;
    ImGui::SetCursorPosX((windowSize.x - titleWidth) / 2.0f);
    ImGui::TextColored(UIColors::LavenderBlue, "Steam Log Collector");
    ImGui::PopFont();

    ImGui::Spacing();

    ImGui::PushFont(UIFonts::GetDefault());
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
        state.statusMessage = "Searching for Steam...";
        state.steamDir = SteamUtils::findSteamDirectory();

        if (!state.steamDir.empty())
        {
            state.steamDirFound = true;
            UIToast::Success("Steam directory auto-detected successfully.");
            Logger::log("Found Steam directory: " + state.steamDir.string(), SeverityLevel::Info);
            state.games = SteamUtils::getInstalledGames(state.steamDir);
            state.currentScreen = Screen::GameSelection;
        }
        else
        {
            UIToast::Error("Could not auto-detect Steam directory. Please enter it manually.");
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
        std::string manualPath = state.manualSteamDir;

        if (manualPath.empty())
        {
            UIToast::Error("Please enter a Steam directory path.");
        }
        else if (!SteamUtils::directoryExists(manualPath))
        {
            UIToast::Error("The specified directory does not exist: " + manualPath);
        }
        else if (!SteamUtils::isValidSteamDirectory(manualPath))
        {
            UIToast::Error("This does not appear to be a valid Steam directory.");
        }
        else
        {
            state.steamDir = manualPath;
            state.steamDirFound = true;
            UIToast::Success("Manual Steam directory set successfully.");
            Logger::log("Using manual Steam directory: " + state.steamDir.string(), SeverityLevel::Info);
            state.games = SteamUtils::getInstalledGames(state.steamDir);
            state.currentScreen = Screen::GameSelection;
        }
    }

    ImGui::EndGroup();
    ImGui::EndChild();
}
