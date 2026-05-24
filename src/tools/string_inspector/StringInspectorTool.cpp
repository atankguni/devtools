#include "tools/string_inspector/StringInspectorTool.hpp"

#include "ui/Clipboard.hpp"

#include <imgui.h>

#include <cctype>
#include <iomanip>
#include <sstream>
#include <string_view>
#include <vector>

namespace {

struct Utf8Result {
    std::size_t codepoints = 0;
    bool valid = true;
    std::string preview;
};

Utf8Result inspectUtf8(std::string_view input)
{
    Utf8Result result;
    std::ostringstream preview;
    std::size_t index = 0;
    while (index < input.size()) {
        const auto lead = static_cast<unsigned char>(input[index]);
        unsigned int codepoint = 0;
        std::size_t length = 0;

        if (lead < 0x80U) {
            codepoint = lead;
            length = 1;
        } else if ((lead & 0xE0U) == 0xC0U) {
            codepoint = lead & 0x1FU;
            length = 2;
        } else if ((lead & 0xF0U) == 0xE0U) {
            codepoint = lead & 0x0FU;
            length = 3;
        } else if ((lead & 0xF8U) == 0xF0U) {
            codepoint = lead & 0x07U;
            length = 4;
        } else {
            result.valid = false;
            ++index;
            continue;
        }

        if (index + length > input.size()) {
            result.valid = false;
            break;
        }

        for (std::size_t offset = 1; offset < length; ++offset) {
            const auto part = static_cast<unsigned char>(input[index + offset]);
            if ((part & 0xC0U) != 0x80U) {
                result.valid = false;
                length = offset;
                break;
            }
            codepoint = (codepoint << 6U) | (part & 0x3FU);
        }

        if (result.codepoints < 256U) {
            preview << "U+" << std::uppercase << std::hex << std::setw(4) << std::setfill('0') << codepoint << ' ';
        }
        ++result.codepoints;
        index += length;
    }

    result.preview = preview.str();
    return result;
}

std::string escapedView(std::string_view input)
{
    std::ostringstream stream;
    stream << std::uppercase << std::hex << std::setfill('0');
    for (unsigned char ch : input) {
        switch (ch) {
        case '\n':
            stream << "\\n";
            break;
        case '\r':
            stream << "\\r";
            break;
        case '\t':
            stream << "\\t";
            break;
        case '\\':
            stream << "\\\\";
            break;
        case '"':
            stream << "\\\"";
            break;
        default:
            if (ch < 0x20U || ch == 0x7FU) {
                stream << "\\x" << std::setw(2) << static_cast<unsigned int>(ch);
            } else {
                stream << static_cast<char>(ch);
            }
            break;
        }
    }
    return stream.str();
}

} // namespace

namespace tools::string_inspector {

void StringInspectorTool::draw()
{
    ImGui::TextUnformatted("Input");
    ImGui::InputTextMultiline("##StringInspectorInput", input_.data(), input_.size(), ImVec2(-1.0F, ImGui::GetTextLineHeight() * 12.0F));

    if (ImGui::Button("Inspect")) {
        inspect();
    }
    ImGui::SameLine();
    if (ImGui::Button("Clear")) {
        input_.fill('\0');
        summary_.clear();
        escaped_.clear();
        codepoints_.clear();
        status_.clear();
    }
    if (!status_.empty()) {
        ImGui::SameLine();
        ImGui::TextDisabled("%s", status_.c_str());
    }

    ImGui::Separator();
    ImGui::TextUnformatted(summary_.c_str());

    if (!escaped_.empty() && ImGui::Button("Copy Escaped")) {
        ui::copyToClipboard(escaped_.c_str());
        status_ = "Copied escaped text";
    }
    std::vector<char> escapedBuffer(escaped_.begin(), escaped_.end());
    escapedBuffer.push_back('\0');
    ImGui::InputTextMultiline("##StringEscaped", escapedBuffer.data(), escapedBuffer.size(), ImVec2(-1.0F, ImGui::GetTextLineHeight() * 8.0F), ImGuiInputTextFlags_ReadOnly);

    std::vector<char> codepointBuffer(codepoints_.begin(), codepoints_.end());
    codepointBuffer.push_back('\0');
    ImGui::InputTextMultiline("##StringCodepoints", codepointBuffer.data(), codepointBuffer.size(), ImVec2(-1.0F, ImGui::GetTextLineHeight() * 6.0F), ImGuiInputTextFlags_ReadOnly);
}

void StringInspectorTool::inspect()
{
    const std::string_view input(input_.data());
    std::size_t lines = input.empty() ? 0U : 1U;
    std::size_t words = 0;
    std::size_t whitespace = 0;
    std::size_t control = 0;
    bool inWord = false;

    for (unsigned char ch : input) {
        if (ch == '\n') {
            ++lines;
        }
        if (std::isspace(ch) != 0) {
            ++whitespace;
            inWord = false;
        } else if (!inWord) {
            ++words;
            inWord = true;
        }
        if (std::iscntrl(ch) != 0) {
            ++control;
        }
    }

    const Utf8Result utf8 = inspectUtf8(input);
    std::ostringstream stream;
    stream << "Bytes: " << input.size()
           << "\nUTF-8 code points: " << utf8.codepoints
           << "\nUTF-8 valid: " << (utf8.valid ? "yes" : "no")
           << "\nLines: " << lines
           << "\nWords: " << words
           << "\nWhitespace bytes: " << whitespace
           << "\nControl bytes: " << control;

    summary_ = stream.str();
    escaped_ = escapedView(input);
    codepoints_ = utf8.preview;
    status_ = "Inspected";
}

} // namespace tools::string_inspector
