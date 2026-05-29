#include "pie/compaction/compactor.hpp"
#include <set>

namespace pie::compaction {

static const std::set<std::string> kValidCutTypes = {"message", "bashExecution", "custom"};

int Compactor::find_cut_point(const std::vector<core::JsonValue>& messages) {
    // Walk backwards to find last valid cut point (at least 1 message retained)
    for (int i = static_cast<int>(messages.size()) - 2; i >= 1; --i) {
        auto& msg = messages[i];
        if (!msg.contains("type")) continue;
        auto type = msg["type"].get<std::string>();
        if (kValidCutTypes.contains(type)) return i;
    }
    return 0;
}

Result<CompactionResult> Compactor::run(const CompactionRequest& req) {
    if (req.messages.size() < 2)
        return CompactionResult{"", 0, true};

    int cut = find_cut_point(req.messages);
    if (cut == 0) return CompactionResult{"", 0, true};

    // Get messages to summarize (everything before cut point)
    std::vector<core::JsonValue> to_summarize(req.messages.begin(), req.messages.begin() + cut);

    auto summary = req.summarize_fn(to_summarize);
    if (!summary) {
        // Failure: no entry appended, aborted
        return CompactionResult{"", 0, true};
    }

    return CompactionResult{*summary, cut, false};
}

Result<std::string> Compactor::summarize_branch(
    const std::vector<core::JsonValue>& messages,
    std::function<Result<std::string>(const std::vector<core::JsonValue>&)> summarize_fn) {
    return summarize_fn(messages);
}

}  // namespace pie::compaction
