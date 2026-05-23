#pragma once

#include <array>
#include <string>

namespace tools::color {

class ColorTool {
public:
    void draw();

private:
    void updateFromHex();
    void updateFromRgb();

    std::array<char, 16> hexInput_ { '#', '3', 'b', '8', '2', 'f', '6', '\0' };
    int red_ = 59;
    int green_ = 130;
    int blue_ = 246;
    std::string output_;
    std::string status_;
};

} // namespace tools::color
