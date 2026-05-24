#include "tools/line_tools/LineToolsTool.hpp"

#include "ui/Clipboard.hpp"

#include <imgui.h>

#include <algorithm>
#include <cctype>
#include <set>
#include <sstream>
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

std::string lowerCopy(std::string value)
{
    std::ranges::transform(value, value.begin(), [](unsigned char ch) {
        return static_cast<char>(std::tolower(ch));
    });
    return value;
}

std::vector<std::string> splitLines(std::string_view input)
{
    std::vector<std::string> lines;
    std::string current;
    for (char ch : input) {
        if (ch == '\r') {
            continue;
        }
        if (ch == '\n') {
            lines.push_back(current);
            current.clear();
            continue;
        }
        current += ch;
    }
    lines.push_back(current);
    return lines;
}

} // namespace

namespace tools::line_tools {

void LineToolsTool::draw()
{
    ImGui::TextUnformatted("Input");
    ImGui::InputTextMultiline("##LineToolsInput", input_.data(), input_.size(), ImVec2(-1.0F, ImGui::GetTextLineHeight() * 12.0F));

    ImGui::Checkbox("Trim lines", &trimLines_);
    ImGui::SameLine();
    ImGui::Checkbox("Remove empty", &removeEmpty_);
    ImGui::SameLine();
    ImGui::Checkbox("Dedupe", &dedupe_);

    ImGui::Checkbox("Descending", &descending_);
    ImGui::SameLine();
    ImGui::Checkbox("Case-sensitive", &caseSensitive_);

    if (ImGui::Button("Sort Lines")) {
        transform();
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
    ImGui::InputTextMultiline("##LineToolsOutput", buffer.data(), buffer.size(), ImVec2(-1.0F, ImGui::GetTextLineHeight() * 14.0F), ImGuiInputTextFlags_ReadOnly);
}

void LineToolsTool::transform()
{
    std::vector<std::string> lines = splitLines(input_.data());
    if (trimLines_) {
        for (std::string& line : lines) {
            line = trim(line);
        }
    }

    if (removeEmpty_) {
        std::erase_if(lines, [](const std::string& line) { return line.empty(); });
    }

    const auto projection = [this](const std::string& value) {
        return caseSensitive_ ? value : lowerCopy(value);
    };

    std::ranges::sort(lines, {}, projection);
    if (descending_) {
        std::ranges::reverse(lines);
    }

    const std::size_t beforeDedupe = lines.size();
    if (dedupe_) {
        std::set<std::string> seen;
        std::vector<std::string> unique;
        for (const std::string& line : lines) {
            const std::string key = caseSensitive_ ? line : lowerCopy(line);
            if (seen.insert(key).second) {
                unique.push_back(line);
            }
        }
        lines = std::move(unique);
    }

    std::ostringstream stream;
    for (std::size_t index = 0; index < lines.size(); ++index) {
        if (index > 0) {
            stream << '\n';
        }
        stream << lines[index];
    }
    output_ = stream.str();

    status_ = "Sorted " + std::to_string(lines.size()) + " lines";
    if (dedupe_ && beforeDedupe != lines.size()) {
        status_ += ", removed " + std::to_string(beforeDedupe - lines.size()) + " duplicates";
    }
}

} // namespace tools::line_tools
