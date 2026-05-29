#pragma once

#include "pie/core/json.hpp"
#include "pie/core/result.hpp"
#include <filesystem>
#include <string>
#include <vector>

namespace pie::resources {

struct PromptTemplate {
    std::string name;
    std::string description;
    std::string argument_hint;
    std::string body;
    std::filesystem::path source;
};

class PromptLoader {
public:
    PromptLoader(const std::filesystem::path& agent_dir,
                 const std::filesystem::path& cwd);

    std::vector<PromptTemplate> discover() const;

    // Substitute {{variable}} in body. Returns error if variable missing.
    static Result<std::string> substitute(
        const std::string& body,
        const std::map<std::string, std::string>& vars);

private:
    std::vector<std::filesystem::path> prompt_dirs_;
};

}  // namespace pie::resources
