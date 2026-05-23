#include "ui/UiSettings.hpp"

#include <algorithm>
#include <charconv>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <optional>
#include <string_view>

namespace ui {

namespace {

std::filesystem::path settingsPath()
{
    const char* home = std::getenv("HOME");
    if (home == nullptr || std::string_view(home).empty()) {
        return ".devtools-settings.ini";
    }

#if defined(__APPLE__)
    return std::filesystem::path(home) / "Library" / "Application Support" / "DevTools" / "settings.ini";
#elif defined(_WIN32)
    const char* appData = std::getenv("APPDATA");
    if (appData != nullptr && !std::string_view(appData).empty()) {
        return std::filesystem::path(appData) / "DevTools" / "settings.ini";
    }
    return std::filesystem::path(home) / "AppData" / "Roaming" / "DevTools" / "settings.ini";
#else
    const char* configHome = std::getenv("XDG_CONFIG_HOME");
    if (configHome != nullptr && !std::string_view(configHome).empty()) {
        return std::filesystem::path(configHome) / "devtools" / "settings.ini";
    }
    return std::filesystem::path(home) / ".config" / "devtools" / "settings.ini";
#endif
}

std::optional<int> parseInt(std::string_view value)
{
    int parsed = 0;
    const char* begin = value.data();
    const char* end = value.data() + value.size();
    const auto [ptr, error] = std::from_chars(begin, end, parsed);
    if (error != std::errc {} || ptr != end) {
        return std::nullopt;
    }
    return parsed;
}

std::optional<float> parseFloat(std::string_view value)
{
    std::string text(value);
    char* end = nullptr;
    const float parsed = std::strtof(text.c_str(), &end);
    if (end == nullptr || *end != '\0') {
        return std::nullopt;
    }
    return parsed;
}

bool parseBool(std::string_view value, bool fallback)
{
    if (value == "true" || value == "1") {
        return true;
    }
    if (value == "false" || value == "0") {
        return false;
    }
    return fallback;
}

std::string_view trim(std::string_view value)
{
    while (!value.empty() && (value.front() == ' ' || value.front() == '\t')) {
        value.remove_prefix(1);
    }
    while (!value.empty() && (value.back() == ' ' || value.back() == '\t' || value.back() == '\r')) {
        value.remove_suffix(1);
    }
    return value;
}

template <typename Enum>
Enum parseEnum(std::string_view value, Enum fallback, int maxValue)
{
    const std::optional<int> parsed = parseInt(value);
    if (!parsed.has_value() || *parsed < 0 || *parsed > maxValue) {
        return fallback;
    }
    return static_cast<Enum>(*parsed);
}

void clampSettings(UiSettings& settings)
{
    settings.fontSize = std::clamp(settings.fontSize, 12.0F, 22.0F);
    settings.tabWidth = std::clamp(settings.tabWidth, 2, 8);
}

} // namespace

const char* startupBehaviorLabel(StartupBehavior behavior)
{
    switch (behavior) {
    case StartupBehavior::LastTool:
        return "Last opened tool";
    case StartupBehavior::SpecificTool:
        return "Specific tool";
    case StartupBehavior::FirstTool:
        return "First tool";
    }
    return "Last opened tool";
}

const char* uiDensityLabel(UiDensity density)
{
    switch (density) {
    case UiDensity::Comfortable:
        return "Comfortable";
    case UiDensity::Compact:
        return "Compact";
    }
    return "Comfortable";
}

UiSettings loadSettings()
{
    UiSettings settings;
    std::ifstream file(settingsPath());
    if (!file.is_open()) {
        return settings;
    }

    std::string line;
    while (std::getline(file, line)) {
        const std::string_view raw(line);
        const std::size_t separator = raw.find('=');
        if (separator == std::string_view::npos) {
            continue;
        }

        const std::string_view key = trim(raw.substr(0, separator));
        const std::string_view value = trim(raw.substr(separator + 1));
        if (key == "theme") {
            settings.themeMode = parseEnum(value, settings.themeMode, 2);
        } else if (key == "font_family") {
            settings.fontFamily = parseEnum(value, settings.fontFamily, 3);
        } else if (key == "font_size") {
            if (const std::optional<float> parsed = parseFloat(value); parsed.has_value()) {
                settings.fontSize = *parsed;
            }
        } else if (key == "startup_behavior") {
            settings.startupBehavior = parseEnum(value, settings.startupBehavior, 2);
        } else if (key == "density") {
            settings.density = parseEnum(value, settings.density, 1);
        } else if (key == "show_sidebar_descriptions") {
            settings.showSidebarDescriptions = parseBool(value, settings.showSidebarDescriptions);
        } else if (key == "include_settings_in_command_palette") {
            settings.includeSettingsInCommandPalette = parseBool(value, settings.includeSettingsInCommandPalette);
        } else if (key == "show_copy_confirmation") {
            settings.showCopyConfirmation = parseBool(value, settings.showCopyConfirmation);
        } else if (key == "auto_copy_generated_output") {
            settings.autoCopyGeneratedOutput = parseBool(value, settings.autoCopyGeneratedOutput);
        } else if (key == "soft_wrap_text") {
            settings.softWrapText = parseBool(value, settings.softWrapText);
        } else if (key == "use_spaces_for_tabs") {
            settings.useSpacesForTabs = parseBool(value, settings.useSpacesForTabs);
        } else if (key == "confirm_before_clearing_input") {
            settings.confirmBeforeClearingInput = parseBool(value, settings.confirmBeforeClearingInput);
        } else if (key == "never_store_tool_inputs") {
            settings.neverStoreToolInputs = parseBool(value, settings.neverStoreToolInputs);
        } else if (key == "clear_inputs_on_exit") {
            settings.clearInputsOnExit = parseBool(value, settings.clearInputsOnExit);
        } else if (key == "tab_width") {
            if (const std::optional<int> parsed = parseInt(value); parsed.has_value()) {
                settings.tabWidth = *parsed;
            }
        } else if (key == "startup_tool_id") {
            settings.startupToolId = std::string(value);
        } else if (key == "last_active_tool_id") {
            settings.lastActiveToolId = std::string(value);
        }
    }

    clampSettings(settings);
    return settings;
}

void saveSettings(const UiSettings& settings)
{
    const std::filesystem::path path = settingsPath();
    std::error_code error;
    std::filesystem::create_directories(path.parent_path(), error);

    std::ofstream file(path, std::ios::trunc);
    if (!file.is_open()) {
        return;
    }

    file << "theme=" << static_cast<int>(settings.themeMode) << '\n';
    file << "font_family=" << static_cast<int>(settings.fontFamily) << '\n';
    file << "font_size=" << settings.fontSize << '\n';
    file << "startup_behavior=" << static_cast<int>(settings.startupBehavior) << '\n';
    file << "density=" << static_cast<int>(settings.density) << '\n';
    file << "show_sidebar_descriptions=" << (settings.showSidebarDescriptions ? "true" : "false") << '\n';
    file << "include_settings_in_command_palette=" << (settings.includeSettingsInCommandPalette ? "true" : "false") << '\n';
    file << "show_copy_confirmation=" << (settings.showCopyConfirmation ? "true" : "false") << '\n';
    file << "auto_copy_generated_output=" << (settings.autoCopyGeneratedOutput ? "true" : "false") << '\n';
    file << "soft_wrap_text=" << (settings.softWrapText ? "true" : "false") << '\n';
    file << "use_spaces_for_tabs=" << (settings.useSpacesForTabs ? "true" : "false") << '\n';
    file << "confirm_before_clearing_input=" << (settings.confirmBeforeClearingInput ? "true" : "false") << '\n';
    file << "never_store_tool_inputs=" << (settings.neverStoreToolInputs ? "true" : "false") << '\n';
    file << "clear_inputs_on_exit=" << (settings.clearInputsOnExit ? "true" : "false") << '\n';
    file << "tab_width=" << settings.tabWidth << '\n';
    file << "startup_tool_id=" << settings.startupToolId << '\n';
    file << "last_active_tool_id=" << settings.lastActiveToolId << '\n';
}

} // namespace ui
