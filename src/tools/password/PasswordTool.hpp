#pragma once

#include <array>
#include <string>
#include <vector>

namespace tools::password {

class PasswordTool {
public:
    void draw();

private:
    enum class Mode {
        Generator,
        Checker,
    };

    struct StrengthResult {
        int score = 0;
        std::string label;
        std::vector<std::string> hints;
    };

    void drawGenerator();
    void drawChecker();
    void generatePassword();
    [[nodiscard]] StrengthResult checkStrength(std::string_view password) const;

    Mode mode_ = Mode::Generator;
    int length_ = 20;
    bool includeSymbols_ = true;
    bool includeNumbers_ = true;
    bool includeLowercase_ = true;
    bool includeUppercase_ = true;
    std::string generatedPassword_;
    std::array<char, 32768> checkerInput_ {};
    std::string status_;
};

} // namespace tools::password
