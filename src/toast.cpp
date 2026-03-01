#include "toast.hpp"
#include "colors.hpp"
#include <algorithm>
#include <array>
#include <cmath>

namespace UIToast
{
    // --- Type metadata ---

    const ImVec4 &color_for(Type type)
    {
        static const auto table = std::array<ImVec4, 4>{{
            UIColors::Info,
            UIColors::Success,
            UIColors::Warning,
            UIColors::Error,
        }};
        return table.at(static_cast<std::size_t>(type));
    }

    // --- Toast implementation ---

    Toast::Toast(std::string message, Type type, float duration, int id)
        : message_{std::move(message)},
          type_{type},
          created_at_{std::chrono::steady_clock::now()},
          duration_{duration},
          id_{id}
    {
    }

    float Toast::elapsed() const
    {
        const auto now = std::chrono::steady_clock::now();
        return std::chrono::duration<float>(now - created_at_).count();
    }

    float Toast::opacity() const
    {
        const float t = elapsed();

        if (dismissed_)
            return 0.0f;
        if (t < kFadeIn)
            return t / kFadeIn;
        if (t > duration_ - kFadeOut)
            return std::max(0.0f,
                            1.0f - (t - (duration_ - kFadeOut)) / kFadeOut);
        return 1.0f;
    }

    bool Toast::is_expired() const
    {
        return dismissed_ || elapsed() > duration_ + kFadeOut;
    }

    void Toast::dismiss()
    {
        dismissed_ = true;
    }

    // --- Module-private rendering helpers ---

    namespace
    {
        std::vector<Toast> toasts;
        int next_id = 0;

        constexpr float kPadding = 20.0f;
        constexpr float kToastWidth = 380.0f;
        constexpr float kSpacing = 8.0f;
        constexpr float kRounding = 10.0f;
        constexpr float kMinVisibleOpacity = 0.01f;
        constexpr float kAccentBarWidth = 4.0f;
        constexpr float kIconRadius = 7.0f;
        constexpr float kIconAreaWidth = 28.0f;
        constexpr float kCloseButtonSize = 20.0f;
        constexpr ImVec2 kWindowPadding{0.0f, 0.0f};

        constexpr ImGuiWindowFlags kToastFlags =
            ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar |
            ImGuiWindowFlags_NoSavedSettings |
            ImGuiWindowFlags_AlwaysAutoResize |
            ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;

        ImU32 to_im_color(const ImVec4 &c, float alpha = 1.0f)
        {
            return IM_COL32(
                static_cast<int>(c.x * 255),
                static_cast<int>(c.y * 255),
                static_cast<int>(c.z * 255),
                static_cast<int>(c.w * alpha * 255));
        }

        void draw_icon(ImDrawList *dl, ImVec2 center, Type type,
                       const ImVec4 &color, float alpha)
        {
            const ImU32 col = to_im_color(color, alpha);
            const float r = kIconRadius;

            switch (type)
            {
            case Type::Success:
            {
                // Circle with checkmark
                dl->AddCircle(center, r, col, 0, 1.8f);
                const ImVec2 pts[3] = {
                    {center.x - 3.0f, center.y + 0.5f},
                    {center.x - 0.5f, center.y + 3.0f},
                    {center.x + 4.0f, center.y - 2.5f}};
                dl->AddPolyline(pts, 3, col, ImDrawFlags_None, 1.8f);
                break;
            }
            case Type::Warning:
            {
                // Triangle with exclamation
                const ImVec2 p1{center.x, center.y - r};
                const ImVec2 p2{center.x - r, center.y + r * 0.7f};
                const ImVec2 p3{center.x + r, center.y + r * 0.7f};
                dl->AddTriangle(p1, p2, p3, col, 1.8f);
                dl->AddLine(
                    ImVec2(center.x, center.y - 2.0f),
                    ImVec2(center.x, center.y + 1.0f), col, 1.8f);
                dl->AddCircleFilled(
                    ImVec2(center.x, center.y + 3.0f), 1.0f, col);
                break;
            }
            case Type::Error:
            {
                // Circle with X
                dl->AddCircle(center, r, col, 0, 1.8f);
                const float d = 3.0f;
                dl->AddLine(
                    ImVec2(center.x - d, center.y - d),
                    ImVec2(center.x + d, center.y + d), col, 1.8f);
                dl->AddLine(
                    ImVec2(center.x + d, center.y - d),
                    ImVec2(center.x - d, center.y + d), col, 1.8f);
                break;
            }
            case Type::Info:
            default:
            {
                // Circle with "i"
                dl->AddCircle(center, r, col, 0, 1.8f);
                dl->AddCircleFilled(
                    ImVec2(center.x, center.y - 3.0f), 1.2f, col);
                dl->AddLine(
                    ImVec2(center.x, center.y - 0.5f),
                    ImVec2(center.x, center.y + 4.0f), col, 1.8f);
                break;
            }
            }
        }

        void prune_expired()
        {
            toasts.erase(
                std::remove_if(toasts.begin(), toasts.end(),
                               [](const Toast &t) { return t.is_expired(); }),
                toasts.end());
        }

        void render_single(Toast &toast, float y_offset)
        {
            const float alpha = toast.opacity();
            if (alpha <= kMinVisibleOpacity)
                return;

            const ImVec4 &color = color_for(toast.type());
            const ImVec2 display_size = ImGui::GetIO().DisplaySize;
            const float x_pos = display_size.x - kToastWidth - kPadding;

            ImGui::SetNextWindowPos(ImVec2(x_pos, y_offset));
            ImGui::SetNextWindowSize(ImVec2(kToastWidth, 0));

            // Use real window padding for content inset (left side adds
            // accent bar width). This avoids SetCursorPos boundary issues.
            constexpr float kContentPadX = 14.0f;
            constexpr float kContentPadY = 12.0f;
            const ImVec2 win_padding{
                kAccentBarWidth + kContentPadX, kContentPadY};

            ImGui::PushStyleVar(ImGuiStyleVar_Alpha, alpha);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, kRounding);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, win_padding);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
            ImGui::PushStyleColor(
                ImGuiCol_WindowBg,
                ImVec4(UIColors::DeepNavy.x, UIColors::DeepNavy.y,
                       UIColors::DeepNavy.z, 0.95f));

            const std::string window_name =
                "##Toast" + std::to_string(toast.id());

            ImGui::Begin(window_name.c_str(), nullptr, kToastFlags);

            // Draw icon via DrawList, vertically centered on the first
            // line of text (font size ~18px, icon diameter ~14px)
            auto *dl = ImGui::GetWindowDrawList();
            const ImVec2 icon_screen = ImGui::GetCursorScreenPos();
            const float font_height = ImGui::GetFontSize();
            const ImVec2 icon_center{
                icon_screen.x + kIconRadius,
                icon_screen.y + font_height * 0.5f};
            draw_icon(dl, icon_center, toast.type(), color, alpha);

            // Reserve space for the icon, then place text on the same line
            ImGui::Dummy(ImVec2(kIconAreaWidth, font_height));
            ImGui::SameLine();

            // Message text
            const float text_width =
                kToastWidth - win_padding.x - kContentPadX -
                kIconAreaWidth - kCloseButtonSize;
            ImGui::PushTextWrapPos(ImGui::GetCursorPosX() + text_width);
            ImGui::TextColored(UIColors::OffWhite, "%s",
                               std::string(toast.message()).c_str());
            ImGui::PopTextWrapPos();

            // Close button on the same line, right-aligned
            ImGui::SameLine(kToastWidth - win_padding.x -
                            kCloseButtonSize + 2.0f);
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered,
                                  ImVec4(color.x, color.y, color.z, 0.2f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive,
                                  ImVec4(color.x, color.y, color.z, 0.4f));
            ImGui::PushStyleColor(ImGuiCol_Text,
                                  ImVec4(color.x, color.y, color.z, 0.7f));
            ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.0f);

            const std::string close_id =
                "x##toast_close_" + std::to_string(toast.id());
            if (ImGui::SmallButton(close_id.c_str()))
            {
                toast.dismiss();
            }
            ImGui::PopStyleVar(1);
            ImGui::PopStyleColor(4);

            // Draw left accent bar over the window background
            const ImVec2 win_pos = ImGui::GetWindowPos();
            const ImVec2 win_size = ImGui::GetWindowSize();
            dl->AddRectFilled(
                win_pos,
                ImVec2(win_pos.x + kAccentBarWidth,
                       win_pos.y + win_size.y),
                to_im_color(color, alpha),
                kRounding, ImDrawFlags_RoundCornersLeft);

            ImGui::End();
            ImGui::PopStyleColor(1);
            ImGui::PopStyleVar(4);
        }
    } // anonymous namespace

    // --- Public API ---

    void Show(std::string_view message, Type type, float duration)
    {
        toasts.emplace_back(std::string(message), type, duration, next_id++);
    }

    void Success(std::string_view message, float duration)
    {
        Show(message, Type::Success, duration);
    }

    void Warning(std::string_view message, float duration)
    {
        Show(message, Type::Warning, duration);
    }

    void Error(std::string_view message, float duration)
    {
        Show(message, Type::Error, duration);
    }

    void Info(std::string_view message, float duration)
    {
        Show(message, Type::Info, duration);
    }

    void Render()
    {
        if (toasts.empty())
            return;

        prune_expired();

        const ImVec2 display_size = ImGui::GetIO().DisplaySize;
        float y_offset = kPadding;

        for (auto &toast : toasts)
        {
            render_single(toast, y_offset);
            y_offset += kSpacing + 60.0f;
        }
    }

}
