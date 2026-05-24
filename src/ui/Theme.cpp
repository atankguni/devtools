#include "ui/Theme.hpp"

#include <imgui.h>

#include <array>
#include <cstdio>
#include <cstdlib>
#include <optional>
#include <string>

namespace ui {

namespace {

constexpr ThemePalette darkPalette {
    .backgroundTop = IM_COL32(13, 14, 18, 255),
    .backgroundBottom = IM_COL32(18, 20, 25, 255),
    .backgroundOverlay = IM_COL32(255, 255, 255, 18),
    .panel = IM_COL32(24, 26, 32, 246),
    .surface = IM_COL32(29, 32, 39, 255),
    .border = IM_COL32(66, 70, 80, 126),
    .accent = IM_COL32(45, 212, 191, 255),
    .accentMuted = IM_COL32(45, 212, 191, 34),
    .row = IM_COL32(24, 26, 32, 0),
    .rowHovered = IM_COL32(38, 42, 50, 230),
    .rowSelected = IM_COL32(35, 62, 61, 238),
    .rowSelectedBorder = IM_COL32(45, 212, 191, 120),
    .rowText = IM_COL32(229, 231, 235, 255),
    .rowSelectedText = IM_COL32(244, 255, 253, 255),
    .rowDescription = IM_COL32(149, 158, 173, 255),
    .pillText = IM_COL32(111, 231, 217, 255),
    .brandOuter = IM_COL32(45, 212, 191, 255),
    .brandInner = IM_COL32(15, 23, 42, 255),
};

constexpr ThemePalette lightPalette {
    .backgroundTop = IM_COL32(246, 247, 249, 255),
    .backgroundBottom = IM_COL32(235, 238, 242, 255),
    .backgroundOverlay = IM_COL32(255, 255, 255, 96),
    .panel = IM_COL32(255, 255, 255, 246),
    .surface = IM_COL32(248, 250, 252, 255),
    .border = IM_COL32(205, 213, 223, 172),
    .accent = IM_COL32(13, 148, 136, 255),
    .accentMuted = IM_COL32(13, 148, 136, 30),
    .row = IM_COL32(255, 255, 255, 0),
    .rowHovered = IM_COL32(241, 245, 249, 245),
    .rowSelected = IM_COL32(219, 242, 239, 245),
    .rowSelectedBorder = IM_COL32(13, 148, 136, 108),
    .rowText = IM_COL32(24, 30, 41, 255),
    .rowSelectedText = IM_COL32(8, 73, 68, 255),
    .rowDescription = IM_COL32(91, 103, 118, 255),
    .pillText = IM_COL32(10, 111, 103, 255),
    .brandOuter = IM_COL32(13, 148, 136, 255),
    .brandInner = IM_COL32(240, 253, 250, 255),
};

bool detectSystemDarkTheme()
{
    static std::optional<bool> cachedSystemDarkTheme;
    if (cachedSystemDarkTheme.has_value()) {
        return *cachedSystemDarkTheme;
    }

#if defined(__APPLE__)
    std::array<char, 64> buffer {};
    std::string output;
    FILE* pipe = popen("defaults read -g AppleInterfaceStyle 2>/dev/null", "r");
    if (pipe == nullptr) {
        cachedSystemDarkTheme = false;
        return false;
    }

    while (fgets(buffer.data(), static_cast<int>(buffer.size()), pipe) != nullptr) {
        output += buffer.data();
    }

    const int result = pclose(pipe);
    cachedSystemDarkTheme = result == 0 && output.find("Dark") != std::string::npos;
#else
    if (const char* gtkTheme = std::getenv("GTK_THEME"); gtkTheme != nullptr) {
        const std::string themeName(gtkTheme);
        cachedSystemDarkTheme = themeName.find("dark") != std::string::npos
            || themeName.find("Dark") != std::string::npos;
    } else {
        cachedSystemDarkTheme = false;
    }
#endif

    return *cachedSystemDarkTheme;
}

void applyBaseStyle()
{
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowRounding = 10.0F;
    style.ChildRounding = 10.0F;
    style.FrameRounding = 6.0F;
    style.PopupRounding = 10.0F;
    style.ScrollbarRounding = 8.0F;
    style.GrabRounding = 7.0F;
    style.TabRounding = 7.0F;
    style.WindowBorderSize = 0.0F;
    style.ChildBorderSize = 1.0F;
    style.FrameBorderSize = 0.0F;
    style.PopupBorderSize = 1.0F;
    style.WindowPadding = ImVec2(14.0F, 14.0F);
    style.FramePadding = ImVec2(10.0F, 7.0F);
    style.ItemSpacing = ImVec2(9.0F, 8.0F);
    style.ItemInnerSpacing = ImVec2(8.0F, 5.0F);
    style.ScrollbarSize = 10.0F;
    style.IndentSpacing = 16.0F;
    style.WindowMenuButtonPosition = ImGuiDir_None;
}

void applyDarkColors()
{
    ImGuiStyle& style = ImGui::GetStyle();

    ImVec4* colors = style.Colors;
    colors[ImGuiCol_Text] = ImVec4(0.91F, 0.94F, 0.97F, 1.00F);
    colors[ImGuiCol_TextDisabled] = ImVec4(0.52F, 0.57F, 0.63F, 1.00F);
    colors[ImGuiCol_WindowBg] = ImVec4(0.055F, 0.065F, 0.078F, 0.96F);
    colors[ImGuiCol_ChildBg] = ImVec4(0.075F, 0.090F, 0.110F, 1.00F);
    colors[ImGuiCol_PopupBg] = ImVec4(0.070F, 0.083F, 0.105F, 0.98F);
    colors[ImGuiCol_Border] = ImVec4(0.18F, 0.24F, 0.31F, 0.72F);
    colors[ImGuiCol_BorderShadow] = ImVec4(0.00F, 0.00F, 0.00F, 0.00F);
    colors[ImGuiCol_Separator] = ImVec4(0.18F, 0.24F, 0.31F, 0.80F);
    colors[ImGuiCol_Header] = ImVec4(0.12F, 0.30F, 0.48F, 0.76F);
    colors[ImGuiCol_HeaderHovered] = ImVec4(0.16F, 0.39F, 0.62F, 0.90F);
    colors[ImGuiCol_HeaderActive] = ImVec4(0.18F, 0.48F, 0.74F, 1.00F);
    colors[ImGuiCol_Button] = ImVec4(0.12F, 0.18F, 0.20F, 1.00F);
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.16F, 0.28F, 0.28F, 1.00F);
    colors[ImGuiCol_ButtonActive] = ImVec4(0.08F, 0.44F, 0.40F, 1.00F);
    colors[ImGuiCol_FrameBg] = ImVec4(0.105F, 0.118F, 0.145F, 1.00F);
    colors[ImGuiCol_FrameBgHovered] = ImVec4(0.14F, 0.16F, 0.19F, 1.00F);
    colors[ImGuiCol_FrameBgActive] = ImVec4(0.15F, 0.26F, 0.26F, 1.00F);
    colors[ImGuiCol_Tab] = ImVec4(0.08F, 0.12F, 0.16F, 1.00F);
    colors[ImGuiCol_TabHovered] = ImVec4(0.16F, 0.36F, 0.56F, 1.00F);
    colors[ImGuiCol_TabActive] = ImVec4(0.12F, 0.24F, 0.37F, 1.00F);
    colors[ImGuiCol_CheckMark] = ImVec4(0.18F, 0.83F, 0.75F, 1.00F);
    colors[ImGuiCol_SliderGrab] = ImVec4(0.13F, 0.68F, 0.62F, 1.00F);
    colors[ImGuiCol_SliderGrabActive] = ImVec4(0.18F, 0.83F, 0.75F, 1.00F);
    colors[ImGuiCol_ScrollbarBg] = ImVec4(0.055F, 0.065F, 0.078F, 1.00F);
    colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.22F, 0.29F, 0.36F, 1.00F);
    colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.30F, 0.39F, 0.48F, 1.00F);
    colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.34F, 0.50F, 0.66F, 1.00F);
}

void applyLightColors()
{
    ImGuiStyle& style = ImGui::GetStyle();
    ImVec4* colors = style.Colors;
    colors[ImGuiCol_Text] = ImVec4(0.13F, 0.17F, 0.22F, 1.00F);
    colors[ImGuiCol_TextDisabled] = ImVec4(0.43F, 0.49F, 0.56F, 1.00F);
    colors[ImGuiCol_WindowBg] = ImVec4(0.98F, 0.99F, 1.00F, 0.96F);
    colors[ImGuiCol_ChildBg] = ImVec4(0.96F, 0.98F, 0.99F, 1.00F);
    colors[ImGuiCol_PopupBg] = ImVec4(0.99F, 0.99F, 1.00F, 0.98F);
    colors[ImGuiCol_Border] = ImVec4(0.66F, 0.72F, 0.78F, 0.72F);
    colors[ImGuiCol_BorderShadow] = ImVec4(0.00F, 0.00F, 0.00F, 0.00F);
    colors[ImGuiCol_Separator] = ImVec4(0.66F, 0.72F, 0.78F, 0.80F);
    colors[ImGuiCol_Header] = ImVec4(0.78F, 0.88F, 0.95F, 0.88F);
    colors[ImGuiCol_HeaderHovered] = ImVec4(0.68F, 0.82F, 0.92F, 0.95F);
    colors[ImGuiCol_HeaderActive] = ImVec4(0.58F, 0.76F, 0.88F, 1.00F);
    colors[ImGuiCol_Button] = ImVec4(0.88F, 0.96F, 0.95F, 1.00F);
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.78F, 0.92F, 0.90F, 1.00F);
    colors[ImGuiCol_ButtonActive] = ImVec4(0.62F, 0.84F, 0.81F, 1.00F);
    colors[ImGuiCol_FrameBg] = ImVec4(0.93F, 0.96F, 0.98F, 1.00F);
    colors[ImGuiCol_FrameBgHovered] = ImVec4(0.87F, 0.92F, 0.96F, 1.00F);
    colors[ImGuiCol_FrameBgActive] = ImVec4(0.78F, 0.88F, 0.95F, 1.00F);
    colors[ImGuiCol_Tab] = ImVec4(0.88F, 0.93F, 0.97F, 1.00F);
    colors[ImGuiCol_TabHovered] = ImVec4(0.72F, 0.84F, 0.93F, 1.00F);
    colors[ImGuiCol_TabActive] = ImVec4(0.78F, 0.88F, 0.95F, 1.00F);
    colors[ImGuiCol_CheckMark] = ImVec4(0.05F, 0.58F, 0.53F, 1.00F);
    colors[ImGuiCol_SliderGrab] = ImVec4(0.08F, 0.55F, 0.51F, 1.00F);
    colors[ImGuiCol_SliderGrabActive] = ImVec4(0.05F, 0.42F, 0.39F, 1.00F);
    colors[ImGuiCol_ScrollbarBg] = ImVec4(0.94F, 0.96F, 0.98F, 1.00F);
    colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.70F, 0.76F, 0.82F, 1.00F);
    colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.58F, 0.66F, 0.74F, 1.00F);
    colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.38F, 0.54F, 0.68F, 1.00F);
}

} // namespace

const char* themeModeLabel(ThemeMode mode)
{
    switch (mode) {
    case ThemeMode::FollowSystem:
        return "Follow system";
    case ThemeMode::Light:
        return "Light";
    case ThemeMode::Dark:
        return "Dark";
    }
    return "Follow system";
}

const char* fontFamilyLabel(FontFamily family)
{
    switch (family) {
    case FontFamily::System:
        return "System";
    case FontFamily::Monospace:
        return "Monospace";
    case FontFamily::Serif:
        return "Serif";
    case FontFamily::Classic:
        return "Classic";
    }
    return "System";
}

ResolvedTheme resolveThemeMode(ThemeMode mode)
{
    if (mode == ThemeMode::Light) {
        return ResolvedTheme::Light;
    }

    if (mode == ThemeMode::Dark) {
        return ResolvedTheme::Dark;
    }

    return detectSystemDarkTheme() ? ResolvedTheme::Dark : ResolvedTheme::Light;
}

const ThemePalette& themePalette(ResolvedTheme theme)
{
    return theme == ResolvedTheme::Light ? lightPalette : darkPalette;
}

void applyTheme(ResolvedTheme theme)
{
    if (theme == ResolvedTheme::Light) {
        ImGui::StyleColorsLight();
        applyBaseStyle();
        applyLightColors();
        return;
    }

    ImGui::StyleColorsDark();
    applyBaseStyle();
    applyDarkColors();
}

void applyThemeMode(ThemeMode mode)
{
    applyTheme(resolveThemeMode(mode));
}

void applyDensity(UiDensity density)
{
    ImGuiStyle& style = ImGui::GetStyle();
    if (density == UiDensity::Compact) {
        style.WindowPadding = ImVec2(10.0F, 10.0F);
        style.FramePadding = ImVec2(8.0F, 4.0F);
        style.ItemSpacing = ImVec2(7.0F, 5.0F);
        style.ItemInnerSpacing = ImVec2(6.0F, 4.0F);
        style.IndentSpacing = 13.0F;
        return;
    }

    style.WindowPadding = ImVec2(14.0F, 14.0F);
    style.FramePadding = ImVec2(10.0F, 6.0F);
    style.ItemSpacing = ImVec2(9.0F, 7.0F);
    style.ItemInnerSpacing = ImVec2(8.0F, 5.0F);
    style.IndentSpacing = 16.0F;
}

} // namespace ui
