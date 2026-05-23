#include "tools/text_diff/TextDiffTool.hpp"

#include "ui/Clipboard.hpp"

#include <imgui.h>

#include <algorithm>
#include <string_view>
#include <vector>

namespace {

enum class DiffKind {
    Equal,
    Added,
    Removed,
};

struct DiffLine {
    DiffKind kind;
    std::string_view text;
};

std::vector<std::string_view> splitLines(std::string_view text)
{
    std::vector<std::string_view> lines;
    std::size_t start = 0;
    while (start < text.size()) {
        const std::size_t newline = text.find('\n', start);
        if (newline == std::string_view::npos) {
            lines.push_back(text.substr(start));
            return lines;
        }

        lines.push_back(text.substr(start, newline - start));
        start = newline + 1U;
    }

    if (!text.empty() && text.back() == '\n') {
        lines.emplace_back();
    }

    return lines;
}

std::string buildUnifiedDiff(std::string_view left, std::string_view right, int& added, int& removed)
{
    const std::vector<std::string_view> leftLines = splitLines(left);
    const std::vector<std::string_view> rightLines = splitLines(right);
    std::vector<std::vector<int>> lcs(leftLines.size() + 1U, std::vector<int>(rightLines.size() + 1U, 0));

    for (std::size_t leftIndex = leftLines.size(); leftIndex > 0; --leftIndex) {
        for (std::size_t rightIndex = rightLines.size(); rightIndex > 0; --rightIndex) {
            if (leftLines[leftIndex - 1U] == rightLines[rightIndex - 1U]) {
                lcs[leftIndex - 1U][rightIndex - 1U] = lcs[leftIndex][rightIndex] + 1;
            } else {
                lcs[leftIndex - 1U][rightIndex - 1U] = std::max(
                    lcs[leftIndex][rightIndex - 1U],
                    lcs[leftIndex - 1U][rightIndex]
                );
            }
        }
    }

    std::vector<DiffLine> diff;
    std::size_t leftIndex = 0;
    std::size_t rightIndex = 0;
    while (leftIndex < leftLines.size() || rightIndex < rightLines.size()) {
        if (leftIndex < leftLines.size()
            && rightIndex < rightLines.size()
            && leftLines[leftIndex] == rightLines[rightIndex]) {
            diff.push_back({ DiffKind::Equal, leftLines[leftIndex] });
            ++leftIndex;
            ++rightIndex;
        } else if (rightIndex < rightLines.size()
            && (leftIndex == leftLines.size() || lcs[leftIndex][rightIndex + 1U] >= lcs[leftIndex + 1U][rightIndex])) {
            diff.push_back({ DiffKind::Added, rightLines[rightIndex] });
            ++rightIndex;
            ++added;
        } else {
            diff.push_back({ DiffKind::Removed, leftLines[leftIndex] });
            ++leftIndex;
            ++removed;
        }
    }

    std::string output = "--- Left\n+++ Right\n";
    for (const DiffLine& line : diff) {
        switch (line.kind) {
        case DiffKind::Equal:
            output += "  ";
            break;
        case DiffKind::Added:
            output += "+ ";
            break;
        case DiffKind::Removed:
            output += "- ";
            break;
        }
        output.append(line.text);
        output += '\n';
    }

    return output;
}

} // namespace

namespace tools::text_diff {

void TextDiffTool::draw()
{
    const float inputHeight = ImGui::GetTextLineHeight() * 12.0F;
    const float halfWidth = (ImGui::GetContentRegionAvail().x - ImGui::GetStyle().ItemSpacing.x) * 0.5F;

    ImGui::BeginGroup();
    ImGui::TextUnformatted("Left");
    ImGui::InputTextMultiline("##TextDiffLeft", leftInput_.data(), leftInput_.size(), ImVec2(halfWidth, inputHeight));
    ImGui::EndGroup();

    ImGui::SameLine();

    ImGui::BeginGroup();
    ImGui::TextUnformatted("Right");
    ImGui::InputTextMultiline("##TextDiffRight", rightInput_.data(), rightInput_.size(), ImVec2(-1.0F, inputHeight));
    ImGui::EndGroup();

    if (ImGui::Button("Compare")) {
        compare();
    }

    ImGui::SameLine();
    if (ImGui::Button("Clear")) {
        leftInput_.fill('\0');
        rightInput_.fill('\0');
        diffOutput_.clear();
        status_.clear();
    }

    if (!diffOutput_.empty()) {
        ImGui::SameLine();
        if (ImGui::Button("Copy Diff")) {
            ui::copyToClipboard(diffOutput_.c_str());
            status_ = "Copied diff";
        }
    }

    if (!status_.empty()) {
        ImGui::SameLine();
        ImGui::TextDisabled("%s", status_.c_str());
    }

    if (!diffOutput_.empty()) {
        ImGui::Separator();
        std::vector<char> outputBuffer(diffOutput_.begin(), diffOutput_.end());
        outputBuffer.push_back('\0');
        ImGui::InputTextMultiline(
            "##TextDiffOutput",
            outputBuffer.data(),
            outputBuffer.size(),
            ImVec2(-1.0F, ImGui::GetTextLineHeight() * 16.0F),
            ImGuiInputTextFlags_ReadOnly
        );
    }
}

void TextDiffTool::compare()
{
    int added = 0;
    int removed = 0;
    diffOutput_ = buildUnifiedDiff(leftInput_.data(), rightInput_.data(), added, removed);

    if (added == 0 && removed == 0) {
        status_ = "No differences";
    } else {
        status_ = std::to_string(added) + " added, " + std::to_string(removed) + " removed";
    }
}

} // namespace tools::text_diff
