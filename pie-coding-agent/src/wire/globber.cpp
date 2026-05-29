#include "pie/wire/globber.hpp"
#include <fnmatch.h>

namespace pie::wire {

// Recursive match supporting ** (matches zero or more path segments)
bool Globber::matches(const std::string& path, const std::string& pattern) {
    // Handle ** by splitting pattern at ** and matching segments
    auto dstar = pattern.find("**");
    if (dstar == std::string::npos) {
        return fnmatch(pattern.c_str(), path.c_str(), FNM_PATHNAME) == 0;
    }

    std::string prefix = pattern.substr(0, dstar);
    std::string suffix = pattern.substr(dstar + 2);
    if (!suffix.empty() && suffix[0] == '/') suffix = suffix.substr(1);

    // ** matches zero or more path segments
    if (prefix.empty() && suffix.empty()) return true;

    if (suffix.empty()) {
        return prefix.empty() ||
               fnmatch(prefix.c_str(), path.c_str(), FNM_PATHNAME | FNM_LEADING_DIR) == 0;
    }

    // Try matching suffix against every possible tail of path
    for (size_t i = 0; i <= path.size(); ++i) {
        if (i > 0 && path[i - 1] != '/') continue;
        std::string tail = path.substr(i);
        if (matches(tail, suffix)) {
            if (prefix.empty()) return true;
            std::string head = (i > 0) ? path.substr(0, i - 1) : "";
            if (prefix.empty() || prefix == "**/") return true;
            std::string pfx = prefix;
            if (!pfx.empty() && pfx.back() == '/') pfx.pop_back();
            if (pfx.empty() || fnmatch(pfx.c_str(), head.c_str(), FNM_PATHNAME) == 0)
                return true;
        }
    }
    return false;
}

std::vector<std::filesystem::path> Globber::glob(
    const std::filesystem::path& root,
    const std::string& pattern) {

    std::vector<std::filesystem::path> results;
    if (!std::filesystem::is_directory(root)) return results;

    for (auto& entry : std::filesystem::recursive_directory_iterator(
             root, std::filesystem::directory_options::skip_permission_denied)) {
        auto rel = std::filesystem::relative(entry.path(), root).string();
        if (matches(rel, pattern)) {
            results.push_back(rel);
        }
    }
    return results;
}

}  // namespace pie::wire
