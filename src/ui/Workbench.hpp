#pragma once

#include "ui/Theme.hpp"

#include <imgui.h>

#include <string_view>

namespace ui::workbench {

enum class StatusTone {
    Neutral,
    Success,
    Warning,
    Error,
};

bool primaryButton(const char* label, const ImVec2& size = ImVec2(0.0F, 0.0F));
bool quietButton(const char* label, const ImVec2& size = ImVec2(0.0F, 0.0F));
bool segmentedControl(const char* id, int& selectedIndex, const char* const* labels, int labelCount);

void drawStatus(std::string_view message, StatusTone tone);
void drawToolbarSpacer();

bool beginPanel(
    const char* id,
    std::string_view title,
    std::string_view detail,
    const ImVec2& size = ImVec2(0.0F, 0.0F)
);
void endPanel();

} // namespace ui::workbench
