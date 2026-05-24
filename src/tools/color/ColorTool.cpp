#include "tools/color/ColorTool.hpp"

#include "ui/Clipboard.hpp"

#include <imgui.h>

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <sstream>

namespace {

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

std::string makeOutput(int red, int green, int blue)
{
    const float r = static_cast<float>(red) / 255.0F;
    const float g = static_cast<float>(green) / 255.0F;
    const float b = static_cast<float>(blue) / 255.0F;
    const float maxValue = std::max({ r, g, b });
    const float minValue = std::min({ r, g, b });
    const float delta = maxValue - minValue;
    float hue = 0.0F;
    if (delta > 0.0F) {
        if (maxValue == r) {
            hue = 60.0F * std::fmod(((g - b) / delta), 6.0F);
        } else if (maxValue == g) {
            hue = 60.0F * (((b - r) / delta) + 2.0F);
        } else {
            hue = 60.0F * (((r - g) / delta) + 4.0F);
        }
    }
    if (hue < 0.0F) {
        hue += 360.0F;
    }
    const float lightness = (maxValue + minValue) * 0.5F;
    const float saturation = delta == 0.0F ? 0.0F : delta / (1.0F - std::fabs((2.0F * lightness) - 1.0F));
    const float value = maxValue;
    const float hsvSaturation = maxValue == 0.0F ? 0.0F : delta / maxValue;

    std::ostringstream stream;
    stream << std::uppercase << std::hex << std::setfill('0')
           << "HEX: #" << std::setw(2) << red << std::setw(2) << green << std::setw(2) << blue
           << std::dec << "\nRGB: rgb(" << red << ", " << green << ", " << blue << ")"
           << "\nHSL: hsl(" << std::lround(hue) << ", " << std::lround(saturation * 100.0F) << "%, " << std::lround(lightness * 100.0F) << "%)"
           << "\nHSV: hsv(" << std::lround(hue) << ", " << std::lround(hsvSaturation * 100.0F) << "%, " << std::lround(value * 100.0F) << "%)";
    return stream.str();
}

} // namespace

namespace tools::color {

void ColorTool::draw()
{
    if (output_.empty()) {
        output_ = makeOutput(red_, green_, blue_);
    }

    const ImVec4 color(static_cast<float>(red_) / 255.0F, static_cast<float>(green_) / 255.0F, static_cast<float>(blue_) / 255.0F, 1.0F);
    const ImVec2 available = ImGui::GetContentRegionAvail();
    const bool split = available.x >= 700.0F;
    const float previewWidth = split ? 230.0F : available.x;

    ImGui::BeginChild("ColorPreview", ImVec2(previewWidth, split ? 214.0F : 160.0F), true);
    const ImVec2 previewMin = ImGui::GetCursorScreenPos();
    const ImVec2 previewSize(ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y - 34.0F);
    ImGui::GetWindowDrawList()->AddRectFilled(
        previewMin,
        ImVec2(previewMin.x + previewSize.x, previewMin.y + previewSize.y),
        ImGui::ColorConvertFloat4ToU32(color),
        8.0F
    );
    ImGui::Dummy(previewSize);
    ImGui::TextUnformatted(hexInput_.data());
    if (!status_.empty()) {
        ImGui::SameLine();
        ImGui::TextDisabled("%s", status_.c_str());
    }
    ImGui::EndChild();

    if (split) {
        ImGui::SameLine();
    }

    ImGui::BeginChild("ColorControls", ImVec2(0.0F, 214.0F), true);
    ImGui::TextUnformatted("Color Input");
    ImGui::SetNextItemWidth(180.0F);
    if (ImGui::InputText("HEX", hexInput_.data(), hexInput_.size())) {
        updateFromHex();
    }

    bool changed = false;
    changed |= ImGui::SliderInt("Red", &red_, 0, 255);
    changed |= ImGui::SliderInt("Green", &green_, 0, 255);
    changed |= ImGui::SliderInt("Blue", &blue_, 0, 255);
    if (changed) {
        updateFromRgb();
    }

    if (ImGui::Button("Copy Values")) {
        ui::copyToClipboard(output_.c_str());
        status_ = "Copied values";
    }
    ImGui::EndChild();

    ImGui::Separator();
    ImGui::TextUnformatted("Converted Values");
    std::array<char, 256> outputBuffer {};
    output_.copy(outputBuffer.data(), outputBuffer.size() - 1U);
    ImGui::InputTextMultiline(
        "##ColorValues",
        outputBuffer.data(),
        outputBuffer.size(),
        ImVec2(-1.0F, ImGui::GetTextLineHeight() * 6.0F),
        ImGuiInputTextFlags_ReadOnly
    );
}

void ColorTool::updateFromHex()
{
    std::string value = hexInput_.data();
    if (!value.empty() && value.front() == '#') {
        value.erase(value.begin());
    }
    if (value.size() != 6U) {
        status_ = "Use 6-digit HEX";
        return;
    }
    int values[6] {};
    for (std::size_t index = 0; index < value.size(); ++index) {
        values[index] = hexValue(value[index]);
        if (values[index] < 0) {
            status_ = "Invalid HEX";
            return;
        }
    }
    red_ = values[0] * 16 + values[1];
    green_ = values[2] * 16 + values[3];
    blue_ = values[4] * 16 + values[5];
    output_ = makeOutput(red_, green_, blue_);
    status_ = "Updated";
}

void ColorTool::updateFromRgb()
{
    std::ostringstream hex;
    hex << '#' << std::uppercase << std::hex << std::setfill('0') << std::setw(2) << red_ << std::setw(2) << green_ << std::setw(2) << blue_;
    const std::string value = hex.str();
    hexInput_.fill('\0');
    value.copy(hexInput_.data(), std::min(value.size(), hexInput_.size() - 1U));
    output_ = makeOutput(red_, green_, blue_);
    status_ = "Updated";
}

} // namespace tools::color
