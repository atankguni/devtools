#pragma once

#include <functional>
#include <string>

namespace core {

struct Tool {
    std::string id;
    std::string name;
    std::string description;
    std::function<void()> draw;
};

} // namespace core
