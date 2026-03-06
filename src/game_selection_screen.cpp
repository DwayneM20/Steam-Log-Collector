#include "game_selection_screen.hpp"

#include <imgui.h>
#include <algorithm>

#include "colors.hpp"
#include "fonts.hpp"
#include "ui_widgets.hpp"
#include "steam-utils.hpp"

void RenderGameSelectionScreen(AppState &state)
{
    ImVec2 windowSize = ImGui::GetContentRegionAvail();
    float padding = std::max(windowSize.x * 0.03f, 20.0f);
    float contentWidth = windowSize.x - padding * 2;

    ImGui::SetCursorPosX(padding);

    // Header row
    ImGui::BeginGroup();

    ImGui::PushFont(UIFonts::GetTitle());
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
    ImGui::PushFont(UIFonts::GetDefault());
    ImGui::TextColored(UIColors::CoolGray, "Steam Directory:");
    ImGui::SameLine();
    ImGui::TextColored(UIColors::OffWhite, "%s", state.steamDir.string().c_str());

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

        ImGui::PushFont(UIFonts::GetLarge());
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

        ImGui::PushFont(UIFonts::GetDefault());
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

            ImGui::PushFont(UIFonts::GetLarge());
            ImGui::TextColored(
                hovered ? UIColors::LavenderBlue : UIColors::OffWhite, "%s",
                game.name.c_str());
            ImGui::PopFont();

            ImGui::SetCursorPosX(15);
            ImGui::PushFont(UIFonts::GetSmall());
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
