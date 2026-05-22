#pragma once

#include <string>
#include <vector>

namespace tools::uuid {

class UuidTool {
public:
    void draw();

private:
    void generate();

    std::vector<std::string> generated_;
};

} // namespace tools::uuid
