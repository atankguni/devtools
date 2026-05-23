#include "ui/UiShell.hpp"

#include <imgui.h>

#include <algorithm>
#include <cctype>
#include <string_view>

namespace {

constexpr float outerPadding = 12.0F;
constexpr float panelGap = 10.0F;
constexpr float sidebarWidth = 244.0F;
constexpr ImU32 backgroundTop = IM_COL32(10, 15, 22, 255);
constexpr ImU32 backgroundBottom = IM_COL32(14, 24, 34, 255);
constexpr ImU32 panelColor = IM_COL32(18, 26, 36, 238);
constexpr ImU32 surfaceColor = IM_COL32(14, 22, 32, 255);
constexpr ImU32 borderColor = IM_COL32(56, 76, 96, 150);
constexpr ImU32 accentColor = IM_COL32(67, 176, 255, 255);
constexpr ImU32 accentMutedColor = IM_COL32(67, 176, 255, 42);

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

void drawViewportBackground(ImGuiViewport* viewport)
{
    ImDrawList* drawList = ImGui::GetBackgroundDrawList(viewport);
    const ImVec2 min = viewport->WorkPos;
    const ImVec2 max(viewport->WorkPos.x + viewport->WorkSize.x, viewport->WorkPos.y + viewport->WorkSize.y);
    drawList->AddRectFilledMultiColor(min, max, backgroundTop, backgroundTop, backgroundBottom, backgroundBottom);
    drawList->AddRectFilled(
        ImVec2(min.x, min.y),
        ImVec2(max.x, min.y + 92.0F),
        IM_COL32(20, 34, 48, 62)
    );
}

void drawPanelBorder()
{
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    const ImVec2 min = ImGui::GetWindowPos();
    const ImVec2 max(min.x + ImGui::GetWindowWidth(), min.y + ImGui::GetWindowHeight());
    drawList->AddRect(min, max, borderColor, 10.0F, 0, 1.0F);
}

void drawBrand()
{
    const ImVec2 start = ImGui::GetCursorScreenPos();
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    drawList->AddRectFilled(start, ImVec2(start.x + 34.0F, start.y + 34.0F), IM_COL32(31, 120, 190, 255), 9.0F);
    drawList->AddRectFilled(
        ImVec2(start.x + 8.0F, start.y + 8.0F),
        ImVec2(start.x + 26.0F, start.y + 26.0F),
        IM_COL32(108, 210, 255, 255),
        6.0F
    );

    ImGui::Dummy(ImVec2(42.0F, 34.0F));
    ImGui::SameLine();
    ImGui::BeginGroup();
    ImGui::SetWindowFontScale(1.04F);
    ImGui::TextUnformatted("DevTools");
    ImGui::SetWindowFontScale(1.0F);
    ImGui::TextDisabled("Native utility suite");
    ImGui::EndGroup();
}

bool drawToolRow(const core::Tool& tool, bool selected)
{
    ImGui::PushID(tool.id.c_str());
    const ImVec2 pos = ImGui::GetCursorScreenPos();
    const ImVec2 size(ImGui::GetContentRegionAvail().x, 48.0F);
    const bool pressed = ImGui::InvisibleButton("tool-row", size);
    const bool hovered = ImGui::IsItemHovered();

    ImDrawList* drawList = ImGui::GetWindowDrawList();
    const ImU32 fill = selected ? IM_COL32(22, 58, 82, 230)
        : hovered ? IM_COL32(26, 38, 52, 230)
                  : IM_COL32(16, 24, 34, 110);
    drawList->AddRectFilled(pos, ImVec2(pos.x + size.x, pos.y + size.y), fill, 9.0F);
    drawList->AddRect(pos, ImVec2(pos.x + size.x, pos.y + size.y), selected ? IM_COL32(70, 176, 255, 150) : borderColor, 9.0F);

    if (selected) {
        drawList->AddRectFilled(pos, ImVec2(pos.x + 3.0F, pos.y + size.y), accentColor, 9.0F);
    }

    const ImVec2 textPos(pos.x + 12.0F, pos.y + 7.0F);
    drawList->AddText(textPos, selected ? IM_COL32(236, 248, 255, 255) : IM_COL32(220, 228, 236, 255), tool.name.c_str());
    drawList->AddText(ImVec2(textPos.x, textPos.y + 20.0F), IM_COL32(137, 151, 166, 255), tool.description.c_str());
    ImGui::PopID();
    return pressed;
}

void drawPill(std::string_view text)
{
    const ImVec2 pos = ImGui::GetCursorScreenPos();
    const ImVec2 textSize = ImGui::CalcTextSize(text.data(), text.data() + text.size());
    const ImVec2 size(textSize.x + 18.0F, 22.0F);
    ImGui::Dummy(size);
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    drawList->AddRectFilled(pos, ImVec2(pos.x + size.x, pos.y + size.y), accentMutedColor, 11.0F);
    drawList->AddText(ImVec2(pos.x + 9.0F, pos.y + 3.0F), IM_COL32(139, 217, 255, 255), text.data(), text.data() + text.size());
}

} // namespace

namespace ui {

void UiShell::draw(const core::ToolRegistry& registry)
{
    const auto tools = registry.tools();
    if (activeToolId_.empty() && !tools.empty()) {
        activeToolId_ = tools.front().id;
    }

    ImGuiIO& io = ImGui::GetIO();
    if ((io.KeyCtrl || io.KeySuper) && ImGui::IsKeyPressed(ImGuiKey_P)) {
        commandPaletteOpen_ = true;
        ImGui::OpenPopup("Command Palette");
    }

    ImGuiViewport* viewport = ImGui::GetMainViewport();
    drawViewportBackground(viewport);

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
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImGui::ColorConvertU32ToFloat4(panelColor));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(13.0F, 13.0F));
    if (ImGui::Begin("DevTools Sidebar", nullptr, panelFlags)) {
        drawPanelBorder();
        drawSidebar(registry);
    }
    ImGui::End();
    ImGui::PopStyleVar();
    ImGui::PopStyleColor();

    ImGui::SetNextWindowPos(workspacePos, ImGuiCond_Always);
    ImGui::SetNextWindowSize(workspaceSize, ImGuiCond_Always);
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImGui::ColorConvertU32ToFloat4(panelColor));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(16.0F, 16.0F));
    if (ImGui::Begin("DevTools Workspace", nullptr, panelFlags)) {
        drawPanelBorder();
        drawCurrentTool(registry);
    }
    ImGui::End();
    ImGui::PopStyleVar();
    ImGui::PopStyleColor();

    drawCommandPalette(registry);
}

void UiShell::drawSidebar(const core::ToolRegistry& registry)
{
    drawBrand();
    ImGui::Dummy(ImVec2(0.0F, 10.0F));

    ImGui::TextDisabled("TOOLS");
    ImGui::Separator();
    ImGui::Spacing();

    for (const core::Tool& tool : registry.tools()) {
        if (drawToolRow(tool, activeToolId_ == tool.id)) {
            activeToolId_ = tool.id;
        }
        ImGui::Dummy(ImVec2(0.0F, 4.0F));
    }

    const float footerY = ImGui::GetWindowHeight() - 56.0F;
    if (footerY > ImGui::GetCursorPosY()) {
        ImGui::SetCursorPosY(footerY);
    }
    ImGui::Separator();
    ImGui::Dummy(ImVec2(0.0F, 2.0F));
    drawPill("Cmd/Ctrl+P");
    ImGui::SameLine();
    ImGui::TextDisabled("Open command palette");
}

void UiShell::drawCurrentTool(const core::ToolRegistry& registry)
{
    const core::Tool* activeTool = registry.findById(activeToolId_);
    if (activeTool == nullptr) {
        ImGui::TextDisabled("No tool selected.");
        return;
    }

    drawPill("Active tool");
    ImGui::SameLine();
    ImGui::TextDisabled("%s", activeTool->description.c_str());
    ImGui::Dummy(ImVec2(0.0F, 3.0F));

    ImGui::SetWindowFontScale(1.14F);
    ImGui::TextUnformatted(activeTool->name.c_str());
    ImGui::SetWindowFontScale(1.0F);
    ImGui::TextDisabled("%s", activeTool->description.c_str());
    ImGui::Dummy(ImVec2(0.0F, 6.0F));

    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImGui::ColorConvertU32ToFloat4(surfaceColor));
    ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 10.0F);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(13.0F, 13.0F));
    if (ImGui::BeginChild("Tool Surface", ImVec2(0.0F, 0.0F), true)) {
        activeTool->draw();
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
        drawPanelBorder();
        ImGui::SetWindowFontScale(1.06F);
        ImGui::TextUnformatted("Command Palette");
        ImGui::SetWindowFontScale(1.0F);
        ImGui::TextDisabled("Jump to a tool by name or description.");
        ImGui::Dummy(ImVec2(0.0F, 5.0F));

        ImGui::SetKeyboardFocusHere();
        ImGui::InputTextWithHint("##CommandSearch", "Search tools...", commandSearch_.data(), commandSearch_.size());
        ImGui::Dummy(ImVec2(0.0F, 5.0F));

        const std::string_view query(commandSearch_.data());
        int resultCount = 0;
        for (const core::Tool& tool : registry.tools()) {
            if (!containsCaseInsensitive(tool.name, query) && !containsCaseInsensitive(tool.description, query)) {
                continue;
            }

            ++resultCount;
            if (drawToolRow(tool, activeToolId_ == tool.id)) {
                activeToolId_ = tool.id;
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
