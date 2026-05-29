#include "pie/wire/diff.hpp"
#include <dtl/dtl.hpp>
#include <sstream>
#include <vector>

namespace pie::wire {

static std::vector<std::string> split_lines(const std::string& text) {
    std::vector<std::string> lines;
    std::istringstream stream(text);
    std::string line;
    while (std::getline(stream, line)) {
        lines.push_back(line);
    }
    return lines;
}

std::string Diff::unified_diff(
    const std::string& old_text,
    const std::string& new_text,
    [[maybe_unused]] const std::string& old_label,
    [[maybe_unused]] const std::string& new_label) {

    auto old_lines = split_lines(old_text);
    auto new_lines = split_lines(new_text);

    dtl::Diff<std::string> diff(old_lines, new_lines);
    diff.compose();
    diff.composeUnifiedDiff();
    return diff.getUnifiedDiffString();
}

}  // namespace pie::wire
