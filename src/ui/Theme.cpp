#include "ui/Theme.hpp"

#include <imgui.h>

namespace ui {

void applyDarkTheme()
{
    ImGui::StyleColorsDark();

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

} // namespace ui
