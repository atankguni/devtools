#pragma once

#include "core/ToolRegistry.hpp"

#include <array>
#include <string>

namespace ui {

class UiShell {
public:
    void draw(const core::ToolRegistry& registry);

private:
    void drawCommandPalette(const core::ToolRegistry& registry);
    void drawSidebar(const core::ToolRegistry& registry);
    void drawCurrentTool(const core::ToolRegistry& registry);

    std::string activeToolId_;
    bool commandPaletteOpen_ = false;
    std::array<char, 128> commandSearch_ {};
};

} // namespace ui
