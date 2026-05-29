#pragma once

#include "pie/core/json.hpp"
#include "pie/core/result.hpp"
#include <functional>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>

namespace pie::tools {

class Tool {
public:
    virtual ~Tool() = default;
    virtual std::string name() const = 0;
    virtual std::string label() const = 0;
    virtual std::string description() const = 0;
    virtual core::JsonValue parameters_schema() const = 0;
    virtual Result<core::JsonValue> execute(const core::JsonValue& params) = 0;
};

struct ToolAllowlist {
    bool no_tools = false;
    bool no_builtin_tools = false;
    std::set<std::string> allowed;  // empty = all allowed
};

class ToolHost {
public:
    void register_tool(std::shared_ptr<Tool> tool);
    void apply_allowlist(const ToolAllowlist& list) { allowlist_ = list; }

    bool is_allowed(const std::string& name) const;
    Tool* find(const std::string& name) const;
    std::vector<Tool*> all_allowed() const;

    // Build allowlist from CLI flags
    static ToolAllowlist build_allowlist(
        bool no_tools, bool no_builtin_tools,
        const std::vector<std::string>& tools_flag);

private:
    std::vector<std::shared_ptr<Tool>> tools_;
    ToolAllowlist allowlist_;
};

}  // namespace pie::tools
