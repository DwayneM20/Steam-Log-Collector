#ifndef THEME_HPP
#define THEME_HPP

#include <imgui.h>
#include "colors.hpp"

namespace UITheme
{
    inline void ApplyModernStyle()
    {
        ImGuiStyle &style = ImGui::GetStyle();

        style.WindowRounding = 12.0f;
        style.ChildRounding = 8.0f;
        style.FrameRounding = 6.0f;
        style.PopupRounding = 8.0f;
        style.ScrollbarRounding = 9.0f;
        style.GrabRounding = 6.0f;
        style.TabRounding = 6.0f;

        style.WindowPadding = ImVec2(16, 16);
        style.FramePadding = ImVec2(12, 8);
        style.ItemSpacing = ImVec2(12, 10);
        style.ItemInnerSpacing = ImVec2(10, 8);
        style.IndentSpacing = 24.0f;
        style.ScrollbarSize = 16.0f;
        style.GrabMinSize = 14.0f;

        style.WindowBorderSize = 1.0f;
        style.ChildBorderSize = 1.0f;
        style.PopupBorderSize = 1.0f;
        style.FrameBorderSize = 1.0f;

        ImVec4 *colors = style.Colors;

        colors[ImGuiCol_Text] = UIColors::OffWhite;
        colors[ImGuiCol_TextDisabled] = UIColors::CoolGray;
        colors[ImGuiCol_TextSelectedBg] = ImVec4(
            UIColors::LavenderBlue.x,
            UIColors::LavenderBlue.y,
            UIColors::LavenderBlue.z,
            0.35f);

        colors[ImGuiCol_WindowBg] = UIColors::DarkBg;
        colors[ImGuiCol_ChildBg] = ImVec4(
            UIColors::DarkBg.x * 0.95f,
            UIColors::DarkBg.y * 0.95f,
            UIColors::DarkBg.z * 0.95f,
            1.0f);
        colors[ImGuiCol_PopupBg] = UIColors::DeepNavy;
        colors[ImGuiCol_MenuBarBg] = UIColors::DeepNavy;

        colors[ImGuiCol_Border] = ImVec4(
            UIColors::Base.x, UIColors::Base.y, UIColors::Base.z, 0.2f);
        colors[ImGuiCol_BorderShadow] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);

        colors[ImGuiCol_FrameBg] = UIColors::DeepNavy;
        colors[ImGuiCol_FrameBgHovered] = ImVec4(
            UIColors::Base.x, UIColors::Base.y, UIColors::Base.z, 0.4f);
        colors[ImGuiCol_FrameBgActive] = ImVec4(
            UIColors::Base.x, UIColors::Base.y, UIColors::Base.z, 0.6f);

        colors[ImGuiCol_TitleBg] = UIColors::DeepNavy;
        colors[ImGuiCol_TitleBgActive] = ImVec4(
            UIColors::DeepNavy.x * 1.1f,
            UIColors::DeepNavy.y * 1.1f,
            UIColors::DeepNavy.z * 1.1f,
            1.0f);
        colors[ImGuiCol_TitleBgCollapsed] = ImVec4(
            UIColors::DeepNavy.x, UIColors::DeepNavy.y,
            UIColors::DeepNavy.z, 0.75f);

        colors[ImGuiCol_ScrollbarBg] = ImVec4(
            UIColors::DeepNavy.x, UIColors::DeepNavy.y,
            UIColors::DeepNavy.z, 0.53f);
        colors[ImGuiCol_ScrollbarGrab] = UIColors::CoolGray;
        colors[ImGuiCol_ScrollbarGrabHovered] = UIColors::Base;
        colors[ImGuiCol_ScrollbarGrabActive] = UIColors::LavenderBlue;

        colors[ImGuiCol_CheckMark] = UIColors::LavenderBlue;
        colors[ImGuiCol_SliderGrab] = UIColors::Base;
        colors[ImGuiCol_SliderGrabActive] = UIColors::LavenderBlue;

        colors[ImGuiCol_Button] = UIColors::Base;
        colors[ImGuiCol_ButtonHovered] = UIColors::LavenderBlue;
        colors[ImGuiCol_ButtonActive] = UIColors::SoftCoral;

        colors[ImGuiCol_Header] = ImVec4(
            UIColors::Base.x, UIColors::Base.y, UIColors::Base.z, 0.4f);
        colors[ImGuiCol_HeaderHovered] = ImVec4(
            UIColors::LavenderBlue.x,
            UIColors::LavenderBlue.y,
            UIColors::LavenderBlue.z,
            0.7f);
        colors[ImGuiCol_HeaderActive] = UIColors::LavenderBlue;

        colors[ImGuiCol_Separator] = ImVec4(
            UIColors::CoolGray.x, UIColors::CoolGray.y,
            UIColors::CoolGray.z, 0.3f);
        colors[ImGuiCol_SeparatorHovered] = UIColors::Base;
        colors[ImGuiCol_SeparatorActive] = UIColors::LavenderBlue;

        colors[ImGuiCol_ResizeGrip] = ImVec4(
            UIColors::Base.x, UIColors::Base.y, UIColors::Base.z, 0.25f);
        colors[ImGuiCol_ResizeGripHovered] = ImVec4(
            UIColors::LavenderBlue.x,
            UIColors::LavenderBlue.y,
            UIColors::LavenderBlue.z,
            0.67f);
        colors[ImGuiCol_ResizeGripActive] = UIColors::LavenderBlue;

        colors[ImGuiCol_Tab] = ImVec4(
            UIColors::Base.x, UIColors::Base.y, UIColors::Base.z, 0.3f);
        colors[ImGuiCol_TabHovered] = UIColors::LavenderBlue;
        colors[ImGuiCol_TabActive] = UIColors::Base;
        colors[ImGuiCol_TabUnfocused] = ImVec4(
            UIColors::CoolGray.x, UIColors::CoolGray.y,
            UIColors::CoolGray.z, 0.4f);
        colors[ImGuiCol_TabUnfocusedActive] = ImVec4(
            UIColors::CoolGray.x, UIColors::CoolGray.y,
            UIColors::CoolGray.z, 0.7f);

        colors[ImGuiCol_PlotLines] = UIColors::Base;
        colors[ImGuiCol_PlotLinesHovered] = UIColors::Terracotta;
        colors[ImGuiCol_PlotHistogram] = UIColors::LavenderBlue;
        colors[ImGuiCol_PlotHistogramHovered] = UIColors::Terracotta;

        colors[ImGuiCol_TableHeaderBg] = UIColors::DeepNavy;
        colors[ImGuiCol_TableBorderStrong] = UIColors::CoolGray;
        colors[ImGuiCol_TableBorderLight] = ImVec4(
            UIColors::CoolGray.x, UIColors::CoolGray.y,
            UIColors::CoolGray.z, 0.2f);
        colors[ImGuiCol_TableRowBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
        colors[ImGuiCol_TableRowBgAlt] = ImVec4(
            UIColors::Base.x, UIColors::Base.y, UIColors::Base.z, 0.1f);

        colors[ImGuiCol_DragDropTarget] = UIColors::Terracotta;

        colors[ImGuiCol_NavHighlight] = UIColors::LavenderBlue;
        colors[ImGuiCol_NavWindowingHighlight] = ImVec4(
            1.0f, 1.0f, 1.0f, 0.7f);
        colors[ImGuiCol_NavWindowingDimBg] = ImVec4(
            0.8f, 0.8f, 0.8f, 0.2f);
        colors[ImGuiCol_ModalWindowDimBg] = ImVec4(
            UIColors::DeepNavy.x, UIColors::DeepNavy.y,
            UIColors::DeepNavy.z, 0.5f);
    }
}

#endif