#pragma once

#include <array>
#include <string>

namespace tools::hash {

class HashTool {
public:
    void draw();

private:
    void generate();

    std::array<char, 32768> input_ {};
    std::string md5_;
    std::string sha1_;
    std::string sha256_;
    std::string sha384_;
    std::string sha512_;
    std::string status_;
};

} // namespace tools::hash
