#include "tools/base64/Base64Tool.hpp"

#include "ui/Clipboard.hpp"
#include "ui/Workbench.hpp"

#include <imgui.h>

#include <array>
#include <string>
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
    auto encode = [&] {
        output_ = encodeBase64(input_.data());
        status_ = "Encoded";
        statusIsError_ = false;
    };

    auto decode = [&] {
        if (decodeBase64(input_.data(), output_)) {
            status_ = "Decoded";
            statusIsError_ = false;
        } else {
            status_ = "Invalid Base64 input";
            statusIsError_ = true;
        }
    };

    if (ui::workbench::primaryButton("Encode")) {
        encode();
    }

    ImGui::SameLine();
    if (ui::workbench::quietButton("Decode")) {
        decode();
    }

    ImGui::SameLine();
    if (ui::workbench::quietButton("Clear")) {
        input_.fill('\0');
        output_.clear();
        status_.clear();
        statusIsError_ = false;
    }

    ImGui::SameLine();
    if (ui::workbench::quietButton("Copy") && !output_.empty()) {
        ui::copyToClipboard(output_.c_str());
        status_ = "Copied output";
        statusIsError_ = false;
    }

    if (!status_.empty()) {
        ImGui::SameLine();
        ui::workbench::drawStatus(
            status_,
            statusIsError_ ? ui::workbench::StatusTone::Error : ui::workbench::StatusTone::Success
        );
    }

    ImGui::Dummy(ImVec2(0.0F, 6.0F));

    const ImVec2 available = ImGui::GetContentRegionAvail();
    const float gap = 10.0F;
    const bool split = available.x >= 820.0F;
    const ImVec2 paneSize(
        split ? (available.x - gap) * 0.5F : available.x,
        split ? available.y : (available.y - gap) * 0.5F
    );

    if (ui::workbench::beginPanel("Base64InputPane", "Input", "Text or Base64 data", paneSize)) {
        ImGui::InputTextMultiline(
            "##Base64Input",
            input_.data(),
            input_.size(),
            ImVec2(-1.0F, -1.0F),
            ImGuiInputTextFlags_AllowTabInput
        );
    }
    ui::workbench::endPanel();

    if (split) {
        ImGui::SameLine(0.0F, gap);
    } else {
        ImGui::Dummy(ImVec2(0.0F, gap));
    }

    std::vector<char> outputBuffer(output_.begin(), output_.end());
    outputBuffer.push_back('\0');
    const std::string outputDetail = output_.empty() ? "Result appears here" : std::to_string(output_.size()) + " bytes";
    if (ui::workbench::beginPanel("Base64OutputPane", "Output", outputDetail, ImVec2(0.0F, 0.0F))) {
        ImGui::InputTextMultiline(
            "##Base64Output",
            outputBuffer.data(),
            outputBuffer.size(),
            ImVec2(-1.0F, -1.0F),
            ImGuiInputTextFlags_ReadOnly
        );
    }
    ui::workbench::endPanel();
}

} // namespace tools::base64
