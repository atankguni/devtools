#pragma once

#include <array>
#include <string>

namespace tools::line_tools {

class LineToolsTool {
public:
    void draw();

private:
    void transform();

    std::array<char, 65536> input_ {};
    bool trimLines_ = false;
    bool removeEmpty_ = false;
    bool dedupe_ = true;
    bool descending_ = false;
    bool caseSensitive_ = true;
    std::string output_;
    std::string status_;
};

} // namespace tools::line_tools
