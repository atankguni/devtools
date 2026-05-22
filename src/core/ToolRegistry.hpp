#pragma once

#include "core/Tool.hpp"

#include <span>
#include <string>
#include <vector>

namespace core {

class ToolRegistry {
public:
    void add(Tool tool);

    [[nodiscard]] std::span<const Tool> tools() const;
    [[nodiscard]] const Tool* findById(const std::string& id) const;

private:
    std::vector<Tool> tools_;
};

} // namespace core
