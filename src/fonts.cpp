#include "fonts.hpp"
#include "logger.hpp"
#include <filesystem>

namespace UIFonts
{
    static ImFont *sDefault = nullptr;
    static ImFont *sLarge = nullptr;
    static ImFont *sMedium = nullptr;
    static ImFont *sTitle = nullptr;
    static ImFont *sSmall = nullptr;

    ImFont *GetDefault() { return sDefault; }
    ImFont *GetLarge() { return sLarge; }
    ImFont *GetMedium() { return sMedium; }
    ImFont *GetTitle() { return sTitle; }
    ImFont *GetSmall() { return sSmall; }

    void LoadFonts(ImGuiIO &io)
    {
        ImFontConfig config;
        config.OversampleH = 2;
        config.OversampleV = 1;

        static const ImWchar glyphRanges[] = {
            0x0020,
            0x00FF, // Basic Latin + Latin Supplement
            0x2000,
            0x206F, // General Punctuation
            0x2100,
            0x214F, // Letterlike Symbols (includes ℹ)
            0x2190,
            0x21FF, // Arrows
            0x2200,
            0x22FF, // Mathematical Operators
            0x2300,
            0x23FF, // Miscellaneous Technical
            0x2500,
            0x257F, // Box Drawing
            0x2600,
            0x26FF, // Miscellaneous Symbols (includes ⚠)
            0x2700,
            0x27BF, // Dingbats (includes ✔ ✖)
            0,
        };

        const char *fontPath = "resources/DejaVuSansMono.ttf";
        bool fontLoaded = false;

        if (std::filesystem::exists(fontPath))
        {
            sDefault = io.Fonts->AddFontFromFileTTF(fontPath, 18.0f, &config,
                                                     glyphRanges);
            if (sDefault)
            {
                sLarge = io.Fonts->AddFontFromFileTTF(fontPath, 24.0f, &config,
                                                       glyphRanges);
                sTitle = io.Fonts->AddFontFromFileTTF(fontPath, 28.0f, &config,
                                                       glyphRanges);
                sMedium = io.Fonts->AddFontFromFileTTF(fontPath, 20.0f, &config,
                                                        glyphRanges);
                sSmall = io.Fonts->AddFontFromFileTTF(fontPath, 14.0f, &config,
                                                       glyphRanges);
                fontLoaded = true;
                Logger::log("Loaded DejaVu Sans Mono font successfully",
                            SeverityLevel::Info);
            }
        }
        else
        {
            Logger::log("Font file not found: " + std::string(fontPath),
                        SeverityLevel::Warning);
        }

        if (!fontLoaded)
        {
            Logger::log("Falling back to default ImGui font",
                        SeverityLevel::Warning);

            config.SizePixels = 18.0f;
            sDefault = io.Fonts->AddFontDefault(&config);

            config.SizePixels = 24.0f;
            sLarge = io.Fonts->AddFontDefault(&config);

            config.SizePixels = 28.0f;
            sTitle = io.Fonts->AddFontDefault(&config);

            config.SizePixels = 20.0f;
            sMedium = io.Fonts->AddFontDefault(&config);

            config.SizePixels = 14.0f;
            sSmall = io.Fonts->AddFontDefault(&config);
        }
    }
}
