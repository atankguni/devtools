#include "ui/Clipboard.hpp"

#include <imgui.h>

namespace ui {

namespace {

bool showCopyConfirmation = true;
float confirmationUntil = 0.0F;

} // namespace

void applyClipboardSettings(const UiSettings& settings)
{
    showCopyConfirmation = settings.showCopyConfirmation;
}

void copyToClipboard(const char* text)
{
    ImGui::SetClipboardText(text);
    if (showCopyConfirmation) {
        confirmationUntil = static_cast<float>(ImGui::GetTime()) + 1.6F;
    }
}

void drawClipboardStatus()
{
    if (!showCopyConfirmation || confirmationUntil <= ImGui::GetTime()) {
        return;
    }

    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    constexpr ImVec2 size(142.0F, 34.0F);
    ImGui::SetNextWindowPos(
        ImVec2(
            viewport->WorkPos.x + viewport->WorkSize.x - size.x - 24.0F,
            viewport->WorkPos.y + viewport->WorkSize.y - size.y - 24.0F
        ),
        ImGuiCond_Always
    );
    ImGui::SetNextWindowSize(size, ImGuiCond_Always);
    ImGui::SetNextWindowBgAlpha(0.92F);
    constexpr ImGuiWindowFlags flags =
        ImGuiWindowFlags_NoDecoration
        | ImGuiWindowFlags_NoMove
        | ImGuiWindowFlags_NoSavedSettings
        | ImGuiWindowFlags_NoInputs;

    if (ImGui::Begin("Clipboard Status", nullptr, flags)) {
        ImGui::TextUnformatted("Copied");
    }
    ImGui::End();
}

} // namespace ui
