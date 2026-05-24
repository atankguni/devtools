#pragma once

#include <array>
#include <string>

namespace tools::json_formatter {

class JsonFormatterTool {
public:
    void draw();

private:
    enum class OutputMode {
        TwoSpaces,
        FourSpaces,
        Minified,
    };

    std::array<char, 65536> input_ {};
    OutputMode outputMode_ = OutputMode::TwoSpaces;
    std::string output_;
    std::string status_;
    bool statusIsError_ = false;
};

} // namespace tools::json_formatter
