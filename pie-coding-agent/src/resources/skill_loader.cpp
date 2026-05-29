#include "pie/resources/skill_loader.hpp"
#include "pie/wire/yaml_frontmatter.hpp"
#include <fstream>

namespace pie::resources {

namespace fs = std::filesystem;

SkillLoader::SkillLoader(const fs::path& agent_dir, const fs::path& cwd) {
    // Seven-source discovery order (simplified)
    skill_dirs_.push_back(cwd / ".pie" / "skills");
    skill_dirs_.push_back(cwd / ".kiro" / "skills");
    skill_dirs_.push_back(agent_dir / "skills");
}

std::vector<Skill> SkillLoader::discover() const {
    if (cached_) return cache_;

    std::map<std::string, bool> seen;
    for (const auto& dir : skill_dirs_) {
        if (!fs::is_directory(dir)) continue;
        for (auto& entry : fs::directory_iterator(dir)) {
            if (!entry.is_directory()) continue;
            auto skill_file = entry.path() / "SKILL.md";
            if (!fs::exists(skill_file)) continue;

            auto name = entry.path().filename().string();
            if (seen.contains(name)) continue;  // first-wins
            seen[name] = true;

            std::ifstream f(skill_file);
            std::string content((std::istreambuf_iterator<char>(f)), {});

            Skill skill{name, "", content, false, skill_file};
            auto fm = wire::YamlFrontmatter::parse(content);
            if (fm) {
                skill.description = fm->metadata.value("description", "");
                skill.disable_model_invocation = fm->metadata.value("disable-model-invocation", false);
                skill.body = fm->body;
            }

            // Skip skills without description
            if (skill.description.empty()) continue;

            cache_.push_back(std::move(skill));
        }
    }
    cached_ = true;
    return cache_;
}

std::optional<Skill> SkillLoader::find(const std::string& name) const {
    discover();
    for (const auto& s : cache_) {
        if (s.name == name) return s;
    }
    return std::nullopt;
}

Result<std::string> SkillLoader::invoke(const std::string& name, const std::string& args) const {
    if (!enabled_) return std::unexpected("skills are disabled");
    auto skill = find(name);
    if (!skill) return std::unexpected("unknown skill: " + name);
    auto result = skill->body;
    if (!args.empty()) result += "\n\nArguments: " + args;
    return result;
}

}  // namespace pie::resources
