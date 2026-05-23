#pragma once

#include <array>
#include <string>

namespace tools::text_diff {

class TextDiffTool {
public:
    void draw();

private:
    void compare();

    std::array<char, 32768> leftInput_ {};
    std::array<char, 32768> rightInput_ {};
    std::string diffOutput_;
    std::string status_;
};

} // namespace tools::text_diff
