#include "ui/UiShell.hpp"

#include "ui/Clipboard.hpp"
#include "ui/Theme.hpp"

#include <imgui.h>

#include <algorithm>
#include <cctype>
#include <string_view>

namespace {

constexpr float outerPadding = 12.0F;
constexpr float panelGap = 8.0F;
constexpr float sidebarWidth = 232.0F;
constexpr std::string_view settingsToolId = "__settings";

struct ToolCategory {
    std::string_view name;
    std::string_view ids;
};

constexpr std::array toolCategories {
    ToolCategory { "Format", "json_formatter sql_formatter markup text_diff" },
    ToolCategory { "Convert", "csv_json yaml_json timestamp number_base color permissions url_parser" },
    ToolCategory { "Encode", "base64 url jwt hash" },
    ToolCategory { "Inspect", "regex string_inspector cron line_tools" },
    ToolCategory { "Generate", "uuid password test_data" },
};

bool containsCaseInsensitive(std::string_view text, std::string_view query)
{
    if (query.empty()) {
        return true;
    }

    auto lower = [](unsigned char value) {
        return static_cast<char>(std::tolower(value));
    };

    for (std::size_t start = 0; start + query.size() <= text.size(); ++start) {
        bool matches = true;
        for (std::size_t index = 0; index < query.size(); ++index) {
            if (lower(static_cast<unsigned char>(text[start + index]))
                != lower(static_cast<unsigned char>(query[index]))) {
                matches = false;
                break;
            }
        }

        if (matches) {
            return true;
        }
    }

    return false;
}

bool categoryContains(const ToolCategory& category, std::string_view toolId)
{
    std::size_t start = 0;
    while (start < category.ids.size()) {
        const std::size_t end = category.ids.find(' ', start);
        const std::string_view id = category.ids.substr(start, end == std::string_view::npos ? end : end - start);
        if (id == toolId) {
            return true;
        }
        if (end == std::string_view::npos) {
            break;
        }
        start = end + 1;
    }
    return false;
}

std::string_view toolCategoryName(std::string_view toolId)
{
    for (const ToolCategory& category : toolCategories) {
        if (categoryContains(category, toolId)) {
            return category.name;
        }
    }
    return "Other";
}

bool toolMatchesQuery(const core::Tool& tool, std::string_view query)
{
    return containsCaseInsensitive(tool.name, query)
        || containsCaseInsensitive(tool.description, query)
        || containsCaseInsensitive(toolCategoryName(tool.id), query);
}

void drawViewportBackground(ImGuiViewport* viewport, const ui::ThemePalette& palette)
{
    ImDrawList* drawList = ImGui::GetBackgroundDrawList(viewport);
    const ImVec2 min = viewport->WorkPos;
    const ImVec2 max(viewport->WorkPos.x + viewport->WorkSize.x, viewport->WorkPos.y + viewport->WorkSize.y);
    drawList->AddRectFilledMultiColor(
        min,
        max,
        palette.backgroundTop,
        palette.backgroundTop,
        palette.backgroundBottom,
        palette.backgroundBottom
    );
    drawList->AddRectFilled(
        ImVec2(min.x, min.y),
        ImVec2(max.x, min.y + 92.0F),
        palette.backgroundOverlay
    );
}

void drawPanelBorder(const ui::ThemePalette& palette)
{
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    const ImVec2 min = ImGui::GetWindowPos();
    const ImVec2 max(min.x + ImGui::GetWindowWidth(), min.y + ImGui::GetWindowHeight());
    drawList->AddRect(min, max, palette.border, 8.0F, 0, 1.0F);
}

void drawBrand(const ui::ThemePalette& palette)
{
    const ImVec2 start = ImGui::GetCursorScreenPos();
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    drawList->AddRectFilled(start, ImVec2(start.x + 34.0F, start.y + 34.0F), palette.brandOuter, 9.0F);
    drawList->AddRectFilled(
        ImVec2(start.x + 8.0F, start.y + 8.0F),
        ImVec2(start.x + 26.0F, start.y + 26.0F),
        palette.brandInner,
        6.0F
    );

    ImGui::Dummy(ImVec2(42.0F, 34.0F));
    ImGui::SameLine();
    ImGui::BeginGroup();
    ImGui::SetWindowFontScale(1.02F);
    ImGui::TextUnformatted("DevTools");
    ImGui::SetWindowFontScale(1.0F);
    ImGui::TextDisabled("Developer workbench");
    ImGui::EndGroup();
}

void drawSectionHeader(std::string_view text)
{
    ImGui::Dummy(ImVec2(0.0F, 4.0F));
    ImGui::TextDisabled("%.*s", static_cast<int>(text.size()), text.data());
    ImGui::Dummy(ImVec2(0.0F, 1.0F));
}

bool drawNavigationRow(std::string_view id, std::string_view name, std::string_view description, bool selected, const ui::ThemePalette& palette)
{
    ImGui::PushID(id.data(), id.data() + id.size());
    const ImVec2 pos = ImGui::GetCursorScreenPos();
    const ImVec2 size(ImGui::GetContentRegionAvail().x, 46.0F);
    const bool pressed = ImGui::InvisibleButton("nav-row", size);
    const bool hovered = ImGui::IsItemHovered();

    ImDrawList* drawList = ImGui::GetWindowDrawList();
    const ImU32 fill = selected ? palette.rowSelected : hovered ? palette.rowHovered
                                                                : palette.row;
    drawList->AddRectFilled(pos, ImVec2(pos.x + size.x, pos.y + size.y), fill, 9.0F);
    if (selected) {
        drawList->AddRect(pos, ImVec2(pos.x + size.x, pos.y + size.y), palette.rowSelectedBorder, 9.0F);
    }

    if (selected) {
        drawList->AddRectFilled(pos, ImVec2(pos.x + 3.0F, pos.y + size.y), palette.accent, 9.0F);
    }

    const ImVec2 textPos(pos.x + 12.0F, pos.y + 7.0F);
    drawList->AddText(
        textPos,
        selected ? palette.rowSelectedText : palette.rowText,
        name.data(),
        name.data() + name.size()
    );
    drawList->AddText(
        ImVec2(textPos.x, textPos.y + 19.0F),
        palette.rowDescription,
        description.data(),
        description.data() + description.size()
    );
    ImGui::PopID();
    return pressed;
}

bool drawCompactNavigationRow(std::string_view id, std::string_view name, bool selected, const ui::ThemePalette& palette)
{
    ImGui::PushID(id.data(), id.data() + id.size());
    const ImVec2 pos = ImGui::GetCursorScreenPos();
    const ImVec2 size(ImGui::GetContentRegionAvail().x, 36.0F);
    const bool pressed = ImGui::InvisibleButton("nav-row", size);
    const bool hovered = ImGui::IsItemHovered();

    ImDrawList* drawList = ImGui::GetWindowDrawList();
    const ImU32 fill = selected ? palette.rowSelected : hovered ? palette.rowHovered
                                                                : palette.row;
    drawList->AddRectFilled(pos, ImVec2(pos.x + size.x, pos.y + size.y), fill, 9.0F);
    if (selected) {
        drawList->AddRect(pos, ImVec2(pos.x + size.x, pos.y + size.y), palette.rowSelectedBorder, 9.0F);
    }

    if (selected) {
        drawList->AddRectFilled(pos, ImVec2(pos.x + 3.0F, pos.y + size.y), palette.accent, 9.0F);
    }

    const ImVec2 textPos(pos.x + 12.0F, pos.y + 9.0F);
    drawList->AddText(
        textPos,
        selected ? palette.rowSelectedText : palette.rowText,
        name.data(),
        name.data() + name.size()
    );
    ImGui::PopID();
    return pressed;
}

bool drawToolRow(const core::Tool& tool, bool selected, const ui::ThemePalette& palette, bool showDescription)
{
    if (!showDescription) {
        return drawCompactNavigationRow(tool.id, tool.name, selected, palette);
    }
    return drawNavigationRow(tool.id, tool.name, tool.description, selected, palette);
}

void drawPill(std::string_view text, const ui::ThemePalette& palette)
{
    const ImVec2 pos = ImGui::GetCursorScreenPos();
    const ImVec2 textSize = ImGui::CalcTextSize(text.data(), text.data() + text.size());
    const ImVec2 size(textSize.x + 18.0F, 22.0F);
    ImGui::Dummy(size);
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    drawList->AddRectFilled(pos, ImVec2(pos.x + size.x, pos.y + size.y), palette.accentMuted, 11.0F);
    drawList->AddText(ImVec2(pos.x + 9.0F, pos.y + 3.0F), palette.pillText, text.data(), text.data() + text.size());
}

bool drawCommandPaletteRow(std::string_view id, std::string_view name, std::string_view description, bool selected)
{
    ImGui::PushID(id.data(), id.data() + id.size());
    const bool pressed = ImGui::Selectable(name.data(), selected, ImGuiSelectableFlags_None, ImVec2(0.0F, 42.0F));

    const ImVec2 itemMin = ImGui::GetItemRectMin();
    ImGui::GetWindowDrawList()->AddText(
        ImVec2(itemMin.x + 10.0F, itemMin.y + 24.0F),
        ImGui::GetColorU32(ImGuiCol_TextDisabled),
        description.data(),
        description.data() + description.size()
    );

    ImGui::PopID();
    return pressed;
}

} // namespace

namespace ui {

void UiShell::loadSettings(const core::ToolRegistry& registry)
{
    settings_ = ui::loadSettings();

    const auto tools = registry.tools();
    if (tools.empty()) {
        activeToolId_.clear();
        return;
    }

    auto hasTool = [&](const std::string& id) {
        return registry.findById(id) != nullptr;
    };

    if (settings_.startupBehavior == StartupBehavior::SpecificTool && hasTool(settings_.startupToolId)) {
        activeToolId_ = settings_.startupToolId;
    } else if (settings_.startupBehavior == StartupBehavior::LastTool && hasTool(settings_.lastActiveToolId)) {
        activeToolId_ = settings_.lastActiveToolId;
    } else {
        activeToolId_ = tools.front().id;
    }

    settings_.lastActiveToolId = activeToolId_;
    if (settings_.startupToolId.empty() || !hasTool(settings_.startupToolId)) {
        settings_.startupToolId = tools.front().id;
    }
}

void UiShell::saveSettings() const
{
    ui::saveSettings(settings_);
}

const UiSettings& UiShell::settings() const
{
    return settings_;
}

void UiShell::selectTool(std::string_view toolId)
{
    activeToolId_ = std::string(toolId);
    if (activeToolId_ != settingsToolId) {
        settings_.lastActiveToolId = activeToolId_;
    }
}

void UiShell::draw(const core::ToolRegistry& registry)
{
    const auto tools = registry.tools();
    if (activeToolId_.empty() && !tools.empty()) {
        selectTool(tools.front().id);
    }

    ImGuiIO& io = ImGui::GetIO();
    if ((io.KeyCtrl || io.KeySuper) && ImGui::IsKeyPressed(ImGuiKey_P)) {
        commandPaletteOpen_ = true;
        commandPaletteFocusSearch_ = true;
    }

    const ThemePalette& palette = themePalette(resolveThemeMode(settings_.themeMode));
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    drawViewportBackground(viewport, palette);

    const ImVec2 sidebarPos(viewport->WorkPos.x + outerPadding, viewport->WorkPos.y + outerPadding);
    const ImVec2 sidebarSize(sidebarWidth, std::max(320.0F, viewport->WorkSize.y - (outerPadding * 2.0F)));
    const ImVec2 workspacePos(sidebarPos.x + sidebarWidth + panelGap, sidebarPos.y);
    const ImVec2 workspaceSize(
        std::max(520.0F, viewport->WorkSize.x - sidebarWidth - panelGap - (outerPadding * 2.0F)),
        sidebarSize.y
    );

    constexpr ImGuiWindowFlags panelFlags =
        ImGuiWindowFlags_NoTitleBar
        | ImGuiWindowFlags_NoCollapse
        | ImGuiWindowFlags_NoResize
        | ImGuiWindowFlags_NoMove
        | ImGuiWindowFlags_NoSavedSettings;

    ImGui::SetNextWindowPos(sidebarPos, ImGuiCond_Always);
    ImGui::SetNextWindowSize(sidebarSize, ImGuiCond_Always);
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImGui::ColorConvertU32ToFloat4(palette.panel));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(13.0F, 13.0F));
    if (ImGui::Begin("DevTools Sidebar", nullptr, panelFlags)) {
        drawPanelBorder(palette);
        drawSidebar(registry);
    }
    ImGui::End();
    ImGui::PopStyleVar();
    ImGui::PopStyleColor();

    ImGui::SetNextWindowPos(workspacePos, ImGuiCond_Always);
    ImGui::SetNextWindowSize(workspaceSize, ImGuiCond_Always);
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImGui::ColorConvertU32ToFloat4(palette.panel));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(14.0F, 14.0F));
    if (ImGui::Begin("DevTools Workspace", nullptr, panelFlags)) {
        drawPanelBorder(palette);
        drawCurrentTool(registry);
    }
    ImGui::End();
    ImGui::PopStyleVar();
    ImGui::PopStyleColor();

    drawCommandPalette(registry);
    drawClipboardStatus();
}

void UiShell::drawSidebar(const core::ToolRegistry& registry)
{
    const ThemePalette& palette = themePalette(resolveThemeMode(settings_.themeMode));
    drawBrand(palette);
    ImGui::Dummy(ImVec2(0.0F, 12.0F));

    ImGui::SetNextItemWidth(-1.0F);
    ImGui::InputTextWithHint("##SidebarSearch", "Search tools", sidebarSearch_.data(), sidebarSearch_.size());
    ImGui::Dummy(ImVec2(0.0F, 6.0F));

    const std::string_view query(sidebarSearch_.data());
    const bool showDescriptions = settings_.showSidebarDescriptions;
    const float footerHeight = showDescriptions ? 96.0F : 88.0F;
    if (ImGui::BeginChild("Tool Navigation", ImVec2(0.0F, -footerHeight), false)) {
        int visibleCount = 0;
        for (const ToolCategory& category : toolCategories) {
            bool drewHeader = false;
            for (const core::Tool& tool : registry.tools()) {
                if (!categoryContains(category, tool.id) || !toolMatchesQuery(tool, query)) {
                    continue;
                }
                if (!drewHeader) {
                    drawSectionHeader(category.name);
                    drewHeader = true;
                }
                if (drawToolRow(tool, activeToolId_ == tool.id, palette, showDescriptions)) {
                    selectTool(tool.id);
                }
                ++visibleCount;
                ImGui::Dummy(ImVec2(0.0F, 3.0F));
            }
        }

        bool drewOtherHeader = false;
        for (const core::Tool& tool : registry.tools()) {
            bool knownCategory = false;
            for (const ToolCategory& category : toolCategories) {
                knownCategory = knownCategory || categoryContains(category, tool.id);
            }
            if (knownCategory || !toolMatchesQuery(tool, query)) {
                continue;
            }
            if (!drewOtherHeader) {
                drawSectionHeader("Other");
                drewOtherHeader = true;
            }
            if (drawToolRow(tool, activeToolId_ == tool.id, palette, showDescriptions)) {
                selectTool(tool.id);
            }
            ++visibleCount;
            ImGui::Dummy(ImVec2(0.0F, 3.0F));
        }

        if (visibleCount == 0) {
            ImGui::TextDisabled("No matching tools.");
        }
    }
    ImGui::EndChild();

    ImGui::Separator();
    ImGui::Dummy(ImVec2(0.0F, 4.0F));
    const bool settingsPressed = showDescriptions
        ? drawNavigationRow(settingsToolId, "Settings", "Theme, font, and size.", activeToolId_ == settingsToolId, palette)
        : drawCompactNavigationRow(settingsToolId, "Settings", activeToolId_ == settingsToolId, palette);
    if (settingsPressed) {
        selectTool(settingsToolId);
    }
    ImGui::Dummy(ImVec2(0.0F, 4.0F));
    ImGui::Separator();
    ImGui::Dummy(ImVec2(0.0F, 2.0F));
    drawPill("Cmd/Ctrl+P", palette);
    ImGui::SameLine();
    ImGui::TextDisabled("Open command palette");
}

void UiShell::drawCurrentTool(const core::ToolRegistry& registry)
{
    const ThemePalette& palette = themePalette(resolveThemeMode(settings_.themeMode));
    if (activeToolId_ == settingsToolId) {
        drawSettings(registry);
        return;
    }

    const core::Tool* activeTool = registry.findById(activeToolId_);
    if (activeTool == nullptr) {
        ImGui::TextDisabled("No tool selected.");
        return;
    }

    ImGui::SetWindowFontScale(1.13F);
    ImGui::TextUnformatted(activeTool->name.c_str());
    ImGui::SetWindowFontScale(1.0F);

    const ImGuiStyle& style = ImGui::GetStyle();
    const float settingsButtonWidth = ImGui::CalcTextSize("Settings").x + (style.FramePadding.x * 2.0F);
    const float settingsButtonX = ImGui::GetWindowContentRegionMax().x - settingsButtonWidth;
    ImGui::SameLine();
    ImGui::SetCursorPosX(std::max(ImGui::GetCursorPosX(), settingsButtonX));
    if (ImGui::Button("Settings")) {
        selectTool(settingsToolId);
    }

    drawPill(toolCategoryName(activeTool->id), palette);
    ImGui::SameLine();
    ImGui::PushTextWrapPos(ImGui::GetWindowContentRegionMax().x);
    ImGui::TextDisabled("%s", activeTool->description.c_str());
    ImGui::PopTextWrapPos();
    ImGui::Separator();
    ImGui::Dummy(ImVec2(0.0F, 4.0F));

    activeTool->draw();
}

void UiShell::drawSettings(const core::ToolRegistry& registry)
{
    const ThemePalette& palette = themePalette(resolveThemeMode(settings_.themeMode));

    drawPill("Settings", palette);
    ImGui::SameLine();
    ImGui::TextDisabled("Theme and typography.");
    ImGui::Dummy(ImVec2(0.0F, 3.0F));

    ImGui::SetWindowFontScale(1.14F);
    ImGui::TextUnformatted("Settings");
    ImGui::SetWindowFontScale(1.0F);
    ImGui::TextDisabled("Adjust the application appearance.");
    ImGui::Dummy(ImVec2(0.0F, 6.0F));

    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImGui::ColorConvertU32ToFloat4(palette.surface));
    ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 10.0F);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(13.0F, 13.0F));
    if (ImGui::BeginChild("Settings Surface", ImVec2(0.0F, 0.0F), true)) {
        ImGui::TextUnformatted("Appearance");
        ImGui::Separator();
        ImGui::Dummy(ImVec2(0.0F, 4.0F));

        int themeIndex = static_cast<int>(settings_.themeMode);
        constexpr const char* themeOptions[] = { "Follow system", "Light", "Dark" };
        ImGui::SetNextItemWidth(260.0F);
        if (ImGui::Combo("Theme", &themeIndex, themeOptions, IM_ARRAYSIZE(themeOptions))) {
            settings_.themeMode = static_cast<ThemeMode>(themeIndex);
        }

        ImGui::Dummy(ImVec2(0.0F, 10.0F));
        ImGui::TextUnformatted("Typography");
        ImGui::Separator();
        ImGui::Dummy(ImVec2(0.0F, 4.0F));

        int fontIndex = static_cast<int>(settings_.fontFamily);
        constexpr const char* fontOptions[] = { "System", "Monospace", "Serif", "Classic" };
        ImGui::SetNextItemWidth(260.0F);
        if (ImGui::Combo("Font", &fontIndex, fontOptions, IM_ARRAYSIZE(fontOptions))) {
            settings_.fontFamily = static_cast<FontFamily>(fontIndex);
        }

        ImGui::SetNextItemWidth(260.0F);
        ImGui::SliderFloat("Size", &settings_.fontSize, 12.0F, 22.0F, "%.0f px", ImGuiSliderFlags_AlwaysClamp);

        int densityIndex = static_cast<int>(settings_.density);
        constexpr const char* densityOptions[] = { "Comfortable", "Compact" };
        ImGui::SetNextItemWidth(260.0F);
        if (ImGui::Combo("Density", &densityIndex, densityOptions, IM_ARRAYSIZE(densityOptions))) {
            settings_.density = static_cast<UiDensity>(densityIndex);
        }

        ImGui::Checkbox("Show sidebar descriptions", &settings_.showSidebarDescriptions);

        ImGui::Dummy(ImVec2(0.0F, 10.0F));
        ImGui::TextUnformatted("Startup");
        ImGui::Separator();
        ImGui::Dummy(ImVec2(0.0F, 4.0F));

        int startupIndex = static_cast<int>(settings_.startupBehavior);
        constexpr const char* startupOptions[] = { "Last opened tool", "Specific tool", "First tool" };
        ImGui::SetNextItemWidth(260.0F);
        if (ImGui::Combo("Open on startup", &startupIndex, startupOptions, IM_ARRAYSIZE(startupOptions))) {
            settings_.startupBehavior = static_cast<StartupBehavior>(startupIndex);
        }

        if (settings_.startupBehavior == StartupBehavior::SpecificTool) {
            ImGui::SetNextItemWidth(260.0F);
            const core::Tool* startupTool = registry.findById(settings_.startupToolId);
            const char* currentToolLabel = startupTool == nullptr ? "Choose tool" : startupTool->name.c_str();
            if (ImGui::BeginCombo("Startup tool", currentToolLabel)) {
                for (const core::Tool& tool : registry.tools()) {
                    const bool selected = settings_.startupToolId == tool.id;
                    if (ImGui::Selectable(tool.name.c_str(), selected)) {
                        settings_.startupToolId = tool.id;
                    }
                    if (selected) {
                        ImGui::SetItemDefaultFocus();
                    }
                }
                ImGui::EndCombo();
            }
            ImGui::SameLine();
            if (!activeToolId_.empty() && activeToolId_ != settingsToolId && ImGui::Button("Set Current")) {
                settings_.startupToolId = activeToolId_;
            }
        }

        ImGui::Checkbox("Include Settings in command palette", &settings_.includeSettingsInCommandPalette);

        ImGui::Dummy(ImVec2(0.0F, 10.0F));
        ImGui::TextUnformatted("Clipboard");
        ImGui::Separator();
        ImGui::Dummy(ImVec2(0.0F, 4.0F));

        ImGui::Checkbox("Show copy confirmation", &settings_.showCopyConfirmation);
        ImGui::Checkbox("Auto-copy generated output", &settings_.autoCopyGeneratedOutput);

        ImGui::Dummy(ImVec2(0.0F, 10.0F));
        ImGui::TextUnformatted("Editor");
        ImGui::Separator();
        ImGui::Dummy(ImVec2(0.0F, 4.0F));

        ImGui::Checkbox("Soft wrap text", &settings_.softWrapText);
        ImGui::Checkbox("Use spaces for tabs", &settings_.useSpacesForTabs);
        ImGui::Checkbox("Confirm before clearing input", &settings_.confirmBeforeClearingInput);
        ImGui::SetNextItemWidth(260.0F);
        ImGui::SliderInt("Tab width", &settings_.tabWidth, 2, 8, "%d spaces", ImGuiSliderFlags_AlwaysClamp);

        ImGui::Dummy(ImVec2(0.0F, 10.0F));
        ImGui::TextUnformatted("Privacy");
        ImGui::Separator();
        ImGui::Dummy(ImVec2(0.0F, 4.0F));

        ImGui::Checkbox("Never store tool inputs", &settings_.neverStoreToolInputs);
        ImGui::Checkbox("Clear inputs on exit", &settings_.clearInputsOnExit);

        ImGui::Dummy(ImVec2(0.0F, 8.0F));
        ImGui::TextDisabled(
            "Current: %s theme, %s font, %.0f px, %s density",
            themeModeLabel(settings_.themeMode),
            fontFamilyLabel(settings_.fontFamily),
            settings_.fontSize,
            uiDensityLabel(settings_.density)
        );
    }
    ImGui::EndChild();
    ImGui::PopStyleVar(2);
    ImGui::PopStyleColor();
}

void UiShell::drawCommandPalette(const core::ToolRegistry& registry)
{
    if (!commandPaletteOpen_) {
        return;
    }

    if (!ImGui::IsPopupOpen("Command Palette", ImGuiPopupFlags_None)) {
        ImGui::OpenPopup("Command Palette");
    }

    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    const ImVec2 paletteSize(520.0F, 340.0F);
    ImGui::SetNextWindowSize(paletteSize, ImGuiCond_Appearing);
    ImGui::SetNextWindowPos(
        ImVec2(
            viewport->WorkPos.x + (viewport->WorkSize.x - paletteSize.x) * 0.5F,
            viewport->WorkPos.y + (viewport->WorkSize.y - paletteSize.y) * 0.25F
        ),
        ImGuiCond_Appearing
    );
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(14.0F, 14.0F));
    if (ImGui::BeginPopupModal("Command Palette", &commandPaletteOpen_, ImGuiWindowFlags_NoSavedSettings)) {
        const ThemePalette& palette = themePalette(resolveThemeMode(settings_.themeMode));
        drawPanelBorder(palette);
        ImGui::SetWindowFontScale(1.06F);
        ImGui::TextUnformatted("Command Palette");
        ImGui::SetWindowFontScale(1.0F);
        ImGui::TextDisabled("Jump to a tool by name or description.");
        ImGui::Dummy(ImVec2(0.0F, 5.0F));

        if (commandPaletteFocusSearch_) {
            ImGui::SetKeyboardFocusHere();
            commandPaletteFocusSearch_ = false;
        }
        ImGui::InputTextWithHint("##CommandSearch", "Search tools...", commandSearch_.data(), commandSearch_.size());
        ImGui::Dummy(ImVec2(0.0F, 5.0F));

        const std::string_view query(commandSearch_.data());
        int resultCount = 0;
        for (const core::Tool& tool : registry.tools()) {
            if (!containsCaseInsensitive(tool.name, query) && !containsCaseInsensitive(tool.description, query)) {
                continue;
            }

            ++resultCount;
            if (drawCommandPaletteRow(tool.id, tool.name, tool.description, activeToolId_ == tool.id)) {
                selectTool(tool.id);
                commandPaletteOpen_ = false;
                ImGui::CloseCurrentPopup();
            }
            ImGui::Dummy(ImVec2(0.0F, 4.0F));
        }

        if (settings_.includeSettingsInCommandPalette
            && (containsCaseInsensitive("Settings", query)
                || containsCaseInsensitive("Theme, font, startup, privacy, and editor preferences.", query))) {
            ++resultCount;
            if (drawCommandPaletteRow(
                    settingsToolId,
                    "Settings",
                    "Theme, font, startup, privacy, and editor preferences.",
                    activeToolId_ == settingsToolId
                )) {
                selectTool(settingsToolId);
                commandPaletteOpen_ = false;
                ImGui::CloseCurrentPopup();
            }
            ImGui::Dummy(ImVec2(0.0F, 4.0F));
        }

        if (resultCount == 0) {
            ImGui::TextDisabled("No matching tools.");
        }

        if (ImGui::IsKeyPressed(ImGuiKey_Escape)) {
            commandPaletteOpen_ = false;
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
    ImGui::PopStyleVar();
}

} // namespace ui
