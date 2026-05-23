#include "tools/number_base/NumberBaseTool.hpp"

#include "ui/Clipboard.hpp"

#include <imgui.h>

#include <algorithm>
#include <array>
#include <charconv>
#include <cctype>
#include <string>
#include <string_view>

namespace {

std::string trimAsciiWhitespace(std::string_view input)
{
    while (!input.empty() && (input.front() == ' ' || input.front() == '\t' || input.front() == '\n' || input.front() == '\r')) {
        input.remove_prefix(1);
    }
    while (!input.empty() && (input.back() == ' ' || input.back() == '\t' || input.back() == '\n' || input.back() == '\r')) {
        input.remove_suffix(1);
    }
    return std::string(input);
}

std::string stripBasePrefix(std::string value, int base)
{
    if (value.size() < 2U || value[0] != '0') {
        return value;
    }

    const char prefix = value[1];
    if ((base == 2 && (prefix == 'b' || prefix == 'B'))
        || (base == 8 && (prefix == 'o' || prefix == 'O'))
        || (base == 16 && (prefix == 'x' || prefix == 'X'))) {
        value.erase(0, 2);
    }

    return value;
}

std::string formatUnsigned(unsigned long long value, int base)
{
    std::array<char, 128> buffer {};
    const auto result = std::to_chars(buffer.data(), buffer.data() + buffer.size(), value, base);
    if (result.ec != std::errc {}) {
        return {};
    }
    std::string formatted(buffer.data(), result.ptr);
    if (base == 16) {
        std::ranges::transform(formatted, formatted.begin(), [](unsigned char ch) {
            return static_cast<char>(std::toupper(ch));
        });
    }
    return formatted;
}

bool parseUnsigned(std::string_view input, int base, unsigned long long& value)
{
    std::string normalized = stripBasePrefix(trimAsciiWhitespace(input), base);
    normalized.erase(std::remove(normalized.begin(), normalized.end(), '_'), normalized.end());
    if (normalized.empty() || normalized.front() == '-' || normalized.front() == '+') {
        return false;
    }

    const auto result = std::from_chars(normalized.data(), normalized.data() + normalized.size(), value, base);
    return result.ec == std::errc {} && result.ptr == normalized.data() + normalized.size();
}

template <std::size_t Size>
void setInput(std::array<char, Size>& input, std::string_view value)
{
    input.fill('\0');
    const std::size_t length = std::min(value.size(), input.size() - 1U);
    value.copy(input.data(), length);
}

template <std::size_t Size>
void drawCopyableInput(const char* label, const char* id, std::array<char, Size>& input, int base, std::string& status, auto&& onChanged)
{
    ImGui::PushID(id);
    ImGui::TextUnformatted(label);
    ImGui::SameLine(112.0F);
    ImGui::SetNextItemWidth(-84.0F);
    if (ImGui::InputText("##value", input.data(), input.size(), ImGuiInputTextFlags_CharsNoBlank)) {
        onChanged(base, label);
    }
    ImGui::SameLine();
    if (ImGui::Button("Copy", ImVec2(72.0F, 0.0F))) {
        ui::copyToClipboard(input.data());
        status = std::string("Copied ") + label;
    }
    ImGui::PopID();
}

} // namespace

namespace tools::number_base {

void NumberBaseTool::draw()
{
    ImGui::TextUnformatted("Enter a non-negative integer in any base.");
    ImGui::Spacing();

    auto update = [this](int base, const char* source) {
        updateFromInput(base, source);
    };

    drawCopyableInput("Decimal", "decimal", decimalInput_, 10, status_, update);
    drawCopyableInput("Octal", "octal", octalInput_, 8, status_, update);
    drawCopyableInput("Hexadecimal", "hexadecimal", hexadecimalInput_, 16, status_, update);
    drawCopyableInput("Binary", "binary", binaryInput_, 2, status_, update);

    if (ImGui::Button("Clear")) {
        setValue(0);
        status_.clear();
    }
    if (!status_.empty()) {
        ImGui::SameLine();
        ImGui::TextDisabled("%s", status_.c_str());
    }
}

void NumberBaseTool::updateFromInput(int base, const char* source)
{
    const char* input = decimalInput_.data();
    if (base == 8) {
        input = octalInput_.data();
    } else if (base == 16) {
        input = hexadecimalInput_.data();
    } else if (base == 2) {
        input = binaryInput_.data();
    }

    unsigned long long value = 0;
    if (!parseUnsigned(input, base, value)) {
        status_ = std::string("Invalid ") + source;
        return;
    }

    setValue(value);
    status_ = std::string("Updated from ") + source;
}

void NumberBaseTool::setValue(unsigned long long value)
{
    setInput(decimalInput_, formatUnsigned(value, 10));
    setInput(octalInput_, formatUnsigned(value, 8));
    setInput(hexadecimalInput_, formatUnsigned(value, 16));
    setInput(binaryInput_, formatUnsigned(value, 2));
}

} // namespace tools::number_base
