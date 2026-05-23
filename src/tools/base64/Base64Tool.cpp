#include "tools/base64/Base64Tool.hpp"

#include "ui/Clipboard.hpp"

#include <imgui.h>

#include <array>
#include <string_view>
#include <vector>

namespace {

constexpr std::string_view alphabet = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

std::string encodeBase64(std::string_view input)
{
    std::string output;
    output.reserve(((input.size() + 2U) / 3U) * 4U);

    for (std::size_t index = 0; index < input.size(); index += 3U) {
        const unsigned int b0 = static_cast<unsigned char>(input[index]);
        const unsigned int b1 = index + 1U < input.size() ? static_cast<unsigned char>(input[index + 1U]) : 0U;
        const unsigned int b2 = index + 2U < input.size() ? static_cast<unsigned char>(input[index + 2U]) : 0U;

        output += alphabet[(b0 >> 2U) & 0x3FU];
        output += alphabet[((b0 << 4U) | (b1 >> 4U)) & 0x3FU];
        output += index + 1U < input.size() ? alphabet[((b1 << 2U) | (b2 >> 6U)) & 0x3FU] : '=';
        output += index + 2U < input.size() ? alphabet[b2 & 0x3FU] : '=';
    }

    return output;
}

bool decodeBase64(std::string_view input, std::string& output)
{
    std::array<int, 256> reverse {};
    reverse.fill(-1);
    for (int index = 0; index < static_cast<int>(alphabet.size()); ++index) {
        reverse[static_cast<unsigned char>(alphabet[static_cast<std::size_t>(index)])] = index;
    }

    output.clear();
    int value = 0;
    int bits = -8;

    for (const char ch : input) {
        if (ch == '=' || ch == '\n' || ch == '\r' || ch == '\t' || ch == ' ') {
            continue;
        }

        const int decoded = reverse[static_cast<unsigned char>(ch)];
        if (decoded < 0) {
            return false;
        }

        value = (value << 6) + decoded;
        bits += 6;

        if (bits >= 0) {
            output.push_back(static_cast<char>((value >> bits) & 0xFF));
            bits -= 8;
        }
    }

    return true;
}

} // namespace

namespace tools::base64 {

void Base64Tool::draw()
{
    ImGui::TextUnformatted("Input");
    ImGui::InputTextMultiline(
        "##Base64Input",
        input_.data(),
        input_.size(),
        ImVec2(-1.0F, ImGui::GetTextLineHeight() * 10.0F)
    );

    if (ImGui::Button("Encode")) {
        output_ = encodeBase64(input_.data());
        status_ = "Encoded";
    }

    ImGui::SameLine();
    if (ImGui::Button("Decode")) {
        if (decodeBase64(input_.data(), output_)) {
            status_ = "Decoded";
        } else {
            status_ = "Invalid Base64 input";
        }
    }

    ImGui::SameLine();
    if (ImGui::Button("Clear")) {
        input_.fill('\0');
        output_.clear();
        status_.clear();
    }

    if (!status_.empty()) {
        ImGui::SameLine();
        ImGui::TextDisabled("%s", status_.c_str());
    }

    ImGui::Separator();
    ImGui::TextUnformatted("Output");
    if (!output_.empty()) {
        ImGui::SameLine();
        if (ImGui::Button("Copy Output")) {
            ui::copyToClipboard(output_.c_str());
            status_ = "Copied output";
        }
    }

    std::vector<char> outputBuffer(output_.begin(), output_.end());
    outputBuffer.push_back('\0');
    ImGui::InputTextMultiline(
        "##Base64Output",
        outputBuffer.data(),
        outputBuffer.size(),
        ImVec2(-1.0F, ImGui::GetTextLineHeight() * 12.0F),
        ImGuiInputTextFlags_ReadOnly
    );
}

} // namespace tools::base64
