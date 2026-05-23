#include "tools/uuid/UuidTool.hpp"

#include <imgui.h>

#include <array>
#include <iomanip>
#include <random>
#include <sstream>

namespace {

std::string makeUuidV4()
{
    static thread_local std::mt19937_64 engine(std::random_device {}());
    std::array<unsigned char, 16> bytes {};

    for (std::size_t index = 0; index < bytes.size(); index += 8U) {
        const auto value = engine();
        for (std::size_t offset = 0; offset < 8U && index + offset < bytes.size(); ++offset) {
            bytes[index + offset] = static_cast<unsigned char>((value >> (offset * 8U)) & 0xFFU);
        }
    }

    bytes[6] = static_cast<unsigned char>((bytes[6] & 0x0FU) | 0x40U);
    bytes[8] = static_cast<unsigned char>((bytes[8] & 0x3FU) | 0x80U);

    std::ostringstream stream;
    stream << std::hex << std::setfill('0');
    for (std::size_t index = 0; index < bytes.size(); ++index) {
        stream << std::setw(2) << static_cast<int>(bytes[index]);
        if (index == 3 || index == 5 || index == 7 || index == 9) {
            stream << '-';
        }
    }

    return stream.str();
}

} // namespace

namespace tools::uuid {

void UuidTool::draw()
{
    if (generated_.empty()) {
        generate();
    }

    if (ImGui::Button("Generate UUID v4")) {
        generate();
    }

    ImGui::SameLine();
    if (!generated_.empty() && ImGui::Button("Copy Latest")) {
        ImGui::SetClipboardText(generated_.front().c_str());
    }

    ImGui::SameLine();
    if (ImGui::Button("Clear History")) {
        generated_.clear();
    }

    ImGui::Separator();

    for (const std::string& value : generated_) {
        ImGui::PushID(value.c_str());
        if (ImGui::Button("Copy")) {
            ImGui::SetClipboardText(value.c_str());
        }
        ImGui::SameLine();
        ImGui::TextUnformatted(value.c_str());
        ImGui::PopID();
    }
}

void UuidTool::generate()
{
    generated_.insert(generated_.begin(), makeUuidV4());
    if (generated_.size() > 20U) {
        generated_.pop_back();
    }
}

} // namespace tools::uuid
