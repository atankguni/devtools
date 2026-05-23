#include "tools/regex/RegexTool.hpp"

#include "ui/Clipboard.hpp"

#include <imgui.h>

#include <regex>
#include <string_view>
#include <vector>

namespace tools::regex_tool {

void RegexTool::draw()
{
    ImGui::TextUnformatted("Pattern");
    ImGui::InputText("##RegexPattern", pattern_.data(), pattern_.size());
    ImGui::Checkbox("Case insensitive", &caseInsensitive_);
    ImGui::TextUnformatted("Test text");
    ImGui::InputTextMultiline("##RegexText", text_.data(), text_.size(), ImVec2(-1.0F, ImGui::GetTextLineHeight() * 10.0F));

    if (ImGui::Button("Run")) {
        run();
    }
    ImGui::SameLine();
    if (ImGui::Button("Clear")) {
        pattern_.fill('\0');
        text_.fill('\0');
        output_.clear();
        status_.clear();
    }
    if (!status_.empty()) {
        ImGui::SameLine();
        ImGui::TextDisabled("%s", status_.c_str());
    }

    if (!output_.empty()) {
        ImGui::Separator();
        if (ImGui::Button("Copy Matches")) {
            ui::copyToClipboard(output_.c_str());
            status_ = "Copied matches";
        }
        std::vector<char> buffer(output_.begin(), output_.end());
        buffer.push_back('\0');
        ImGui::InputTextMultiline("##RegexOutput", buffer.data(), buffer.size(), ImVec2(-1.0F, ImGui::GetTextLineHeight() * 12.0F), ImGuiInputTextFlags_ReadOnly);
    }
}

void RegexTool::run()
{
    output_.clear();
    try {
        std::regex::flag_type flags = std::regex::ECMAScript;
        if (caseInsensitive_) {
            flags |= std::regex::icase;
        }
        const std::regex expression(pattern_.data(), flags);
        const std::string input(text_.data());
        int count = 0;
        for (std::sregex_iterator it(input.begin(), input.end(), expression), end; it != end; ++it) {
            ++count;
            output_ += "Match " + std::to_string(count) + " at " + std::to_string(it->position()) + ": " + it->str() + '\n';
            for (std::size_t group = 1; group < it->size(); ++group) {
                output_ += "  $" + std::to_string(group) + ": " + (*it)[group].str() + '\n';
            }
        }
        status_ = count == 0 ? "No matches" : std::to_string(count) + " matches";
    } catch (const std::regex_error& error) {
        status_ = std::string("Regex error: ") + error.what();
    }
}

} // namespace tools::regex_tool
