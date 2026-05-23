#pragma once

namespace ui {

enum class ThemeMode {
    FollowSystem,
    Light,
    Dark,
};

enum class FontFamily {
    System,
    Monospace,
    Serif,
    Classic,
};

struct UiSettings {
    ThemeMode themeMode = ThemeMode::FollowSystem;
    FontFamily fontFamily = FontFamily::System;
    float fontSize = 15.0F;
};

const char* themeModeLabel(ThemeMode mode);
const char* fontFamilyLabel(FontFamily family);

} // namespace ui
