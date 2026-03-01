#pragma once

#include <imgui.h>

namespace UIFonts
{
    [[nodiscard]] ImFont *GetDefault();
    [[nodiscard]] ImFont *GetLarge();
    [[nodiscard]] ImFont *GetMedium();
    [[nodiscard]] ImFont *GetTitle();
    [[nodiscard]] ImFont *GetSmall();

    void LoadFonts(ImGuiIO &io);
}
