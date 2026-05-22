#include "core/ToolRegistry.hpp"

#include <algorithm>
#include <utility>

namespace core {

void ToolRegistry::add(Tool tool)
{
    tools_.push_back(std::move(tool));
}

std::span<const Tool> ToolRegistry::tools() const
{
    return tools_;
}

const Tool* ToolRegistry::findById(const std::string& id) const
{
    const auto it = std::ranges::find_if(tools_, [&](const Tool& tool) {
        return tool.id == id;
    });

    if (it == tools_.end()) {
        return nullptr;
    }

    return &*it;
}

} // namespace core
