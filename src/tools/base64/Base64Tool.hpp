#pragma once

#include <array>
#include <string>

namespace tools::base64 {

class Base64Tool {
public:
    void draw();

private:
    std::array<char, 32768> input_ {};
    std::string output_;
    std::string status_;
    bool statusIsError_ = false;
};

} // namespace tools::base64
