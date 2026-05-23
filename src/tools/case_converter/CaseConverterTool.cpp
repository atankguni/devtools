#include "tools/case_converter/CaseConverterTool.hpp"

#include "ui/Clipboard.hpp"

#include <imgui.h>

#include <algorithm>
#include <cctype>
#include <string_view>
#include <vector>

namespace {

std::vector<std::string> wordsFrom(std::string_view input)
{
    std::vector<std::string> words;
    std::string current;
    char previous = '\0';
    for (char raw : input) {
        const unsigned char ch = static_cast<unsigned char>(raw);
        if (std::isalnum(ch) == 0) {
            if (!current.empty()) {
                words.push_back(current);
                current.clear();
            }
            previous = '\0';
            continue;
        }
        if (!current.empty() && std::isupper(ch) != 0 && std::islower(static_cast<unsigned char>(previous)) != 0) {
            words.push_back(current);
            current.clear();
        }
        current += static_cast<char>(std::tolower(ch));
        previous = raw;
    }
    if (!current.empty()) {
        words.push_back(current);
    }
    return words;
}

std::string capitalize(std::string word)
{
    if (!word.empty()) {
        word[0] = static_cast<char>(std::toupper(static_cast<unsigned char>(word[0])));
    }
    return word;
}

std::string joinWords(const std::vector<std::string>& words, std::string_view separator)
{
    std::string output;
    for (std::size_t index = 0; index < words.size(); ++index) {
        if (index > 0) {
            output.append(separator);
        }
        output.append(words[index]);
    }
    return output;
}

} // namespace

namespace tools::case_converter {

void CaseConverterTool::draw()
{
    ImGui::TextUnformatted("Input");
    ImGui::InputTextMultiline("##CaseInput", input_.data(), input_.size(), ImVec2(-1.0F, ImGui::GetTextLineHeight() * 8.0F));

    if (ImGui::Button("Convert")) {
        convert();
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
            ui::copyToClipboard(output_.c_str());
            status_ = "Copied output";
        }
        std::vector<char> buffer(output_.begin(), output_.end());
        buffer.push_back('\0');
        ImGui::InputTextMultiline("##CaseOutput", buffer.data(), buffer.size(), ImVec2(-1.0F, ImGui::GetTextLineHeight() * 14.0F), ImGuiInputTextFlags_ReadOnly);
    }
}

void CaseConverterTool::convert()
{
    const std::vector<std::string> words = wordsFrom(input_.data());
    if (words.empty()) {
        output_.clear();
        status_ = "No words found";
        return;
    }

    std::vector<std::string> upper = words;
    for (std::string& word : upper) {
        std::ranges::transform(word, word.begin(), [](unsigned char ch) { return static_cast<char>(std::toupper(ch)); });
    }

    std::string camel = words.front();
    std::string pascal;
    std::string title;
    for (std::size_t index = 0; index < words.size(); ++index) {
        const std::string capitalized = capitalize(words[index]);
        if (index > 0) {
            camel += capitalized;
            title += ' ';
        }
        pascal += capitalized;
        title += capitalized;
    }

    output_ = "camelCase: " + camel
        + "\nPascalCase: " + pascal
        + "\nsnake_case: " + joinWords(words, "_")
        + "\nkebab-case: " + joinWords(words, "-")
        + "\nCONSTANT_CASE: " + joinWords(upper, "_")
        + "\nTitle Case: " + title;
    status_ = "Converted";
}

} // namespace tools::case_converter
