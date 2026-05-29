#pragma once

#include <filesystem>
#include <string>
#include <vector>

namespace pie::wire {

class Globber {
public:
    // Match files against a glob pattern with ** support.
    // Returns matching paths relative to root.
    static std::vector<std::filesystem::path> glob(
        const std::filesystem::path& root,
        const std::string& pattern);

    // Test if a path matches a glob pattern (supports *, ?, **)
    static bool matches(const std::string& path, const std::string& pattern);
};

}  // namespace pie::wire
