#pragma once

#include <array>
#include <string>

namespace tools::timestamp {

class TimestampTool {
public:
    void draw();

private:
    std::array<char, 64> timestampInput_ {};
    std::string result_;
};

} // namespace tools::timestamp
