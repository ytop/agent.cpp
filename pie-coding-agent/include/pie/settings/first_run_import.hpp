#pragma once

#include "pie/core/result.hpp"
#include <filesystem>

namespace pie::settings {

class FirstRunImport {
public:
    // Import from ~/.pi/agent to agent_dir if agent_dir is empty
    static Result<bool> run(
        const std::filesystem::path& agent_dir,
        const std::filesystem::path& ts_agent_dir = "");
};

}  // namespace pie::settings
