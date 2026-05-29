#include "pie/resources/resource_loader.hpp"
#include <algorithm>
#include <fstream>
#include <sstream>

namespace pie::resources {

namespace fs = std::filesystem;

ResourceLoader::ResourceLoader(const fs::path& agent_dir, const fs::path& cwd)
    : agent_dir_(agent_dir), cwd_(cwd) {}

std::string ResourceLoader::read_file(const fs::path& path) const {
    std::ifstream f(path);
    if (!f) return "";
    return std::string((std::istreambuf_iterator<char>(f)), {});
}

std::vector<fs::path> ResourceLoader::discover_context_files() const {
    std::vector<fs::path> files;
    static const char* names[] = {"AGENTS.md", "CLAUDE.md"};

    // 1. Agent dir
    for (auto name : names) {
        auto p = agent_dir_ / name;
        if (fs::exists(p)) files.push_back(p);
    }

    // 2. Walk ancestors from cwd to root
    auto dir = cwd_;
    while (true) {
        for (auto name : names) {
            auto p = dir / name;
            if (fs::exists(p) && std::find(files.begin(), files.end(), p) == files.end())
                files.push_back(p);
        }
        auto parent = dir.parent_path();
        if (parent == dir) break;
        dir = parent;
    }

    return files;
}

std::string ResourceLoader::load_context_files() const {
    auto files = discover_context_files();
    std::string result;
    for (const auto& f : files) {
        if (!result.empty()) result += "\n";
        result += read_file(f);
    }
    return result;
}

std::string ResourceLoader::load_system_prompt(
    const std::string& cli_override,
    const std::string& cli_append) const {

    if (!cli_override.empty()) {
        auto result = cli_override;
        if (!cli_append.empty()) result += "\n" + cli_append;
        return result;
    }

    // Look for SYSTEM.md: project (.pie/SYSTEM.md) > global (agent_dir/SYSTEM.md)
    std::string system_prompt;
    auto project_system = cwd_ / ".pie" / "SYSTEM.md";
    auto global_system = agent_dir_ / "SYSTEM.md";

    if (fs::exists(project_system)) system_prompt = read_file(project_system);
    else if (fs::exists(global_system)) system_prompt = read_file(global_system);

    // APPEND_SYSTEM.md
    auto project_append = cwd_ / ".pie" / "APPEND_SYSTEM.md";
    auto global_append = agent_dir_ / "APPEND_SYSTEM.md";

    std::string append;
    if (fs::exists(project_append)) append = read_file(project_append);
    else if (fs::exists(global_append)) append = read_file(global_append);

    if (!cli_append.empty()) append = cli_append;

    if (!append.empty() && !system_prompt.empty()) system_prompt += "\n" + append;
    else if (!append.empty()) system_prompt = append;

    return system_prompt;
}

}  // namespace pie::resources
