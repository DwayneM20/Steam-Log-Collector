#ifndef FONTS_HPP
#define FONTS_HPP

#include <imgui.h>

namespace UIFonts
{
    ImFont *GetDefault();
    ImFont *GetLarge();
    ImFont *GetMedium();
    ImFont *GetTitle();
    ImFont *GetSmall();

    void LoadFonts(ImGuiIO &io);
}

#endif
