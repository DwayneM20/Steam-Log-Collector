#include "file_preview.hpp"

#include <imgui.h>
#include <fstream>
#include <sstream>

#include "colors.hpp"
#include "fonts.hpp"
#include "ui_widgets.hpp"
#include "steam-utils.hpp"

std::string ReadFileContent(const std::filesystem::path &filePath,
                            size_t maxBytes)
{
    std::ifstream file(filePath, std::ios::binary);
    if (!file.is_open())
    {
        return "Error: Could not open file for reading.";
    }

    std::ostringstream content;
    char buffer[kReadBufferSize];
    size_t bytesRead = 0;

    while (bytesRead < maxBytes && file.read(buffer, sizeof(buffer)))
    {
        size_t count = file.gcount();
        content.write(buffer, count);
        bytesRead += count;
    }

    if (bytesRead < maxBytes && file.gcount() > 0)
    {
        content.write(buffer, file.gcount());
        bytesRead += file.gcount();
    }

    std::string result = content.str();
    if (bytesRead >= maxBytes)
    {
        result += "\n\n[Preview truncated - file is larger than 1MB]";
    }

    return result;
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

            ImGui::PushFont(UIFonts::GetLarge());
            ImGui::TextColored(UIColors::LavenderBlue, "%s",
                               log.filename.c_str());
            ImGui::PopFont();

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            UIWidgets::InfoText("Path:", log.path.string());
            UIWidgets::InfoText("Size:",
                                SteamUtils::formatFileSize(log.size));
            UIWidgets::InfoText("Type:", log.type);
            UIWidgets::InfoText("Modified:", log.lastModified);

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            ImGui::BeginChild("PreviewContent", ImVec2(0, -50), true,
                              ImGuiWindowFlags_HorizontalScrollbar);
            ImGui::PushFont(UIFonts::GetMedium());
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
