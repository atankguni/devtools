#include "ui/Theme.hpp"

#include <imgui.h>

namespace ui {

void applyDarkTheme()
{
    ImGui::StyleColorsDark();

    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowRounding = 8.0F;
    style.ChildRounding = 8.0F;
    style.FrameRounding = 6.0F;
    style.PopupRounding = 8.0F;
    style.ScrollbarRounding = 8.0F;
    style.GrabRounding = 6.0F;
    style.TabRounding = 6.0F;
    style.WindowBorderSize = 0.0F;
    style.ChildBorderSize = 1.0F;
    style.FrameBorderSize = 0.0F;
    style.WindowPadding = ImVec2(14.0F, 14.0F);
    style.FramePadding = ImVec2(10.0F, 6.0F);
    style.ItemSpacing = ImVec2(10.0F, 8.0F);
    style.ItemInnerSpacing = ImVec2(8.0F, 6.0F);
    style.ScrollbarSize = 12.0F;
    style.IndentSpacing = 18.0F;

    ImVec4* colors = style.Colors;
    colors[ImGuiCol_Text] = ImVec4(0.88F, 0.90F, 0.92F, 1.00F);
    colors[ImGuiCol_TextDisabled] = ImVec4(0.48F, 0.52F, 0.56F, 1.00F);
    colors[ImGuiCol_WindowBg] = ImVec4(0.075F, 0.080F, 0.088F, 1.00F);
    colors[ImGuiCol_ChildBg] = ImVec4(0.105F, 0.112F, 0.124F, 1.00F);
    colors[ImGuiCol_Border] = ImVec4(0.18F, 0.20F, 0.22F, 1.00F);
    colors[ImGuiCol_Separator] = ImVec4(0.20F, 0.22F, 0.24F, 1.00F);
    colors[ImGuiCol_Header] = ImVec4(0.18F, 0.25F, 0.36F, 0.72F);
    colors[ImGuiCol_HeaderHovered] = ImVec4(0.22F, 0.32F, 0.48F, 0.86F);
    colors[ImGuiCol_HeaderActive] = ImVec4(0.27F, 0.39F, 0.58F, 1.00F);
    colors[ImGuiCol_Button] = ImVec4(0.15F, 0.18F, 0.20F, 1.00F);
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.20F, 0.28F, 0.40F, 1.00F);
    colors[ImGuiCol_ButtonActive] = ImVec4(0.25F, 0.36F, 0.54F, 1.00F);
    colors[ImGuiCol_FrameBg] = ImVec4(0.13F, 0.145F, 0.16F, 1.00F);
    colors[ImGuiCol_FrameBgHovered] = ImVec4(0.17F, 0.20F, 0.22F, 1.00F);
    colors[ImGuiCol_FrameBgActive] = ImVec4(0.20F, 0.25F, 0.34F, 1.00F);
    colors[ImGuiCol_Tab] = ImVec4(0.12F, 0.14F, 0.16F, 1.00F);
    colors[ImGuiCol_TabHovered] = ImVec4(0.20F, 0.28F, 0.42F, 1.00F);
    colors[ImGuiCol_TabActive] = ImVec4(0.16F, 0.22F, 0.32F, 1.00F);
    colors[ImGuiCol_ScrollbarBg] = ImVec4(0.08F, 0.09F, 0.10F, 1.00F);
    colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.24F, 0.27F, 0.30F, 1.00F);
    colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.30F, 0.35F, 0.38F, 1.00F);
    colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.34F, 0.39F, 0.50F, 1.00F);
}

} // namespace ui
