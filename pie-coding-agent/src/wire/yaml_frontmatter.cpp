#include "pie/wire/yaml_frontmatter.hpp"
#include <sstream>

namespace pie::wire {

static std::string trim(const std::string& s) {
    auto start = s.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) return "";
    auto end = s.find_last_not_of(" \t\r\n");
    return s.substr(start, end - start + 1);
}

pie::Result<FrontmatterResult> YamlFrontmatter::parse(const std::string& content) {
    // Must start with ---
    if (!content.starts_with("---")) {
        return std::unexpected("no frontmatter delimiter found");
    }

    // Find closing ---
    auto close_pos = content.find("\n---", 3);
    if (close_pos == std::string::npos) {
        return std::unexpected("no closing frontmatter delimiter");
    }

    std::string yaml_block = content.substr(4, close_pos - 4);  // skip "---\n"
    std::string body;
    auto body_start = content.find('\n', close_pos + 1);
    if (body_start != std::string::npos) {
        body = content.substr(body_start + 1);
    }

    // Parse simple key: value pairs
    core::JsonValue metadata = core::JsonValue::object();
    std::istringstream stream(yaml_block);
    std::string line;

    while (std::getline(stream, line)) {
        if (line.empty() || line[0] == '#') continue;
        auto colon = line.find(':');
        if (colon == std::string::npos) continue;

        std::string key = trim(line.substr(0, colon));
        std::string val = trim(line.substr(colon + 1));

        // Remove surrounding quotes if present
        if (val.size() >= 2 &&
            ((val.front() == '"' && val.back() == '"') ||
             (val.front() == '\'' && val.back() == '\''))) {
            val = val.substr(1, val.size() - 2);
        }

        if (val == "true") metadata[key] = true;
        else if (val == "false") metadata[key] = false;
        else metadata[key] = val;
    }

    return FrontmatterResult{std::move(metadata), std::move(body)};
}

}  // namespace pie::wire
