#include "tools/url/UrlTool.hpp"

#include "ui/Clipboard.hpp"
#include "ui/Workbench.hpp"

#include <imgui.h>

#include <cctype>
#include <iomanip>
#include <sstream>
#include <string>
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
    if (ui::workbench::primaryButton("Encode")) {
        output_ = encodeUrl(input_.data());
        status_ = "Encoded";
        statusIsError_ = false;
    }
    ImGui::SameLine();
    if (ui::workbench::quietButton("Decode")) {
        statusIsError_ = !decodeUrl(input_.data(), output_);
        status_ = statusIsError_ ? "Invalid percent encoding" : "Decoded";
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

    if (ui::workbench::beginPanel("UrlInputPane", "Input", "URL text or percent-encoded value", paneSize)) {
        ImGui::InputTextMultiline(
            "##UrlInput",
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

    std::vector<char> buffer(output_.begin(), output_.end());
    buffer.push_back('\0');
    const std::string outputDetail = output_.empty() ? "Result appears here" : std::to_string(output_.size()) + " bytes";
    if (ui::workbench::beginPanel("UrlOutputPane", "Output", outputDetail, ImVec2(0.0F, 0.0F))) {
        ImGui::InputTextMultiline(
            "##UrlOutput",
            buffer.data(),
            buffer.size(),
            ImVec2(-1.0F, -1.0F),
            ImGuiInputTextFlags_ReadOnly
        );
    }
    ui::workbench::endPanel();
}

} // namespace tools::url
