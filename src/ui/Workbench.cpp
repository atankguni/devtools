#include "ui/Workbench.hpp"

#include <algorithm>

namespace ui::workbench {

namespace {

ImU32 withAlpha(ImU32 color, int alpha)
{
    return (color & IM_COL32(255, 255, 255, 0)) | IM_COL32(0, 0, 0, alpha);
}

ImU32 statusColor(StatusTone tone)
{
    switch (tone) {
    case StatusTone::Success:
        return IM_COL32(22, 163, 74, 255);
    case StatusTone::Warning:
        return IM_COL32(217, 119, 6, 255);
    case StatusTone::Error:
        return IM_COL32(220, 38, 38, 255);
    case StatusTone::Neutral:
        return ImGui::GetColorU32(ImGuiCol_CheckMark);
    }
    return ImGui::GetColorU32(ImGuiCol_CheckMark);
}

bool buttonWithColors(const char* label, ImU32 color, ImU32 hovered, ImU32 active, const ImVec2& size)
{
    ImGui::PushStyleColor(ImGuiCol_Button, ImGui::ColorConvertU32ToFloat4(color));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImGui::ColorConvertU32ToFloat4(hovered));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImGui::ColorConvertU32ToFloat4(active));
    const bool pressed = ImGui::Button(label, size);
    ImGui::PopStyleColor(3);
    return pressed;
}

} // namespace

bool primaryButton(const char* label, const ImVec2& size)
{
    const ImU32 color = ImGui::GetColorU32(ImGuiCol_CheckMark);
    return buttonWithColors(label, color, withAlpha(color, 220), withAlpha(color, 180), size);
}

bool quietButton(const char* label, const ImVec2& size)
{
    return buttonWithColors(
        label,
        ImGui::GetColorU32(ImGuiCol_FrameBg),
        ImGui::GetColorU32(ImGuiCol_FrameBgHovered),
        ImGui::GetColorU32(ImGuiCol_FrameBgActive),
        size
    );
}

bool segmentedControl(const char* id, int& selectedIndex, const char* const* labels, int labelCount)
{
    bool changed = false;
    ImGui::PushID(id);
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(2.0F, 0.0F));
    for (int index = 0; index < labelCount; ++index) {
        if (index > 0) {
            ImGui::SameLine();
        }

        const bool selected = selectedIndex == index;
        if (selected) {
            ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_HeaderActive));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImGui::GetStyleColorVec4(ImGuiCol_HeaderActive));
        }

        if (ImGui::Button(labels[index], ImVec2(82.0F, 0.0F)) && !selected) {
            selectedIndex = index;
            changed = true;
        }

        if (selected) {
            ImGui::PopStyleColor(2);
        }
    }
    ImGui::PopStyleVar();
    ImGui::PopID();
    return changed;
}

void drawStatus(std::string_view message, StatusTone tone)
{
    if (message.empty()) {
        return;
    }

    const ImVec2 start = ImGui::GetCursorScreenPos();
    const ImVec2 textSize = ImGui::CalcTextSize(message.data(), message.data() + message.size());
    const ImVec2 size(std::max(textSize.x + 28.0F, 160.0F), textSize.y + 13.0F);
    const ImU32 toneColor = statusColor(tone);

    ImGui::Dummy(size);
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    drawList->AddRectFilled(start, ImVec2(start.x + size.x, start.y + size.y), withAlpha(toneColor, 28), 6.0F);
    drawList->AddRectFilled(start, ImVec2(start.x + 3.0F, start.y + size.y), toneColor, 6.0F);
    drawList->AddText(
        ImVec2(start.x + 12.0F, start.y + 6.0F),
        ImGui::GetColorU32(ImGuiCol_Text),
        message.data(),
        message.data() + message.size()
    );
}

void drawToolbarSpacer()
{
    ImGui::SameLine();
    ImGui::Dummy(ImVec2(8.0F, 0.0F));
    ImGui::SameLine();
}

bool beginPanel(const char* id, std::string_view title, std::string_view detail, const ImVec2& size)
{
    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImGui::GetStyleColorVec4(ImGuiCol_ChildBg));
    ImGui::PushStyleColor(ImGuiCol_Border, ImGui::GetStyleColorVec4(ImGuiCol_Border));
    ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 8.0F);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(12.0F, 12.0F));

    const bool open = ImGui::BeginChild(id, size, true);
    if (open) {
        ImGui::TextUnformatted(title.data(), title.data() + title.size());
        if (!detail.empty()) {
            ImGui::SameLine();
            ImGui::TextDisabled("%.*s", static_cast<int>(detail.size()), detail.data());
        }
        ImGui::Separator();
        ImGui::Dummy(ImVec2(0.0F, 4.0F));
    }
    return open;
}

void endPanel()
{
    ImGui::EndChild();
    ImGui::PopStyleVar(2);
    ImGui::PopStyleColor(2);
}

} // namespace ui::workbench
