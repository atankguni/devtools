#include "tools/jwt/JwtTool.hpp"

#include "ui/Clipboard.hpp"

#include <imgui.h>

#include <algorithm>
#include <array>
#include <span>
#include <string_view>
#include <vector>

namespace {

constexpr std::string_view base64UrlAlphabet = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";

void copyToBuffer(std::string_view value, std::span<char> buffer)
{
    std::ranges::fill(buffer, '\0');
    const std::size_t count = std::min(value.size(), buffer.size() - 1U);
    std::copy_n(value.data(), count, buffer.data());
}

std::string base64UrlEncode(std::string_view input)
{
    std::string output;
    output.reserve(((input.size() + 2U) / 3U) * 4U);

    for (std::size_t index = 0; index < input.size(); index += 3U) {
        const unsigned int b0 = static_cast<unsigned char>(input[index]);
        const unsigned int b1 = index + 1U < input.size() ? static_cast<unsigned char>(input[index + 1U]) : 0U;
        const unsigned int b2 = index + 2U < input.size() ? static_cast<unsigned char>(input[index + 2U]) : 0U;

        output += base64UrlAlphabet[(b0 >> 2U) & 0x3FU];
        output += base64UrlAlphabet[((b0 << 4U) | (b1 >> 4U)) & 0x3FU];
        if (index + 1U < input.size()) {
            output += base64UrlAlphabet[((b1 << 2U) | (b2 >> 6U)) & 0x3FU];
        }
        if (index + 2U < input.size()) {
            output += base64UrlAlphabet[b2 & 0x3FU];
        }
    }

    return output;
}

bool base64UrlDecode(std::string_view input, std::string& output)
{
    std::array<int, 256> reverse {};
    reverse.fill(-1);
    for (int index = 0; index < static_cast<int>(base64UrlAlphabet.size()); ++index) {
        reverse[static_cast<unsigned char>(base64UrlAlphabet[static_cast<std::size_t>(index)])] = index;
    }

    output.clear();
    int value = 0;
    int bits = -8;

    for (const char ch : input) {
        if (ch == '=') {
            break;
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

std::vector<std::string_view> splitJwt(std::string_view token)
{
    std::vector<std::string_view> parts;
    std::size_t start = 0;
    while (start <= token.size()) {
        const std::size_t separator = token.find('.', start);
        if (separator == std::string_view::npos) {
            parts.push_back(token.substr(start));
            break;
        }
        parts.push_back(token.substr(start, separator - start));
        start = separator + 1U;
    }
    return parts;
}

} // namespace

namespace tools::jwt {

JwtTool::JwtTool()
{
    copyToBuffer(R"({"alg":"none","typ":"JWT"})", headerInput_);
    copyToBuffer(R"({"sub":"1234567890","name":"DevTools","iat":1710000000})", payloadInput_);
}

void JwtTool::draw()
{
    int selectedMode = static_cast<int>(mode_);
    if (ImGui::RadioButton("Decode", selectedMode == static_cast<int>(Mode::Decode))) {
        mode_ = Mode::Decode;
    }
    ImGui::SameLine();
    if (ImGui::RadioButton("Encode", selectedMode == static_cast<int>(Mode::Encode))) {
        mode_ = Mode::Encode;
    }

    ImGui::TextDisabled("JWT signatures are not generated or verified.");
    ImGui::Separator();

    if (mode_ == Mode::Decode) {
        ImGui::TextUnformatted("Token");
        ImGui::InputTextMultiline(
            "##JwtTokenInput",
            tokenInput_.data(),
            tokenInput_.size(),
            ImVec2(-1.0F, ImGui::GetTextLineHeight() * 6.0F)
        );

        if (ImGui::Button("Decode Token")) {
            decodedHeader_.clear();
            decodedPayload_.clear();
            encodedToken_.clear();

            const std::string_view token(tokenInput_.data());
            const std::vector<std::string_view> parts = splitJwt(token);
            if (parts.size() < 2U || parts.size() > 3U || parts[0].empty() || parts[1].empty()) {
                status_ = "Invalid JWT structure";
            } else if (!base64UrlDecode(parts[0], decodedHeader_)) {
                status_ = "Invalid JWT header encoding";
            } else if (!base64UrlDecode(parts[1], decodedPayload_)) {
                status_ = "Invalid JWT payload encoding";
            } else {
                status_ = parts.size() == 3U && !parts[2].empty() ? "Decoded signed token" : "Decoded unsigned token";
            }
        }

        ImGui::SameLine();
        if (ImGui::Button("Clear")) {
            tokenInput_.fill('\0');
            decodedHeader_.clear();
            decodedPayload_.clear();
            encodedToken_.clear();
            status_.clear();
        }

        if (!status_.empty()) {
            ImGui::SameLine();
            ImGui::TextDisabled("%s", status_.c_str());
        }

        if (!decodedHeader_.empty() || !decodedPayload_.empty()) {
            ImGui::Separator();
            ImGui::TextUnformatted("Header");
            if (!decodedHeader_.empty()) {
                ImGui::SameLine();
                if (ImGui::Button("Copy Header")) {
                    ui::copyToClipboard(decodedHeader_.c_str());
                    status_ = "Copied header";
                }
            }
            std::vector<char> headerBuffer(decodedHeader_.begin(), decodedHeader_.end());
            headerBuffer.push_back('\0');
            ImGui::InputTextMultiline(
                "##JwtDecodedHeader",
                headerBuffer.data(),
                headerBuffer.size(),
                ImVec2(-1.0F, ImGui::GetTextLineHeight() * 5.0F),
                ImGuiInputTextFlags_ReadOnly
            );

            ImGui::TextUnformatted("Payload");
            if (!decodedPayload_.empty()) {
                ImGui::SameLine();
                if (ImGui::Button("Copy Payload")) {
                    ui::copyToClipboard(decodedPayload_.c_str());
                    status_ = "Copied payload";
                }
            }
            std::vector<char> payloadBuffer(decodedPayload_.begin(), decodedPayload_.end());
            payloadBuffer.push_back('\0');
            ImGui::InputTextMultiline(
                "##JwtDecodedPayload",
                payloadBuffer.data(),
                payloadBuffer.size(),
                ImVec2(-1.0F, ImGui::GetTextLineHeight() * 8.0F),
                ImGuiInputTextFlags_ReadOnly
            );
        }

        return;
    }

    ImGui::TextUnformatted("Header JSON");
    ImGui::InputTextMultiline(
        "##JwtHeaderInput",
        headerInput_.data(),
        headerInput_.size(),
        ImVec2(-1.0F, ImGui::GetTextLineHeight() * 5.0F)
    );

    ImGui::TextUnformatted("Payload JSON");
    ImGui::InputTextMultiline(
        "##JwtPayloadInput",
        payloadInput_.data(),
        payloadInput_.size(),
        ImVec2(-1.0F, ImGui::GetTextLineHeight() * 8.0F)
    );

    if (ImGui::Button("Encode Unsigned Token")) {
        decodedHeader_.clear();
        decodedPayload_.clear();
        encodedToken_ = base64UrlEncode(headerInput_.data()) + "." + base64UrlEncode(payloadInput_.data()) + ".";
        status_ = "Encoded unsigned token";
    }

    ImGui::SameLine();
    if (ImGui::Button("Reset Example")) {
        copyToBuffer(R"({"alg":"none","typ":"JWT"})", headerInput_);
        copyToBuffer(R"({"sub":"1234567890","name":"DevTools","iat":1710000000})", payloadInput_);
        encodedToken_.clear();
        status_.clear();
    }

    if (!status_.empty()) {
        ImGui::SameLine();
        ImGui::TextDisabled("%s", status_.c_str());
    }

    if (!encodedToken_.empty()) {
        ImGui::Separator();
        ImGui::TextUnformatted("Token");
        ImGui::SameLine();
        if (ImGui::Button("Copy Token")) {
            ui::copyToClipboard(encodedToken_.c_str());
            status_ = "Copied token";
        }

        std::vector<char> tokenBuffer(encodedToken_.begin(), encodedToken_.end());
        tokenBuffer.push_back('\0');
        ImGui::InputTextMultiline(
            "##JwtEncodedToken",
            tokenBuffer.data(),
            tokenBuffer.size(),
            ImVec2(-1.0F, ImGui::GetTextLineHeight() * 5.0F),
            ImGuiInputTextFlags_ReadOnly
        );
    }
}

} // namespace tools::jwt
