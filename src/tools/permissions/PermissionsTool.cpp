#include "tools/permissions/PermissionsTool.hpp"

#include <imgui.h>

#include <array>
#include <charconv>

namespace {

std::string symbolicFromBits(const bool bits[9])
{
    std::string value;
    value.reserve(9U);
    constexpr std::array<char, 9> chars { 'r', 'w', 'x', 'r', 'w', 'x', 'r', 'w', 'x' };
    for (std::size_t index = 0; index < chars.size(); ++index) {
        value += bits[index] ? chars[index] : '-';
    }
    return value;
}

std::string octalFromBits(const bool bits[9])
{
    std::string value;
    for (int group = 0; group < 3; ++group) {
        int digit = 0;
        digit += bits[group * 3] ? 4 : 0;
        digit += bits[group * 3 + 1] ? 2 : 0;
        digit += bits[group * 3 + 2] ? 1 : 0;
        value += static_cast<char>('0' + digit);
    }
    return value;
}

} // namespace

namespace tools::permissions {

void PermissionsTool::draw()
{
    ImGui::TextUnformatted("Octal");
    if (ImGui::InputText("##OctalPermission", octalInput_.data(), octalInput_.size())) {
        bool valid = true;
        for (int index = 0; index < 3; ++index) {
            const char ch = octalInput_[index];
            if (ch < '0' || ch > '7') {
                valid = false;
                break;
            }
            const int digit = ch - '0';
            bits_[index * 3] = (digit & 4) != 0;
            bits_[index * 3 + 1] = (digit & 2) != 0;
            bits_[index * 3 + 2] = (digit & 1) != 0;
        }
        symbolic_ = symbolicFromBits(bits_);
        status_ = valid ? "Updated from octal" : "Use three octal digits";
    }

    ImGui::Separator();
    constexpr std::array<const char*, 3> labels { "Owner", "Group", "Other" };
    for (int group = 0; group < 3; ++group) {
        ImGui::TextUnformatted(labels[group]);
        ImGui::SameLine(90.0F);
        ImGui::PushID(group);
        bool changed = false;
        changed |= ImGui::Checkbox("Read", &bits_[group * 3]);
        ImGui::SameLine();
        changed |= ImGui::Checkbox("Write", &bits_[group * 3 + 1]);
        ImGui::SameLine();
        changed |= ImGui::Checkbox("Execute", &bits_[group * 3 + 2]);
        if (changed) {
            const std::string octal = octalFromBits(bits_);
            octalInput_.fill('\0');
            octal.copy(octalInput_.data(), octal.size());
            symbolic_ = symbolicFromBits(bits_);
            status_ = "Updated from checkboxes";
        }
        ImGui::PopID();
    }

    ImGui::Separator();
    const std::string octal = octalInput_.data();
    ImGui::Text("Octal: %s", octal.c_str());
    ImGui::Text("Symbolic: %s", symbolic_.c_str());
    if (ImGui::Button("Copy Octal")) {
        ImGui::SetClipboardText(octal.c_str());
        status_ = "Copied octal";
    }
    ImGui::SameLine();
    if (ImGui::Button("Copy Symbolic")) {
        ImGui::SetClipboardText(symbolic_.c_str());
        status_ = "Copied symbolic";
    }
    if (!status_.empty()) {
        ImGui::SameLine();
        ImGui::TextDisabled("%s", status_.c_str());
    }
}

} // namespace tools::permissions
