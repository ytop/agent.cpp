#pragma once

#include "pie/core/json.hpp"
#include "pie/core/result.hpp"
#include "pie/tools/tool_host.hpp"
#include <functional>
#include <memory>
#include <string>

namespace pie {

struct ToolDefinition {
    std::string name;
    std::string label;
    std::string description;
    core::JsonValue parameters_schema;
    std::function<Result<core::JsonValue>(const core::JsonValue&)> execute;
};

[[nodiscard]] Result<std::shared_ptr<tools::Tool>> define_tool(ToolDefinition def);

} // namespace pie
