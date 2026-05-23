#pragma once

#include <array>
#include <string>

namespace tools::case_converter {

class CaseConverterTool {
public:
    void draw();

private:
    void convert();

    std::array<char, 32768> input_ {};
    std::string output_;
    std::string status_;
};

} // namespace tools::case_converter
