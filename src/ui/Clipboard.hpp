#pragma once

#include "ui/UiSettings.hpp"

namespace ui {

void applyClipboardSettings(const UiSettings& settings);
void copyToClipboard(const char* text);
void drawClipboardStatus();

} // namespace ui
