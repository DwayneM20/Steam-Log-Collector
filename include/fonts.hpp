#ifndef FONTS_HPP
#define FONTS_HPP

#include <imgui.h>

namespace UIFonts
{
    inline ImFont *Default = nullptr;
    inline ImFont *Large = nullptr;
    inline ImFont *Medium = nullptr;
    inline ImFont *Title = nullptr;
    inline ImFont *Small = nullptr;

    inline void LoadFonts(ImGuiIO &io)
    {
        ImFontConfig config;
        config.OversampleH = 2;
        config.OversampleV = 1;

        config.SizePixels = 18.0f;
        Default = io.Fonts->AddFontDefault(&config);

        config.SizePixels = 24.0f;
        Large = io.Fonts->AddFontDefault(&config);

        config.SizePixels = 28.0f;
        Title = io.Fonts->AddFontDefault(&config);

        config.SizePixels = 20.0f;
        Medium = io.Fonts->AddFontDefault(&config);

        config.SizePixels = 14.0f;
        Small = io.Fonts->AddFontDefault(&config);
    }
}

#endif