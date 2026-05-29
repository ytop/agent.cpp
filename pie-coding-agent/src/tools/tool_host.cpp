#include "pie/tools/tool_host.hpp"

namespace pie::tools {

void ToolHost::register_tool(std::shared_ptr<Tool> tool) {
    tools_.push_back(std::move(tool));
}

bool ToolHost::is_allowed(const std::string& name) const {
    if (allowlist_.no_tools) return false;
    if (allowlist_.no_builtin_tools) return false;
    if (!allowlist_.allowed.empty()) return allowlist_.allowed.contains(name);
    return true;
}

Tool* ToolHost::find(const std::string& name) const {
    for (auto& t : tools_) {
        if (t->name() == name && is_allowed(name)) return t.get();
    }
    return nullptr;
}

std::vector<Tool*> ToolHost::all_allowed() const {
    std::vector<Tool*> result;
    for (auto& t : tools_) {
        if (is_allowed(t->name())) result.push_back(t.get());
    }
    return result;
}

ToolAllowlist ToolHost::build_allowlist(
    bool no_tools, bool no_builtin_tools,
    const std::vector<std::string>& tools_flag) {
    ToolAllowlist list;
    list.no_tools = no_tools;
    list.no_builtin_tools = no_builtin_tools;
    for (const auto& t : tools_flag) list.allowed.insert(t);
    return list;
}

}  // namespace pie::tools
