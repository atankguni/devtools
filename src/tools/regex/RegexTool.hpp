#pragma once

#include <array>
#include <string>

namespace tools::regex_tool {

class RegexTool {
public:
    void draw();

private:
    void run();

    std::array<char, 4096> pattern_ {};
    std::array<char, 32768> text_ {};
    bool caseInsensitive_ = false;
    std::string output_;
    std::string status_;
};

} // namespace tools::regex_tool
