#include "ui/UiShell.hpp"

#include <imgui.h>

#include <algorithm>
#include <cctype>
#include <string_view>

namespace {

constexpr float outerPadding = 14.0F;
constexpr float panelGap = 12.0F;
constexpr float sidebarWidth = 248.0F;

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

    const ImGuiViewport* viewport = ImGui::GetMainViewport();
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
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(14.0F, 16.0F));
    if (ImGui::Begin("DevTools Sidebar", nullptr, panelFlags)) {
        drawSidebar(registry);
    }
    ImGui::End();
    ImGui::PopStyleVar();

    ImGui::SetNextWindowPos(workspacePos, ImGuiCond_Always);
    ImGui::SetNextWindowSize(workspaceSize, ImGuiCond_Always);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(18.0F, 18.0F));
    if (ImGui::Begin("DevTools Workspace", nullptr, panelFlags)) {
        drawCurrentTool(registry);
    }
    ImGui::End();
    ImGui::PopStyleVar();

    drawCommandPalette(registry);
}

void UiShell::drawSidebar(const core::ToolRegistry& registry)
{
    ImGui::TextColored(ImVec4(0.72F, 0.82F, 0.96F, 1.0F), "DevTools");
    ImGui::TextDisabled("Developer tools");
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    for (const core::Tool& tool : registry.tools()) {
        const bool selected = activeToolId_ == tool.id;
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(8.0F, 5.0F));
        if (ImGui::Selectable(tool.name.c_str(), selected, 0, ImVec2(0.0F, 28.0F))) {
            activeToolId_ = tool.id;
        }
        ImGui::PopStyleVar();

        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("%s", tool.description.c_str());
        }
    }

    const float footerY = ImGui::GetWindowHeight() - 48.0F;
    if (footerY > ImGui::GetCursorPosY()) {
        ImGui::SetCursorPosY(footerY);
    }
    ImGui::Separator();
    ImGui::TextDisabled("Cmd/Ctrl+P");
    ImGui::SameLine();
    ImGui::TextDisabled("Command palette");
}

void UiShell::drawCurrentTool(const core::ToolRegistry& registry)
{
    const core::Tool* activeTool = registry.findById(activeToolId_);
    if (activeTool == nullptr) {
        ImGui::TextDisabled("No tool selected.");
        return;
    }

    ImGui::TextColored(ImVec4(0.78F, 0.87F, 1.0F, 1.0F), "%s", activeTool->name.c_str());
    ImGui::TextDisabled("%s", activeTool->description.c_str());
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    activeTool->draw();
}

void UiShell::drawCommandPalette(const core::ToolRegistry& registry)
{
    if (!commandPaletteOpen_) {
        return;
    }

    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    const ImVec2 paletteSize(560.0F, 380.0F);
    ImGui::SetNextWindowSize(paletteSize, ImGuiCond_Appearing);
    ImGui::SetNextWindowPos(
        ImVec2(
            viewport->WorkPos.x + (viewport->WorkSize.x - paletteSize.x) * 0.5F,
            viewport->WorkPos.y + (viewport->WorkSize.y - paletteSize.y) * 0.25F
        ),
        ImGuiCond_Appearing
    );
    if (ImGui::BeginPopupModal("Command Palette", &commandPaletteOpen_, ImGuiWindowFlags_NoSavedSettings)) {
        ImGui::SetKeyboardFocusHere();
        ImGui::InputTextWithHint("##CommandSearch", "Search tools...", commandSearch_.data(), commandSearch_.size());
        ImGui::Separator();

        const std::string_view query(commandSearch_.data());
        for (const core::Tool& tool : registry.tools()) {
            if (!containsCaseInsensitive(tool.name, query) && !containsCaseInsensitive(tool.description, query)) {
                continue;
            }

            if (ImGui::Selectable(tool.name.c_str())) {
                activeToolId_ = tool.id;
                commandPaletteOpen_ = false;
                ImGui::CloseCurrentPopup();
            }
            ImGui::TextDisabled("%s", tool.description.c_str());
        }

        if (ImGui::IsKeyPressed(ImGuiKey_Escape)) {
            commandPaletteOpen_ = false;
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
}

} // namespace ui
