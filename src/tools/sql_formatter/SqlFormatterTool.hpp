#pragma once

#include <array>
#include <string>

namespace tools::sql_formatter {

class SqlFormatterTool {
public:
    void draw();

private:
    std::array<char, 65536> input_ {};
    int indentWidth_ = 2;
    std::string output_;
    std::string status_;
};

} // namespace tools::sql_formatter
