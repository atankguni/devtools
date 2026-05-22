#include "ui/UiShell.hpp"

#include <imgui.h>

#include <algorithm>
#include <cctype>
#include <string_view>

namespace {

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
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);
    ImGui::SetNextWindowViewport(viewport->ID);

    constexpr ImGuiWindowFlags windowFlags =
        ImGuiWindowFlags_NoTitleBar
        | ImGuiWindowFlags_NoCollapse
        | ImGuiWindowFlags_NoResize
        | ImGuiWindowFlags_NoMove
        | ImGuiWindowFlags_NoBackground
        | ImGuiWindowFlags_NoBringToFrontOnFocus
        | ImGuiWindowFlags_NoNavFocus;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0F);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0F);
    ImGui::Begin("Forge", nullptr, windowFlags);
    ImGui::PopStyleVar(2);

    const ImGuiID dockspaceId = ImGui::GetID("ForgeDockspace");
    ImGui::DockSpace(dockspaceId, ImVec2(0.0F, 0.0F), ImGuiDockNodeFlags_PassthruCentralNode);

    ImGui::End();

    ImGui::SetNextWindowPos(ImVec2(viewport->WorkPos.x + 12.0F, viewport->WorkPos.y + 12.0F), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(240.0F, std::max(320.0F, viewport->WorkSize.y - 24.0F)), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("Tools")) {
        drawSidebar(registry);
    }
    ImGui::End();

    ImGui::SetNextWindowPos(ImVec2(viewport->WorkPos.x + 264.0F, viewport->WorkPos.y + 12.0F), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(
        ImVec2(std::max(520.0F, viewport->WorkSize.x - 276.0F), std::max(320.0F, viewport->WorkSize.y - 24.0F)),
        ImGuiCond_FirstUseEver
    );
    if (ImGui::Begin("Workspace")) {
        drawCurrentTool(registry);
    }
    ImGui::End();

    drawCommandPalette(registry);
}

void UiShell::drawSidebar(const core::ToolRegistry& registry)
{
    ImGui::TextUnformatted("Tools");
    ImGui::Separator();

    for (const core::Tool& tool : registry.tools()) {
        const bool selected = activeToolId_ == tool.id;
        if (ImGui::Selectable(tool.name.c_str(), selected)) {
            activeToolId_ = tool.id;
        }

        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("%s", tool.description.c_str());
        }
    }

    ImGui::SetCursorPosY(ImGui::GetWindowHeight() - 34.0F);
    ImGui::Separator();
    ImGui::TextDisabled("Ctrl/Cmd+P: commands");
}

void UiShell::drawCurrentTool(const core::ToolRegistry& registry)
{
    const core::Tool* activeTool = registry.findById(activeToolId_);
    if (activeTool == nullptr) {
        ImGui::TextDisabled("No tool selected.");
        return;
    }

    ImGui::TextUnformatted(activeTool->name.c_str());
    ImGui::TextDisabled("%s", activeTool->description.c_str());
    ImGui::Separator();

    activeTool->draw();
}

void UiShell::drawCommandPalette(const core::ToolRegistry& registry)
{
    if (!commandPaletteOpen_) {
        return;
    }

    ImGui::SetNextWindowSize(ImVec2(520.0F, 360.0F), ImGuiCond_Appearing);
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
