#pragma once

#include "pie/core/json.hpp"
#include "pie/core/result.hpp"
#include "pie/wire/jsonl.hpp"
#include <filesystem>
#include <string>
#include <vector>

namespace pie::session {

class SessionFile {
public:
    static Result<SessionFile> open(const std::filesystem::path& path);
    static Result<SessionFile> create(const std::filesystem::path& path, int version = 3);

    // Atomic crash-safe append: lock → seek-end → write → fsync → unlock
    Result<void> append(const core::JsonValue& entry);

    // Rewrite file (for migration): write to .tmp then rename
    Result<void> rewrite(const std::vector<core::JsonValue>& entries);

    const std::vector<wire::JsonlEntry>& entries() const { return entries_; }
    int version() const { return version_; }
    const std::filesystem::path& path() const { return path_; }

private:
    SessionFile() = default;
    std::filesystem::path path_;
    std::vector<wire::JsonlEntry> entries_;
    int version_ = 3;
};

}  // namespace pie::session
