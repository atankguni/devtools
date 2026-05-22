#include "ui/Theme.hpp"

#include <imgui.h>

namespace ui {

void applyDarkTheme()
{
    ImGui::StyleColorsDark();

    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowRounding = 5.0F;
    style.FrameRounding = 4.0F;
    style.PopupRounding = 4.0F;
    style.ScrollbarRounding = 4.0F;
    style.GrabRounding = 4.0F;
    style.TabRounding = 4.0F;
    style.WindowBorderSize = 1.0F;
    style.FrameBorderSize = 0.0F;
    style.WindowPadding = ImVec2(10.0F, 10.0F);
    style.FramePadding = ImVec2(8.0F, 5.0F);
    style.ItemSpacing = ImVec2(8.0F, 7.0F);

    ImVec4* colors = style.Colors;
    colors[ImGuiCol_WindowBg] = ImVec4(0.08F, 0.09F, 0.10F, 1.00F);
    colors[ImGuiCol_ChildBg] = ImVec4(0.10F, 0.11F, 0.12F, 1.00F);
    colors[ImGuiCol_Header] = ImVec4(0.20F, 0.22F, 0.25F, 1.00F);
    colors[ImGuiCol_HeaderHovered] = ImVec4(0.25F, 0.28F, 0.32F, 1.00F);
    colors[ImGuiCol_HeaderActive] = ImVec4(0.30F, 0.34F, 0.38F, 1.00F);
    colors[ImGuiCol_Button] = ImVec4(0.18F, 0.20F, 0.23F, 1.00F);
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.24F, 0.27F, 0.31F, 1.00F);
    colors[ImGuiCol_ButtonActive] = ImVec4(0.29F, 0.33F, 0.37F, 1.00F);
    colors[ImGuiCol_FrameBg] = ImVec4(0.13F, 0.14F, 0.16F, 1.00F);
    colors[ImGuiCol_FrameBgHovered] = ImVec4(0.18F, 0.20F, 0.23F, 1.00F);
    colors[ImGuiCol_FrameBgActive] = ImVec4(0.22F, 0.25F, 0.29F, 1.00F);
    colors[ImGuiCol_Tab] = ImVec4(0.13F, 0.14F, 0.16F, 1.00F);
    colors[ImGuiCol_TabHovered] = ImVec4(0.25F, 0.28F, 0.32F, 1.00F);
    colors[ImGuiCol_TabActive] = ImVec4(0.18F, 0.20F, 0.23F, 1.00F);
}

} // namespace ui
