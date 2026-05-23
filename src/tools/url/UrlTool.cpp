#include "tools/url/UrlTool.hpp"

#include "ui/Clipboard.hpp"

#include <imgui.h>

#include <cctype>
#include <iomanip>
#include <sstream>
#include <string_view>
#include <vector>

namespace {

std::string encodeUrl(std::string_view input)
{
    std::ostringstream stream;
    stream << std::uppercase << std::hex << std::setfill('0');
    for (const unsigned char ch : input) {
        if (std::isalnum(ch) != 0 || ch == '-' || ch == '_' || ch == '.' || ch == '~') {
            stream << static_cast<char>(ch);
        } else {
            stream << '%' << std::setw(2) << static_cast<unsigned int>(ch);
        }
    }
    return stream.str();
}

int hexValue(char ch)
{
    if (ch >= '0' && ch <= '9') {
        return ch - '0';
    }
    if (ch >= 'a' && ch <= 'f') {
        return ch - 'a' + 10;
    }
    if (ch >= 'A' && ch <= 'F') {
        return ch - 'A' + 10;
    }
    return -1;
}

bool decodeUrl(std::string_view input, std::string& output)
{
    output.clear();
    for (std::size_t index = 0; index < input.size(); ++index) {
        if (input[index] == '+') {
            output += ' ';
            continue;
        }
        if (input[index] != '%') {
            output += input[index];
            continue;
        }
        if (index + 2U >= input.size()) {
            return false;
        }
        const int high = hexValue(input[index + 1U]);
        const int low = hexValue(input[index + 2U]);
        if (high < 0 || low < 0) {
            return false;
        }
        output += static_cast<char>((high << 4) | low);
        index += 2U;
    }
    return true;
}

} // namespace

namespace tools::url {

void UrlTool::draw()
{
    ImGui::TextUnformatted("Input");
    ImGui::InputTextMultiline("##UrlInput", input_.data(), input_.size(), ImVec2(-1.0F, ImGui::GetTextLineHeight() * 10.0F));

    if (ImGui::Button("Encode")) {
        output_ = encodeUrl(input_.data());
        status_ = "Encoded";
    }
    ImGui::SameLine();
    if (ImGui::Button("Decode")) {
        status_ = decodeUrl(input_.data(), output_) ? "Decoded" : "Invalid percent encoding";
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

    std::vector<char> buffer(output_.begin(), output_.end());
    buffer.push_back('\0');
    ImGui::InputTextMultiline("##UrlOutput", buffer.data(), buffer.size(), ImVec2(-1.0F, ImGui::GetTextLineHeight() * 12.0F), ImGuiInputTextFlags_ReadOnly);
}

} // namespace tools::url
