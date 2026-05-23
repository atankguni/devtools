#include "tools/cron/CronTool.hpp"

#include "ui/Clipboard.hpp"

#include <imgui.h>

#include <algorithm>
#include <array>
#include <charconv>
#include <chrono>
#include <ctime>
#include <sstream>
#include <span>
#include <string_view>
#include <vector>

namespace {

void copyToBuffer(std::string_view value, std::span<char> buffer)
{
    std::ranges::fill(buffer, '\0');
    value.copy(buffer.data(), std::min(value.size(), buffer.size() - 1U));
}

std::vector<std::string_view> split(std::string_view value, char separator)
{
    std::vector<std::string_view> parts;
    std::size_t start = 0;
    while (start <= value.size()) {
        const std::size_t end = value.find(separator, start);
        if (end == std::string_view::npos) {
            parts.push_back(value.substr(start));
            break;
        }
        parts.push_back(value.substr(start, end - start));
        start = end + 1U;
    }
    return parts;
}

bool parseInt(std::string_view value, int& output)
{
    const auto result = std::from_chars(value.data(), value.data() + value.size(), output);
    return result.ec == std::errc {} && result.ptr == value.data() + value.size();
}

bool markRange(std::vector<bool>& values, int minValue, int maxValue, int start, int end, int step)
{
    if (start < minValue || end > maxValue || start > end || step <= 0) {
        return false;
    }
    for (int value = start; value <= end; value += step) {
        values[static_cast<std::size_t>(value - minValue)] = true;
    }
    return true;
}

bool parseField(std::string_view field, int minValue, int maxValue, std::vector<bool>& values)
{
    values.assign(static_cast<std::size_t>(maxValue - minValue + 1), false);
    for (std::string_view part : split(field, ',')) {
        int step = 1;
        const std::size_t slash = part.find('/');
        if (slash != std::string_view::npos) {
            if (!parseInt(part.substr(slash + 1U), step)) {
                return false;
            }
            part = part.substr(0, slash);
        }

        if (part == "*") {
            if (!markRange(values, minValue, maxValue, minValue, maxValue, step)) {
                return false;
            }
            continue;
        }

        const std::size_t dash = part.find('-');
        if (dash != std::string_view::npos) {
            int start = 0;
            int end = 0;
            if (!parseInt(part.substr(0, dash), start) || !parseInt(part.substr(dash + 1U), end)) {
                return false;
            }
            if (!markRange(values, minValue, maxValue, start, end, step)) {
                return false;
            }
            continue;
        }

        int exact = 0;
        if (!parseInt(part, exact) || !markRange(values, minValue, maxValue, exact, exact, step)) {
            return false;
        }
    }
    return true;
}

bool matches(const std::vector<bool>& values, int minValue, int value)
{
    const int index = value - minValue;
    return index >= 0 && static_cast<std::size_t>(index) < values.size() && values[static_cast<std::size_t>(index)];
}

std::string formatTime(std::time_t value)
{
    std::tm local {};
#if defined(_WIN32)
    localtime_s(&local, &value);
#else
    localtime_r(&value, &local);
#endif
    char buffer[64] {};
    std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S %Z", &local);
    return buffer;
}

} // namespace

namespace tools::cron {

CronTool::CronTool()
{
    copyToBuffer("*/15 * * * *", expression_);
}

void CronTool::draw()
{
    ImGui::TextUnformatted("Cron expression");
    ImGui::InputText("##CronExpression", expression_.data(), expression_.size());
    ImGui::TextDisabled("Supports five fields: minute hour day-of-month month day-of-week.");

    if (ImGui::Button("Show Next Runs")) {
        const std::vector<std::string_view> fields = split(expression_.data(), ' ');
        if (fields.size() != 5U) {
            status_ = "Expected 5 fields";
            output_.clear();
            return;
        }

        std::vector<bool> minutes;
        std::vector<bool> hours;
        std::vector<bool> days;
        std::vector<bool> months;
        std::vector<bool> weekdays;
        if (!parseField(fields[0], 0, 59, minutes)
            || !parseField(fields[1], 0, 23, hours)
            || !parseField(fields[2], 1, 31, days)
            || !parseField(fields[3], 1, 12, months)
            || !parseField(fields[4], 0, 6, weekdays)) {
            status_ = "Invalid cron field";
            output_.clear();
            return;
        }

        output_.clear();
        auto current = std::chrono::system_clock::now() + std::chrono::minutes(1);
        int found = 0;
        for (int scanned = 0; scanned < 525600 && found < 10; ++scanned) {
            const std::time_t timestamp = std::chrono::system_clock::to_time_t(current);
            std::tm local {};
#if defined(_WIN32)
            localtime_s(&local, &timestamp);
#else
            localtime_r(&timestamp, &local);
#endif
            if (matches(minutes, 0, local.tm_min)
                && matches(hours, 0, local.tm_hour)
                && matches(days, 1, local.tm_mday)
                && matches(months, 1, local.tm_mon + 1)
                && matches(weekdays, 0, local.tm_wday)) {
                output_ += formatTime(timestamp) + '\n';
                ++found;
            }
            current += std::chrono::minutes(1);
        }
        status_ = found == 0 ? "No runs found in one year" : "Found next runs";
    }

    ImGui::SameLine();
    if (ImGui::Button("Clear")) {
        output_.clear();
        status_.clear();
    }
    if (!status_.empty()) {
        ImGui::SameLine();
        ImGui::TextDisabled("%s", status_.c_str());
    }

    if (!output_.empty()) {
        ImGui::Separator();
        if (ImGui::Button("Copy Runs")) {
            ui::copyToClipboard(output_.c_str());
            status_ = "Copied runs";
        }
        ImGui::TextUnformatted(output_.c_str());
    }
}

} // namespace tools::cron
