#ifndef COLORS_HPP
#define COLORS_HPP

#include <imgui.h>

namespace UIColors
{

    inline const ImVec4 Base = ImVec4(
        161.0f / 255.0f, 202.0f / 255.0f, 241.0f / 255.0f, 1.0f); // Soft blue
    inline const ImVec4 LightTeal = ImVec4(
        161.0f / 255.0f, 241.0f / 255.0f, 232.0f / 255.0f, 1.0f); // Light teal
    inline const ImVec4 LavenderBlue = ImVec4(
        173.0f / 255.0f, 161.0f / 255.0f, 241.0f / 255.0f, 1.0f); // Lavender blue
    inline const ImVec4 SoftCoral = ImVec4(
        241.0f / 255.0f, 200.0f / 255.0f, 161.0f / 255.0f, 1.0f); // Soft coral
    inline const ImVec4 Terracotta = ImVec4(
        241.0f / 255.0f, 177.0f / 255.0f, 120.0f / 255.0f, 1.0f); // Terracotta
    inline const ImVec4 OffWhite = ImVec4(
        245.0f / 255.0f, 247.0f / 255.0f, 252.0f / 255.0f, 1.0f); // Off-white
    inline const ImVec4 CoolGray = ImVec4(
        110.0f / 255.0f, 132.0f / 255.0f, 153.0f / 255.0f, 1.0f); // Cool gray
    inline const ImVec4 DeepNavy = ImVec4(
        45.0f / 255.0f, 61.0f / 255.0f, 81.0f / 255.0f, 1.0f); // Deep navy
    inline const ImVec4 DarkBg = ImVec4(
        32.0f / 255.0f, 35.0f / 255.0f, 42.0f / 255.0f, 1.0f); // Dark background

    inline const ImVec4 Success = ImVec4(
        74.0f / 255.0f, 222.0f / 255.0f, 128.0f / 255.0f, 1.0f); // Green
    inline const ImVec4 Warning = ImVec4(
        251.0f / 255.0f, 191.0f / 255.0f, 36.0f / 255.0f, 1.0f); // Yellow
    inline const ImVec4 Error = ImVec4(
        239.0f / 255.0f, 68.0f / 255.0f, 68.0f / 255.0f, 1.0f); // Red
    inline const ImVec4 Info = ImVec4(
        59.0f / 255.0f, 130.0f / 255.0f, 246.0f / 255.0f, 1.0f); // Blue
}

#endif