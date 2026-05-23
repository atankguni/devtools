#pragma once

#include "core/ToolRegistry.hpp"
#include "ui/UiSettings.hpp"

#include <array>
#include <string>

namespace ui {

class UiShell {
public:
    void draw(const core::ToolRegistry& registry);
    const UiSettings& settings() const;

private:
    void drawCommandPalette(const core::ToolRegistry& registry);
    void drawSidebar(const core::ToolRegistry& registry);
    void drawCurrentTool(const core::ToolRegistry& registry);
    void drawSettings();

    std::string activeToolId_;
    UiSettings settings_;
    bool commandPaletteOpen_ = false;
    std::array<char, 128> commandSearch_ {};
};

} // namespace ui
