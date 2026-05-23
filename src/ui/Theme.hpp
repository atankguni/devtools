#pragma once

#include "ui/UiSettings.hpp"

namespace ui {

enum class ResolvedTheme {
    Light,
    Dark,
};

struct ThemePalette {
    unsigned int backgroundTop;
    unsigned int backgroundBottom;
    unsigned int backgroundOverlay;
    unsigned int panel;
    unsigned int surface;
    unsigned int border;
    unsigned int accent;
    unsigned int accentMuted;
    unsigned int row;
    unsigned int rowHovered;
    unsigned int rowSelected;
    unsigned int rowSelectedBorder;
    unsigned int rowText;
    unsigned int rowSelectedText;
    unsigned int rowDescription;
    unsigned int pillText;
    unsigned int brandOuter;
    unsigned int brandInner;
};

ResolvedTheme resolveThemeMode(ThemeMode mode);
const ThemePalette& themePalette(ResolvedTheme theme);
void applyTheme(ResolvedTheme theme);
void applyThemeMode(ThemeMode mode);
void applyDensity(UiDensity density);

} // namespace ui
