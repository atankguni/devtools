#include "tools/markup/MarkupTool.hpp"

#include <imgui.h>

#include <algorithm>
#include <cctype>
#include <string_view>
#include <vector>

namespace {

std::string trim(std::string_view value)
{
    std::size_t start = 0;
    while (start < value.size() && std::isspace(static_cast<unsigned char>(value[start])) != 0) {
        ++start;
    }
    std::size_t end = value.size();
    while (end > start && std::isspace(static_cast<unsigned char>(value[end - 1U])) != 0) {
        --end;
    }
    return std::string(value.substr(start, end - start));
}

bool startsWith(std::string_view value, std::string_view prefix)
{
    return value.substr(0, prefix.size()) == prefix;
}

bool isClosingTag(std::string_view tag)
{
    return tag.size() >= 2U && tag[0] == '<' && tag[1] == '/';
}

bool isSelfClosingTag(std::string_view tag)
{
    return startsWith(tag, "<!") || startsWith(tag, "<?") || (tag.size() >= 2U && tag[tag.size() - 2U] == '/');
}

std::string formatMarkup(std::string_view input)
{
    std::string output;
    int depth = 0;
    std::size_t index = 0;
    while (index < input.size()) {
        if (input[index] != '<') {
            const std::size_t next = input.find('<', index);
            const std::string text = trim(input.substr(index, next == std::string_view::npos ? input.size() - index : next - index));
            if (!text.empty()) {
                output.append(static_cast<std::size_t>(std::max(depth, 0)) * 2U, ' ');
                output += text + '\n';
            }
            if (next == std::string_view::npos) {
                break;
            }
            index = next;
            continue;
        }

        const std::size_t close = input.find('>', index);
        if (close == std::string_view::npos) {
            output.append(static_cast<std::size_t>(std::max(depth, 0)) * 2U, ' ');
            output.append(input.substr(index));
            output += '\n';
            break;
        }

        const std::string tag = trim(input.substr(index, close - index + 1U));
        if (isClosingTag(tag)) {
            depth = std::max(0, depth - 1);
        }
        output.append(static_cast<std::size_t>(std::max(depth, 0)) * 2U, ' ');
        output += tag + '\n';
        if (!isClosingTag(tag) && !isSelfClosingTag(tag)) {
            ++depth;
        }
        index = close + 1U;
    }
    return output;
}

std::string minifyMarkup(std::string_view input)
{
    std::string output;
    bool previousWhitespace = false;
    for (char ch : input) {
        if (std::isspace(static_cast<unsigned char>(ch)) != 0) {
            previousWhitespace = true;
            continue;
        }
        if (previousWhitespace && !output.empty() && output.back() != '>' && ch != '<') {
            output += ' ';
        }
        output += ch;
        previousWhitespace = false;
    }
    return output;
}

} // namespace

namespace tools::markup {

void MarkupTool::draw()
{
    ImGui::TextUnformatted("HTML / XML");
    ImGui::InputTextMultiline("##MarkupInput", input_.data(), input_.size(), ImVec2(-1.0F, ImGui::GetTextLineHeight() * 12.0F));
    if (ImGui::Button("Format")) {
        output_ = formatMarkup(input_.data());
        status_ = "Formatted";
    }
    ImGui::SameLine();
    if (ImGui::Button("Minify")) {
        output_ = minifyMarkup(input_.data());
        status_ = "Minified";
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

    if (!output_.empty()) {
        ImGui::Separator();
        if (ImGui::Button("Copy Output")) {
            ImGui::SetClipboardText(output_.c_str());
            status_ = "Copied output";
        }
        std::vector<char> buffer(output_.begin(), output_.end());
        buffer.push_back('\0');
        ImGui::InputTextMultiline("##MarkupOutput", buffer.data(), buffer.size(), ImVec2(-1.0F, ImGui::GetTextLineHeight() * 14.0F), ImGuiInputTextFlags_ReadOnly);
    }
}

} // namespace tools::markup
