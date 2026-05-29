#pragma once

#include "pie/core/json.hpp"
#include "pie/core/result.hpp"
#include <functional>
#include <string>
#include <vector>

namespace pie::compaction {

enum class CompactionReason { Manual, Threshold, Overflow };

struct CompactionRequest {
    CompactionReason reason;
    std::vector<core::JsonValue> messages;
    std::function<Result<std::string>(const std::vector<core::JsonValue>&)> summarize_fn;
};

struct CompactionResult {
    std::string summary;
    int cut_point_index = 0;  // messages before this are compacted
    bool aborted = false;
};

class Compactor {
public:
    Result<CompactionResult> run(const CompactionRequest& req);

    // Summarize a branch for /tree navigation
    Result<std::string> summarize_branch(
        const std::vector<core::JsonValue>& messages,
        std::function<Result<std::string>(const std::vector<core::JsonValue>&)> summarize_fn);

    // Find valid cut point (user/assistant/bashExecution/custom only)
    static int find_cut_point(const std::vector<core::JsonValue>& messages);
};

}  // namespace pie::compaction
