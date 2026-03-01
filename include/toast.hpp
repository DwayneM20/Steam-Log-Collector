#ifndef TOAST_HPP
#define TOAST_HPP

#include <imgui.h>
#include <string>
#include <vector>
#include <chrono>
#include "colors.hpp"
#include "fonts.hpp"

namespace UIToast
{
    enum class Type
    {
        INFO,
        SUCCESS,
        WARNING,
        ERROR
    };

    struct Toast
    {
        std::string message;
        Type type;
        std::chrono::steady_clock::time_point createdAt;
        float duration;
        float opacity = 1.0f;
        int id;
    };

    inline std::vector<Toast> toasts;
    inline int nextToastId = 0;

    inline void Show(const std::string &message, Type type = Type::INFO,
                     float duration = 3.0f)
    {
        Toast toast;
        toast.message = message;
        toast.type = type;
        toast.createdAt = std::chrono::steady_clock::now();
        toast.duration = duration;
        toast.id = nextToastId++;
        toasts.push_back(toast);
    }

    inline void Success(const std::string &message, float duration = 3.0f)
    {
        Show(message, Type::SUCCESS, duration);
    }

    inline void Warning(const std::string &message, float duration = 3.0f)
    {
        Show(message, Type::WARNING, duration);
    }

    inline void Error(const std::string &message, float duration = 3.0f)
    {
        Show(message, Type::ERROR, duration);
    }

    inline void Info(const std::string &message, float duration = 3.0f)
    {
        Show(message, Type::INFO, duration);
    }

    inline ImVec4 GetColor(Type type)
    {
        switch (type)
        {
        case Type::SUCCESS:
            return UIColors::Success;
        case Type::WARNING:
            return UIColors::Warning;
        case Type::ERROR:
            return UIColors::Error;
        case Type::INFO:
        default:
            return UIColors::Info;
        }
    }

    inline const char *GetIcon(Type type)
    {
        switch (type)
        {
        case Type::SUCCESS:
            return u8"\u2714"; // Check mark
        case Type::WARNING:
            return u8"\u26A0"; // Warning sign
        case Type::ERROR:
            return u8"\u2716"; // Cross mark
        case Type::INFO:
        default:
            return u8"\u2139"; // Information sign
        }
    }

    inline void Render()
    {
        if (toasts.empty())
            return;

        auto now = std::chrono::steady_clock::now();
        ImVec2 displaySize = ImGui::GetIO().DisplaySize;

        constexpr float padding = 20.0f;
        constexpr float toastWidth = 350.0f;
        constexpr float toastHeight = 60.0f;
        constexpr float spacing = 10.0f;
        constexpr float fadeInDuration = 0.2f;
        constexpr float fadeOutDuration = 0.5f;
        constexpr float minVisibleOpacity = 0.01f;
        constexpr float toastRounding = 8.0f;

        float startX = displaySize.x - toastWidth - padding;
        float startY = padding;

        toasts.erase(
            std::remove_if(toasts.begin(), toasts.end(),
                           [&now, fadeOutDuration](const Toast &t)
                           {
                               float elapsed =
                                   std::chrono::duration<float>(now - t.createdAt)
                                       .count();
                               return elapsed > t.duration + fadeOutDuration;
                           }),
            toasts.end());

        int visibleIndex = 0;
        for (auto &toast : toasts)
        {
            float elapsed =
                std::chrono::duration<float>(now - toast.createdAt).count();

            if (elapsed < fadeInDuration)
            {
                toast.opacity = elapsed / fadeInDuration;
            }
            else if (elapsed > toast.duration - fadeOutDuration)
            {
                toast.opacity =
                    1.0f -
                    (elapsed - (toast.duration - fadeOutDuration)) / fadeOutDuration;
                toast.opacity = std::max(0.0f, toast.opacity);
            }
            else
            {
                toast.opacity = 1.0f;
            }

            if (toast.opacity <= minVisibleOpacity)
                continue;

            ImVec4 color = GetColor(toast.type);
            float yPos = startY + visibleIndex * (toastHeight + spacing);

            ImGui::SetNextWindowPos(ImVec2(startX, yPos));
            ImGui::SetNextWindowSize(ImVec2(toastWidth, 0));

            ImGui::PushStyleVar(ImGuiStyleVar_Alpha, toast.opacity);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, toastRounding);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(15, 12));
            ImGui::PushStyleColor(
                ImGuiCol_WindowBg,
                ImVec4(UIColors::DeepNavy.x, UIColors::DeepNavy.y,
                       UIColors::DeepNavy.z, 0.95f));
            ImGui::PushStyleColor(ImGuiCol_Border, color);

            std::string windowName = "##Toast" + std::to_string(toast.id);
            ImGui::Begin(windowName.c_str(), nullptr,
                         ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                             ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar |
                             ImGuiWindowFlags_NoSavedSettings |
                             ImGuiWindowFlags_AlwaysAutoResize |
                             ImGuiWindowFlags_NoFocusOnAppearing |
                             ImGuiWindowFlags_NoNav);

            ImGui::TextColored(color, "%s", GetIcon(toast.type));
            ImGui::SameLine();

            ImGui::PushTextWrapPos(ImGui::GetCursorPosX() + toastWidth - 60);
            ImGui::TextColored(UIColors::OffWhite, "%s", toast.message.c_str());
            ImGui::PopTextWrapPos();

            ImGui::SameLine(toastWidth - 40);
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered,
                                  ImVec4(color.x, color.y, color.z, 0.3f));
            if (ImGui::SmallButton("x"))
            {
                toast.opacity = 0.0f;
            }
            ImGui::PopStyleColor(2);

            ImGui::End();
            ImGui::PopStyleColor(2);
            ImGui::PopStyleVar(3);

            visibleIndex++;
        }
    }
}

#endif