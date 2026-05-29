#pragma once

#include "pie/tools/tool_host.hpp"
#include <memory>
#include <vector>

namespace pie::tools {

// Register all built-in tools with the host
void register_builtin_tools(ToolHost& host);

// Individual tool factories
std::shared_ptr<Tool> make_read_tool();
std::shared_ptr<Tool> make_write_tool();
std::shared_ptr<Tool> make_edit_tool();
std::shared_ptr<Tool> make_ls_tool();
std::shared_ptr<Tool> make_grep_tool();
std::shared_ptr<Tool> make_find_tool();

}  // namespace pie::tools
