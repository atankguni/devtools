#pragma once

#include <array>
#include <string>

namespace tools::number_base {

class NumberBaseTool {
public:
    void draw();

private:
    void updateFromInput(int base, const char* source);
    void setValue(unsigned long long value);

    std::array<char, 128> decimalInput_ { '0' };
    std::array<char, 128> octalInput_ { '0' };
    std::array<char, 128> hexadecimalInput_ { '0' };
    std::array<char, 256> binaryInput_ { '0' };
    std::string status_;
};

} // namespace tools::number_base
