#include "log_files_screen.hpp"

#include <imgui.h>
#include <algorithm>
#include <string>

#include "colors.hpp"
#include "fonts.hpp"
#include "ui_widgets.hpp"
#include "toast.hpp"
#include "steam-utils.hpp"
#include "file_preview.hpp"

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

    ImGui::PushFont(UIFonts::GetTitle());
    ImGui::TextColored(UIColors::LavenderBlue, "%s", game.name.c_str());
    ImGui::PopFont();

    ImGui::Spacing();

    // Info columns
    float colWidth = (contentWidth - cardPadding * 2) / 3.0f;
    ImGui::Columns(3, nullptr, false);
    ImGui::SetColumnWidth(0, colWidth);
    ImGui::SetColumnWidth(1, colWidth);
    ImGui::SetColumnWidth(2, colWidth);

    ImGui::PushFont(UIFonts::GetMedium());
    ImGui::TextColored(UIColors::CoolGray, "APP ID");
    ImGui::PopFont();
    ImGui::PushFont(UIFonts::GetDefault());
    ImGui::Text("%s", game.appId.c_str());
    ImGui::PopFont();

    ImGui::NextColumn();

    ImGui::PushFont(UIFonts::GetMedium());
    ImGui::TextColored(UIColors::CoolGray, "INSTALL DIRECTORY");
    ImGui::PopFont();
    ImGui::PushFont(UIFonts::GetDefault());
    ImGui::TextWrapped("%s", game.installDir.c_str());
    ImGui::PopFont();

    ImGui::NextColumn();

    ImGui::PushFont(UIFonts::GetMedium());
    ImGui::TextColored(UIColors::CoolGray, "LOG FILES FOUND");
    ImGui::PopFont();
    ImGui::PushFont(UIFonts::GetDefault());
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

        ImGui::PushFont(UIFonts::GetLarge());
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
            std::fill(state.selectedLogs.begin(), state.selectedLogs.end(), true);
        }

        ImGui::SameLine();

        if (UIWidgets::SecondaryButton("Deselect All",
                                       ImVec2(150, buttonHeight)))
        {
            std::fill(state.selectedLogs.begin(), state.selectedLogs.end(), false);
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
        int selectedCount = static_cast<int>(
            std::count(state.selectedLogs.begin(), state.selectedLogs.end(), true));

        float copyButtonWidth = 220.0f;
        ImGui::SameLine(contentWidth - copyButtonWidth);

        if (selectedCount == 0)
            ImGui::BeginDisabled();

        std::string copyText =
            "Copy Selected (" + std::to_string(selectedCount) + ")";
        if (UIWidgets::PrimaryButton(copyText.c_str(),
                                     ImVec2(copyButtonWidth, buttonHeight)))
        {
            std::filesystem::path outputDir =
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
                UIToast::Success(
                    "Copied " + std::to_string(copied) +
                    " log files to: " + outputDir.string());
            }
            else
            {
                UIToast::Error("Failed to create output directory.");
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
                ImGui::PushFont(UIFonts::GetSmall());
                ImGui::Text("%s", log.lastModified.c_str());
                ImGui::PopFont();
            }

            ImGui::EndTable();
        }

        ImGui::EndChild();
    }
}
