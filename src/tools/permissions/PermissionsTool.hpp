#pragma once

#include <array>
#include <string>

namespace tools::permissions {

class PermissionsTool {
public:
    void draw();

private:
    std::array<char, 8> octalInput_ { '7', '5', '5', '\0' };
    bool bits_[9] { true, true, true, true, false, true, true, false, true };
    std::string symbolic_ = "rwxr-xr-x";
    std::string status_;
};

} // namespace tools::permissions
