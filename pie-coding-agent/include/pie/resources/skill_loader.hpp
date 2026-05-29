#pragma once

#include "pie/core/json.hpp"
#include "pie/core/result.hpp"
#include <filesystem>
#include <map>
#include <string>
#include <vector>

namespace pie::resources {

struct Skill {
    std::string name;
    std::string description;
    std::string body;
    bool disable_model_invocation = false;
    std::filesystem::path source;
};

class SkillLoader {
public:
    SkillLoader(const std::filesystem::path& agent_dir,
                const std::filesystem::path& cwd);

    std::vector<Skill> discover() const;
    std::optional<Skill> find(const std::string& name) const;

    // Inject skill into prompt: /skill:<name> [args]
    Result<std::string> invoke(const std::string& name, const std::string& args) const;

    void set_enabled(bool enabled) { enabled_ = enabled; }
    bool enabled() const { return enabled_; }

private:
    std::vector<std::filesystem::path> skill_dirs_;
    mutable std::vector<Skill> cache_;
    mutable bool cached_ = false;
    bool enabled_ = true;
};

}  // namespace pie::resources
