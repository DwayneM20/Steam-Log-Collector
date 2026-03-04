#pragma once

#include <imgui.h>
#include <chrono>
#include <string>
#include <string_view>
#include <vector>

namespace UIToast
{
    enum class Type
    {
        Info,
        Success,
        Warning,
        Error
    };

    [[nodiscard]] const ImVec4 &color_for(Type type) noexcept;

    class Toast
    {
    public:
        Toast(std::string message, Type type, float duration, int id);

        [[nodiscard]] float elapsed() const noexcept;
        [[nodiscard]] float opacity() const noexcept;
        [[nodiscard]] bool is_expired() const noexcept;
        [[nodiscard]] Type type() const noexcept { return type_; }
        [[nodiscard]] int id() const noexcept { return id_; }
        [[nodiscard]] std::string_view message() const noexcept { return message_; }

        void dismiss() noexcept;

    private:
        std::string message_;
        Type type_;
        std::chrono::steady_clock::time_point created_at_;
        float duration_;
        bool dismissed_ = false;
        int id_;

        static constexpr float kFadeIn = 0.2f;
        static constexpr float kFadeOut = 0.5f;
    };

    void Show(std::string_view message, Type type = Type::Info,
              float duration = 3.0f);
    void Success(std::string_view message, float duration = 3.0f);
    void Warning(std::string_view message, float duration = 3.0f);
    void Error(std::string_view message, float duration = 3.0f);
    void Info(std::string_view message, float duration = 3.0f);
    void Render();

}
