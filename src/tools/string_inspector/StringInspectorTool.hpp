#pragma once

#include <array>
#include <string>

namespace tools::string_inspector {

class StringInspectorTool {
public:
    void draw();

private:
    void inspect();

    std::array<char, 65536> input_ {};
    std::string summary_;
    std::string escaped_;
    std::string codepoints_;
    std::string status_;
};

} // namespace tools::string_inspector
