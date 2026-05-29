#pragma once

#include "pie/core/result.hpp"
#include <filesystem>
#include <string>
#include <vector>

namespace pie::resources {

class ResourceLoader {
public:
    ResourceLoader(const std::filesystem::path& agent_dir,
                   const std::filesystem::path& cwd);

    // Discover and concatenate AGENTS.md / CLAUDE.md files
    std::string load_context_files() const;

    // Load SYSTEM.md (project > global), with CLI overrides
    std::string load_system_prompt(
        const std::string& cli_override = "",
        const std::string& cli_append = "") const;

private:
    std::vector<std::filesystem::path> discover_context_files() const;
    std::string read_file(const std::filesystem::path& path) const;

    std::filesystem::path agent_dir_;
    std::filesystem::path cwd_;
};

}  // namespace pie::resources
