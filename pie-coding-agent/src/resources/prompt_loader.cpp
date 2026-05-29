#include "pie/resources/prompt_loader.hpp"
#include "pie/wire/yaml_frontmatter.hpp"
#include <fstream>
#include <map>

namespace pie::resources {

namespace fs = std::filesystem;

PromptLoader::PromptLoader(const fs::path& agent_dir, const fs::path& cwd) {
    // Project > user > package precedence
    prompt_dirs_.push_back(cwd / ".pie" / "prompts");
    prompt_dirs_.push_back(agent_dir / "prompts");
}

std::vector<PromptTemplate> PromptLoader::discover() const {
    std::vector<PromptTemplate> templates;
    std::map<std::string, bool> seen;

    for (const auto& dir : prompt_dirs_) {
        if (!fs::is_directory(dir)) continue;
        for (auto& entry : fs::directory_iterator(dir)) {
            if (entry.path().extension() != ".md") continue;
            auto name = entry.path().stem().string();
            if (seen.contains(name)) continue;
            seen[name] = true;

            std::ifstream f(entry.path());
            std::string content((std::istreambuf_iterator<char>(f)), {});

            PromptTemplate tmpl{name, "", "", content, entry.path()};
            auto fm = wire::YamlFrontmatter::parse(content);
            if (fm) {
                tmpl.description = fm->metadata.value("description", "");
                tmpl.argument_hint = fm->metadata.value("argument-hint", "");
                tmpl.body = fm->body;
            }
            templates.push_back(std::move(tmpl));
        }
    }
    return templates;
}

Result<std::string> PromptLoader::substitute(
    const std::string& body,
    const std::map<std::string, std::string>& vars) {

    std::string result = body;
    size_t pos = 0;
    while ((pos = result.find("{{", pos)) != std::string::npos) {
        auto end = result.find("}}", pos);
        if (end == std::string::npos) break;
        auto key = result.substr(pos + 2, end - pos - 2);
        auto it = vars.find(key);
        if (it == vars.end())
            return std::unexpected("missing variable: " + key);
        result.replace(pos, end - pos + 2, it->second);
        pos += it->second.size();
    }
    return result;
}

}  // namespace pie::resources
