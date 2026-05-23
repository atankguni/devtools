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
    .backgroundTop = IM_COL32(10, 15, 22, 255),
    .backgroundBottom = IM_COL32(14, 24, 34, 255),
    .backgroundOverlay = IM_COL32(20, 34, 48, 62),
    .panel = IM_COL32(18, 26, 36, 238),
    .surface = IM_COL32(14, 22, 32, 255),
    .border = IM_COL32(56, 76, 96, 150),
    .accent = IM_COL32(67, 176, 255, 255),
    .accentMuted = IM_COL32(67, 176, 255, 42),
    .row = IM_COL32(16, 24, 34, 110),
    .rowHovered = IM_COL32(26, 38, 52, 230),
    .rowSelected = IM_COL32(22, 58, 82, 230),
    .rowSelectedBorder = IM_COL32(70, 176, 255, 150),
    .rowText = IM_COL32(220, 228, 236, 255),
    .rowSelectedText = IM_COL32(236, 248, 255, 255),
    .rowDescription = IM_COL32(137, 151, 166, 255),
    .pillText = IM_COL32(139, 217, 255, 255),
    .brandOuter = IM_COL32(31, 120, 190, 255),
    .brandInner = IM_COL32(108, 210, 255, 255),
};

constexpr ThemePalette lightPalette {
    .backgroundTop = IM_COL32(238, 242, 246, 255),
    .backgroundBottom = IM_COL32(222, 229, 236, 255),
    .backgroundOverlay = IM_COL32(255, 255, 255, 80),
    .panel = IM_COL32(252, 253, 255, 242),
    .surface = IM_COL32(246, 249, 252, 255),
    .border = IM_COL32(170, 184, 199, 160),
    .accent = IM_COL32(0, 116, 184, 255),
    .accentMuted = IM_COL32(0, 116, 184, 32),
    .row = IM_COL32(242, 246, 250, 180),
    .rowHovered = IM_COL32(230, 238, 246, 245),
    .rowSelected = IM_COL32(213, 232, 246, 245),
    .rowSelectedBorder = IM_COL32(0, 116, 184, 120),
    .rowText = IM_COL32(31, 42, 55, 255),
    .rowSelectedText = IM_COL32(12, 38, 58, 255),
    .rowDescription = IM_COL32(95, 108, 123, 255),
    .pillText = IM_COL32(0, 104, 166, 255),
    .brandOuter = IM_COL32(0, 116, 184, 255),
    .brandInner = IM_COL32(92, 183, 230, 255),
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
    style.FrameRounding = 7.0F;
    style.PopupRounding = 10.0F;
    style.ScrollbarRounding = 8.0F;
    style.GrabRounding = 7.0F;
    style.TabRounding = 7.0F;
    style.WindowBorderSize = 0.0F;
    style.ChildBorderSize = 1.0F;
    style.FrameBorderSize = 0.0F;
    style.PopupBorderSize = 1.0F;
    style.WindowPadding = ImVec2(14.0F, 14.0F);
    style.FramePadding = ImVec2(10.0F, 6.0F);
    style.ItemSpacing = ImVec2(9.0F, 7.0F);
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
    colors[ImGuiCol_Button] = ImVec4(0.12F, 0.25F, 0.39F, 1.00F);
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.16F, 0.36F, 0.56F, 1.00F);
    colors[ImGuiCol_ButtonActive] = ImVec4(0.19F, 0.47F, 0.72F, 1.00F);
    colors[ImGuiCol_FrameBg] = ImVec4(0.095F, 0.115F, 0.140F, 1.00F);
    colors[ImGuiCol_FrameBgHovered] = ImVec4(0.13F, 0.18F, 0.23F, 1.00F);
    colors[ImGuiCol_FrameBgActive] = ImVec4(0.15F, 0.24F, 0.34F, 1.00F);
    colors[ImGuiCol_Tab] = ImVec4(0.08F, 0.12F, 0.16F, 1.00F);
    colors[ImGuiCol_TabHovered] = ImVec4(0.16F, 0.36F, 0.56F, 1.00F);
    colors[ImGuiCol_TabActive] = ImVec4(0.12F, 0.24F, 0.37F, 1.00F);
    colors[ImGuiCol_CheckMark] = ImVec4(0.29F, 0.72F, 1.00F, 1.00F);
    colors[ImGuiCol_SliderGrab] = ImVec4(0.25F, 0.62F, 0.90F, 1.00F);
    colors[ImGuiCol_SliderGrabActive] = ImVec4(0.33F, 0.76F, 1.00F, 1.00F);
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
    colors[ImGuiCol_Button] = ImVec4(0.80F, 0.89F, 0.96F, 1.00F);
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.70F, 0.83F, 0.93F, 1.00F);
    colors[ImGuiCol_ButtonActive] = ImVec4(0.58F, 0.76F, 0.88F, 1.00F);
    colors[ImGuiCol_FrameBg] = ImVec4(0.93F, 0.96F, 0.98F, 1.00F);
    colors[ImGuiCol_FrameBgHovered] = ImVec4(0.87F, 0.92F, 0.96F, 1.00F);
    colors[ImGuiCol_FrameBgActive] = ImVec4(0.78F, 0.88F, 0.95F, 1.00F);
    colors[ImGuiCol_Tab] = ImVec4(0.88F, 0.93F, 0.97F, 1.00F);
    colors[ImGuiCol_TabHovered] = ImVec4(0.72F, 0.84F, 0.93F, 1.00F);
    colors[ImGuiCol_TabActive] = ImVec4(0.78F, 0.88F, 0.95F, 1.00F);
    colors[ImGuiCol_CheckMark] = ImVec4(0.00F, 0.45F, 0.72F, 1.00F);
    colors[ImGuiCol_SliderGrab] = ImVec4(0.08F, 0.50F, 0.76F, 1.00F);
    colors[ImGuiCol_SliderGrabActive] = ImVec4(0.00F, 0.39F, 0.66F, 1.00F);
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
