#pragma once

#include "pie/core/json.hpp"
#include "pie/core/result.hpp"
#include <string>

namespace pie::wire {

struct FrontmatterResult {
    core::JsonValue metadata;  // key-value pairs from YAML frontmatter
    std::string body;          // content after the closing ---
};

class YamlFrontmatter {
public:
    // Parse YAML frontmatter delimited by --- lines from a Markdown string.
    // Returns metadata as JsonValue object + remaining body.
    static pie::Result<FrontmatterResult> parse(const std::string& content);
};

}  // namespace pie::wire
