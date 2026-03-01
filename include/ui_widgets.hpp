#pragma once

#include <imgui.h>
#include <string>
#include "colors.hpp"
#include "fonts.hpp"

namespace UIWidgets
{
    inline void StatusBadge(
        const std::string &text, const ImVec4 &color)
    {
        ImGui::PushStyleColor(ImGuiCol_Button, color);
        ImGui::PushStyleColor(
            ImGuiCol_ButtonHovered,
            ImVec4(color.x * 1.1f, color.y * 1.1f, color.z * 1.1f, 1.0f));
        ImGui::PushStyleColor(
            ImGuiCol_ButtonActive,
            ImVec4(color.x * 0.9f, color.y * 0.9f, color.z * 0.9f, 1.0f));
        ImGui::Button(text.c_str());
        ImGui::PopStyleColor(3);
    }

    inline void SectionHeader(const std::string &title)
    {
        ImGui::PushFont(UIFonts::GetLarge());
        ImGui::TextColored(UIColors::LavenderBlue, "%s", title.c_str());
        ImGui::PopFont();
        ImGui::Separator();
        ImGui::Spacing();
    }

    [[nodiscard]] inline bool PrimaryButton(
        const std::string &label, const ImVec2 &size = ImVec2(0, 0))
    {
        ImGui::PushStyleColor(ImGuiCol_Button, UIColors::Base);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, UIColors::LavenderBlue);
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, UIColors::SoftCoral);
        bool result = ImGui::Button(label.c_str(), size);
        ImGui::PopStyleColor(3);
        return result;
    }

    [[nodiscard]] inline bool SecondaryButton(
        const std::string &label, const ImVec2 &size = ImVec2(0, 0))
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
        bool result = ImGui::Button(label.c_str(), size);
        ImGui::PopStyleColor(3);
        return result;
    }

    inline void InfoText(const std::string &label, const std::string &value)
    {
        ImGui::TextColored(UIColors::CoolGray, "%s", label.c_str());
        ImGui::SameLine();
        ImGui::Text("%s", value.c_str());
    }
}