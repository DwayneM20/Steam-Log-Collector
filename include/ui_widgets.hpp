#pragma once

#include <imgui.h>
#include <string_view>
#include "colors.hpp"
#include "fonts.hpp"

namespace UIWidgets
{
    inline void StatusBadge(
        std::string_view text, const ImVec4 &color)
    {
        ImGui::PushStyleColor(ImGuiCol_Button, color);
        ImGui::PushStyleColor(
            ImGuiCol_ButtonHovered,
            ImVec4(color.x * 1.1f, color.y * 1.1f, color.z * 1.1f, 1.0f));
        ImGui::PushStyleColor(
            ImGuiCol_ButtonActive,
            ImVec4(color.x * 0.9f, color.y * 0.9f, color.z * 0.9f, 1.0f));
        ImGui::Button(text.data());
        ImGui::PopStyleColor(3);
    }

    inline void SectionHeader(std::string_view title)
    {
        ImGui::PushFont(UIFonts::GetLarge());
        ImGui::TextColored(UIColors::LavenderBlue, "%s", title.data());
        ImGui::PopFont();
        ImGui::Separator();
        ImGui::Spacing();
    }

    [[nodiscard]] inline bool PrimaryButton(
        std::string_view label, const ImVec2 &size = ImVec2(0, 0))
    {
        ImGui::PushStyleColor(ImGuiCol_Button, UIColors::Base);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, UIColors::LavenderBlue);
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, UIColors::SoftCoral);
        bool result = ImGui::Button(label.data(), size);
        ImGui::PopStyleColor(3);
        return result;
    }

    [[nodiscard]] inline bool SecondaryButton(
        std::string_view label, const ImVec2 &size = ImVec2(0, 0))
    {
        ImGui::PushStyleColor(ImGuiCol_Button, UIColors::DeepNavy);
        ImGui::PushStyleColor(
            ImGuiCol_ButtonHovered,
            ImVec4(UIColors::CoolGray.x, UIColors::CoolGray.y,
                   UIColors::CoolGray.z, 0.8f));
        ImGui::PushStyleColor(
            ImGuiCol_ButtonActive,
            ImVec4(UIColors::CoolGray.x, UIColors::CoolGray.y,
                   UIColors::CoolGray.z, 1.0f));
        bool result = ImGui::Button(label.data(), size);
        ImGui::PopStyleColor(3);
        return result;
    }

    inline void InfoText(std::string_view label, std::string_view value)
    {
        ImGui::TextColored(UIColors::CoolGray, "%s", label.data());
        ImGui::SameLine();
        ImGui::Text("%s", value.data());
    }
}