#include "tools/timestamp/TimestampTool.hpp"

#include <imgui.h>

#include <charconv>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <string_view>

namespace {

std::string formatLocalTime(std::time_t timestamp)
{
    std::tm local {};
#if defined(_WIN32)
    localtime_s(&local, &timestamp);
#else
    localtime_r(&timestamp, &local);
#endif

    std::ostringstream stream;
    stream << std::put_time(&local, "%Y-%m-%d %H:%M:%S %Z");
    return stream.str();
}

std::string formatUtcIso8601(std::time_t timestamp)
{
    std::tm utc {};
#if defined(_WIN32)
    gmtime_s(&utc, &timestamp);
#else
    gmtime_r(&timestamp, &utc);
#endif

    std::ostringstream stream;
    stream << std::put_time(&utc, "%Y-%m-%dT%H:%M:%SZ");
    return stream.str();
}

std::string formatLocalIso8601(std::time_t timestamp)
{
    std::tm local {};
#if defined(_WIN32)
    localtime_s(&local, &timestamp);
#else
    localtime_r(&timestamp, &local);
#endif

    std::ostringstream stream;
    stream << std::put_time(&local, "%Y-%m-%dT%H:%M:%S%z");
    std::string value = stream.str();
    if (value.size() >= 5U) {
        value.insert(value.size() - 2U, ":");
    }
    return value;
}

std::string formatTimestampResult(std::time_t timestamp)
{
    std::ostringstream stream;
    stream << "Local time: " << formatLocalTime(timestamp) << '\n'
           << "ISO 8601 UTC: " << formatUtcIso8601(timestamp) << '\n'
           << "ISO 8601 local: " << formatLocalIso8601(timestamp);
    return stream.str();
}

} // namespace

namespace tools::timestamp {

void TimestampTool::draw()
{
    const auto now = std::chrono::system_clock::now();
    const auto unixNow = std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count();

    ImGui::Text("Current Unix timestamp: %lld", static_cast<long long>(unixNow));
    ImGui::Text("Current ISO 8601 UTC: %s", formatUtcIso8601(static_cast<std::time_t>(unixNow)).c_str());

    if (ImGui::Button("Use Current Time")) {
        const std::string value = std::to_string(unixNow);
        timestampInput_.fill('\0');
        value.copy(timestampInput_.data(), std::min(value.size(), timestampInput_.size() - 1U));
    }

    ImGui::InputText("Unix timestamp", timestampInput_.data(), timestampInput_.size());

    if (ImGui::Button("Convert")) {
        long long parsed = 0;
        const std::string_view input(timestampInput_.data());
        const auto result = std::from_chars(input.data(), input.data() + input.size(), parsed);

        if (result.ec == std::errc {} && result.ptr == input.data() + input.size()) {
            result_ = formatTimestampResult(static_cast<std::time_t>(parsed));
        } else {
            result_ = "Invalid timestamp";
        }
    }

    if (!result_.empty()) {
        ImGui::Separator();
        ImGui::TextUnformatted(result_.c_str());
    }
}

} // namespace tools::timestamp
