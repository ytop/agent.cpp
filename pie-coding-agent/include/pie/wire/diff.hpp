#pragma once

#include <string>

namespace pie::wire {

class Diff {
public:
    // Produce a unified-diff string from two text inputs
    static std::string unified_diff(
        const std::string& old_text,
        const std::string& new_text,
        const std::string& old_label = "a",
        const std::string& new_label = "b");
};

}  // namespace pie::wire
