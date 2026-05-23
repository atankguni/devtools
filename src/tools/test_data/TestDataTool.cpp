#include "tools/test_data/TestDataTool.hpp"

#include "ui/Clipboard.hpp"

#include <imgui.h>

#include <array>
#include <random>
#include <vector>

namespace {

template <std::size_t Size>
const char* pick(std::mt19937_64& engine, const std::array<const char*, Size>& values)
{
    std::uniform_int_distribution<std::size_t> distribution(0U, values.size() - 1U);
    return values[distribution(engine)];
}

} // namespace

namespace tools::test_data {

void TestDataTool::draw()
{
    ImGui::SliderInt("Count", &count_, 1, 50);
    ImGui::Checkbox("JSON objects", &includeJson_);
    ImGui::SameLine();
    ImGui::Checkbox("Emails", &includeEmails_);
    ImGui::SameLine();
    ImGui::Checkbox("Names", &includeNames_);

    if (ImGui::Button("Generate")) {
        generate();
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
        if (ImGui::Button("Copy Output")) {
            ui::copyToClipboard(output_.c_str());
            status_ = "Copied output";
        }
        std::vector<char> buffer(output_.begin(), output_.end());
        buffer.push_back('\0');
        ImGui::InputTextMultiline("##TestDataOutput", buffer.data(), buffer.size(), ImVec2(-1.0F, ImGui::GetTextLineHeight() * 18.0F), ImGuiInputTextFlags_ReadOnly);
    }
}

void TestDataTool::generate()
{
    static thread_local std::mt19937_64 engine(std::random_device {}());
    static constexpr std::array firstNames { "Alex", "Sam", "Taylor", "Jordan", "Casey", "Morgan", "Riley", "Avery" };
    static constexpr std::array lastNames { "Rivera", "Chen", "Patel", "Garcia", "Smith", "Kim", "Brown", "Davis" };
    static constexpr std::array domains { "example.test", "dev.local", "sample.invalid" };

    output_.clear();
    for (int index = 1; index <= count_; ++index) {
        const std::string first = pick(engine, firstNames);
        const std::string last = pick(engine, lastNames);
        std::string email = first + "." + last + std::to_string(index) + "@" + pick(engine, domains);
        for (char& ch : email) {
            ch = static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));
        }

        if (includeJson_) {
            output_ += "{\"id\":" + std::to_string(index) + ",\"name\":\"" + first + " " + last + "\",\"email\":\"" + email + "\"}\n";
        }
        if (includeNames_) {
            output_ += first + " " + last + '\n';
        }
        if (includeEmails_) {
            output_ += email + '\n';
        }
    }
    status_ = "Generated";
}

} // namespace tools::test_data
