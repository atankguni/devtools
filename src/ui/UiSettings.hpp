#pragma once

#include <string>

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

enum class StartupBehavior {
    LastTool,
    SpecificTool,
    FirstTool,
};

enum class UiDensity {
    Comfortable,
    Compact,
};

struct UiSettings {
    ThemeMode themeMode = ThemeMode::FollowSystem;
    FontFamily fontFamily = FontFamily::System;
    float fontSize = 15.0F;
    StartupBehavior startupBehavior = StartupBehavior::LastTool;
    UiDensity density = UiDensity::Comfortable;
    bool showSidebarDescriptions = true;
    bool includeSettingsInCommandPalette = true;
    bool showCopyConfirmation = true;
    bool autoCopyGeneratedOutput = false;
    bool softWrapText = true;
    bool useSpacesForTabs = true;
    bool confirmBeforeClearingInput = false;
    bool neverStoreToolInputs = true;
    bool clearInputsOnExit = false;
    int tabWidth = 4;
    std::string startupToolId;
    std::string lastActiveToolId;
};

const char* themeModeLabel(ThemeMode mode);
const char* fontFamilyLabel(FontFamily family);
const char* startupBehaviorLabel(StartupBehavior behavior);
const char* uiDensityLabel(UiDensity density);

UiSettings loadSettings();
void saveSettings(const UiSettings& settings);

} // namespace ui
