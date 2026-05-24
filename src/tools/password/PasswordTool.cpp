#include "tools/password/PasswordTool.hpp"

#include "ui/Clipboard.hpp"

#include <imgui.h>

#include <algorithm>
#include <cctype>
#include <cmath>
#include <iomanip>
#include <limits>
#include <random>
#include <sstream>
#include <string_view>

namespace {

constexpr std::string_view lowercase = "abcdefghijklmnopqrstuvwxyz";
constexpr std::string_view uppercase = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
constexpr std::string_view numbers = "0123456789";
constexpr std::string_view symbols = "!@#$%^&*()-_=+[]{};:,.<>/?";

bool containsAny(std::string_view value, std::string_view candidates)
{
    return std::ranges::any_of(value, [&](char ch) {
        return candidates.find(ch) != std::string_view::npos;
    });
}

std::string pickCharacters(std::mt19937_64& engine, std::string_view candidates, std::size_t count)
{
    std::string value;
    value.reserve(count);
    std::uniform_int_distribution<std::size_t> distribution(0U, candidates.size() - 1U);
    for (std::size_t index = 0; index < count; ++index) {
        value += candidates[distribution(engine)];
    }
    return value;
}

std::size_t estimateCharacterPoolSize(std::string_view password)
{
    std::size_t poolSize = 0;
    if (containsAny(password, lowercase)) {
        poolSize += lowercase.size();
    }
    if (containsAny(password, uppercase)) {
        poolSize += uppercase.size();
    }
    if (containsAny(password, numbers)) {
        poolSize += numbers.size();
    }
    if (containsAny(password, symbols)) {
        poolSize += symbols.size();
    }
    return poolSize;
}

std::string formatDuration(double seconds)
{
    if (!std::isfinite(seconds) || seconds > 365.25 * 24.0 * 60.0 * 60.0 * 1.0e12) {
        return "more than 1 trillion years";
    }
    if (seconds < 1.0e-3) {
        return "less than 1 millisecond";
    }

    struct Unit {
        const char* singular;
        const char* plural;
        double seconds;
    };

    constexpr Unit units[] {
        { "year", "years", 365.25 * 24.0 * 60.0 * 60.0 },
        { "day", "days", 24.0 * 60.0 * 60.0 },
        { "hour", "hours", 60.0 * 60.0 },
        { "minute", "minutes", 60.0 },
        { "second", "seconds", 1.0 },
    };

    for (const Unit& unit : units) {
        if (seconds >= unit.seconds) {
            const double value = seconds / unit.seconds;
            std::ostringstream stream;
            if (value >= 100.0) {
                stream << std::fixed << std::setprecision(0);
            } else if (value >= 10.0) {
                stream << std::fixed << std::setprecision(1);
            } else {
                stream << std::fixed << std::setprecision(2);
            }
            stream << value << ' ' << (value == 1.0 ? unit.singular : unit.plural);
            return stream.str();
        }
    }

    std::ostringstream stream;
    stream << std::fixed << std::setprecision(1) << (seconds * 1000.0) << " milliseconds";
    return stream.str();
}

std::string estimateBreakTime(std::string_view password)
{
    if (password.empty()) {
        return "Generate a password to estimate break time.";
    }

    constexpr double offlineGuessesPerSecond = 1.0e11;
    const std::size_t poolSize = estimateCharacterPoolSize(password);
    if (poolSize == 0U) {
        return "Unable to estimate character space.";
    }

    const double entropyBits = static_cast<double>(password.size()) * std::log2(static_cast<double>(poolSize));
    const double averageSeconds = std::pow(2.0, entropyBits - 1.0) / offlineGuessesPerSecond;

    std::ostringstream stream;
    stream << "Estimated brute-force time: " << formatDuration(averageSeconds)
           << " at 100 billion guesses/sec (" << std::fixed << std::setprecision(1)
           << entropyBits << " bits).";
    return stream.str();
}

void drawCopyableOutput(const char* label, const std::string& value)
{
    ImGui::TextUnformatted(label);
    if (!value.empty()) {
        ImGui::SameLine();
        if (ImGui::Button("Copy")) {
            ui::copyToClipboard(value.c_str());
        }
    }
    ImGui::InputTextMultiline(
        "##PasswordOutput",
        const_cast<char*>(value.c_str()),
        value.size() + 1U,
        ImVec2(-1.0F, ImGui::GetTextLineHeight() * 3.0F),
        ImGuiInputTextFlags_ReadOnly
    );
}

} // namespace

namespace tools::password {

void PasswordTool::draw()
{
    int selectedMode = static_cast<int>(mode_);
    if (ImGui::RadioButton("Generator", selectedMode == static_cast<int>(Mode::Generator))) {
        mode_ = Mode::Generator;
    }
    ImGui::SameLine();
    if (ImGui::RadioButton("Checker", selectedMode == static_cast<int>(Mode::Checker))) {
        mode_ = Mode::Checker;
    }

    ImGui::Separator();

    if (mode_ == Mode::Generator) {
        drawGenerator();
    } else {
        drawChecker();
    }
}

void PasswordTool::drawGenerator()
{
    ImGui::SliderInt("Length", &length_, 4, 128);
    ImGui::Checkbox("Symbols", &includeSymbols_);
    ImGui::SameLine();
    ImGui::Checkbox("Numbers", &includeNumbers_);
    ImGui::SameLine();
    ImGui::Checkbox("Lowercase", &includeLowercase_);
    ImGui::SameLine();
    ImGui::Checkbox("Uppercase", &includeUppercase_);

    if (ImGui::Button("Generate Password")) {
        generatePassword();
    }
    ImGui::SameLine();
    if (ImGui::Button("Clear")) {
        generatedPassword_.clear();
        status_.clear();
    }

    if (!status_.empty()) {
        ImGui::SameLine();
        ImGui::TextDisabled("%s", status_.c_str());
    }

    if (!generatedPassword_.empty()) {
        ImGui::Separator();
        drawCopyableOutput("Generated password", generatedPassword_);
        const StrengthResult result = checkStrength(generatedPassword_);
        ImGui::Text("Strength: %s (%d/100)", result.label.c_str(), result.score);
        ImGui::ProgressBar(static_cast<float>(result.score) / 100.0F, ImVec2(-1.0F, 0.0F));
        ImGui::TextWrapped("%s", estimateBreakTime(generatedPassword_).c_str());
    }
}

void PasswordTool::drawChecker()
{
    ImGui::TextUnformatted("Password");
    ImGui::InputTextMultiline(
        "##PasswordCheckerInput",
        checkerInput_.data(),
        checkerInput_.size(),
        ImVec2(-1.0F, ImGui::GetTextLineHeight() * 5.0F),
        ImGuiInputTextFlags_Password
    );

    const std::string_view password(checkerInput_.data());
    const StrengthResult result = checkStrength(password);
    ImGui::Text("Strength: %s (%d/100)", result.label.c_str(), result.score);
    ImGui::ProgressBar(static_cast<float>(result.score) / 100.0F, ImVec2(-1.0F, 0.0F));

    if (!password.empty() && !result.hints.empty()) {
        ImGui::Spacing();
        ImGui::TextDisabled("Suggestions");
        for (const std::string& hint : result.hints) {
            ImGui::BulletText("%s", hint.c_str());
        }
    }
}

void PasswordTool::generatePassword()
{
    std::vector<std::string_view> selectedSets;
    if (includeSymbols_) {
        selectedSets.push_back(symbols);
    }
    if (includeNumbers_) {
        selectedSets.push_back(numbers);
    }
    if (includeLowercase_) {
        selectedSets.push_back(lowercase);
    }
    if (includeUppercase_) {
        selectedSets.push_back(uppercase);
    }

    if (selectedSets.empty()) {
        status_ = "Select at least one character type";
        generatedPassword_.clear();
        return;
    }

    if (length_ < static_cast<int>(selectedSets.size())) {
        length_ = static_cast<int>(selectedSets.size());
    }

    static thread_local std::mt19937_64 engine(std::random_device {}());
    std::string pool;
    generatedPassword_.clear();
    generatedPassword_.reserve(static_cast<std::size_t>(length_));

    for (std::string_view set : selectedSets) {
        generatedPassword_ += pickCharacters(engine, set, 1U);
        pool.append(set);
    }

    generatedPassword_ += pickCharacters(
        engine,
        pool,
        static_cast<std::size_t>(length_ - static_cast<int>(generatedPassword_.size()))
    );

    std::ranges::shuffle(generatedPassword_, engine);
    status_ = "Generated";
}

PasswordTool::StrengthResult PasswordTool::checkStrength(std::string_view password) const
{
    StrengthResult result;
    if (password.empty()) {
        result.label = "Empty";
        result.hints.push_back("Enter a password to measure strength.");
        return result;
    }

    const bool hasLowercase = containsAny(password, lowercase);
    const bool hasUppercase = containsAny(password, uppercase);
    const bool hasNumber = containsAny(password, numbers);
    const bool hasSymbol = containsAny(password, symbols);

    int score = std::min(50, static_cast<int>(password.size()) * 3);
    score += hasLowercase ? 10 : 0;
    score += hasUppercase ? 10 : 0;
    score += hasNumber ? 12 : 0;
    score += hasSymbol ? 14 : 0;

    int categoryCount = 0;
    categoryCount += hasLowercase ? 1 : 0;
    categoryCount += hasUppercase ? 1 : 0;
    categoryCount += hasNumber ? 1 : 0;
    categoryCount += hasSymbol ? 1 : 0;
    score += categoryCount >= 3 ? 8 : 0;
    score += password.size() >= 16U ? 8 : 0;
    score += password.size() >= 24U ? 8 : 0;

    bool repeatedRun = false;
    for (std::size_t index = 2; index < password.size(); ++index) {
        if (password[index] == password[index - 1U] && password[index] == password[index - 2U]) {
            repeatedRun = true;
            break;
        }
    }

    bool sequentialRun = false;
    for (std::size_t index = 2; index < password.size(); ++index) {
        const int current = static_cast<unsigned char>(std::tolower(static_cast<unsigned char>(password[index])));
        const int previous = static_cast<unsigned char>(std::tolower(static_cast<unsigned char>(password[index - 1U])));
        const int beforePrevious = static_cast<unsigned char>(std::tolower(static_cast<unsigned char>(password[index - 2U])));
        if (beforePrevious + 1 == previous && previous + 1 == current) {
            sequentialRun = true;
            break;
        }
    }

    if (repeatedRun) {
        score -= 15;
        result.hints.push_back("Avoid repeated characters.");
    }
    if (sequentialRun) {
        score -= 12;
        result.hints.push_back("Avoid obvious sequences.");
    }

    if (password.size() < 12U) {
        result.hints.push_back("Use at least 12 characters.");
    }
    if (password.size() < 16U) {
        result.hints.push_back("16 or more characters is better.");
    }
    if (!hasLowercase) {
        result.hints.push_back("Add lowercase letters.");
    }
    if (!hasUppercase) {
        result.hints.push_back("Add uppercase letters.");
    }
    if (!hasNumber) {
        result.hints.push_back("Add numbers.");
    }
    if (!hasSymbol) {
        result.hints.push_back("Add symbols.");
    }

    result.score = std::clamp(score, 0, 100);
    if (result.score >= 85) {
        result.label = "Very strong";
    } else if (result.score >= 70) {
        result.label = "Strong";
    } else if (result.score >= 45) {
        result.label = "Moderate";
    } else if (result.score >= 25) {
        result.label = "Weak";
    } else {
        result.label = "Very weak";
    }

    return result;
}

} // namespace tools::password
