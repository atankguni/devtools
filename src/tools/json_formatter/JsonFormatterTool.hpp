#pragma once

#include <array>
#include <string>

namespace tools::json_formatter {

class JsonFormatterTool {
public:
    void draw();

private:
    std::array<char, 65536> input_ {};
    std::string output_;
    std::string status_;
};

} // namespace tools::json_formatter
