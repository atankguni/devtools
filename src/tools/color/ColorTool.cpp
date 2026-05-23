#include "tools/color/ColorTool.hpp"

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

    ImGui::TextUnformatted("HEX");
    if (ImGui::InputText("##HexInput", hexInput_.data(), hexInput_.size())) {
        updateFromHex();
    }

    bool changed = false;
    changed |= ImGui::SliderInt("Red", &red_, 0, 255);
    changed |= ImGui::SliderInt("Green", &green_, 0, 255);
    changed |= ImGui::SliderInt("Blue", &blue_, 0, 255);
    if (changed) {
        updateFromRgb();
    }

    const ImVec4 color(static_cast<float>(red_) / 255.0F, static_cast<float>(green_) / 255.0F, static_cast<float>(blue_) / 255.0F, 1.0F);
    ImGui::ColorButton("Preview", color, ImGuiColorEditFlags_NoTooltip, ImVec2(80.0F, 36.0F));
    if (!status_.empty()) {
        ImGui::SameLine();
        ImGui::TextDisabled("%s", status_.c_str());
    }

    ImGui::Separator();
    if (ImGui::Button("Copy Values")) {
        ImGui::SetClipboardText(output_.c_str());
        status_ = "Copied values";
    }
    ImGui::TextUnformatted(output_.c_str());
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
