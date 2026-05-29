#pragma once

#include "pie/core/result.hpp"
#include <chrono>
#include <filesystem>
#include <string>

namespace pie::io {

class FileLock {
public:
    // Acquire lock on <path>.lock, retrying every 50ms up to timeout.
    // Stale locks (>10s old) are removed automatically.
    static pie::Result<FileLock> acquire(
        const std::filesystem::path& path,
        std::chrono::milliseconds timeout = std::chrono::seconds(10));

    ~FileLock();
    FileLock(FileLock&& other) noexcept;
    FileLock& operator=(FileLock&& other) noexcept;
    FileLock(const FileLock&) = delete;
    FileLock& operator=(const FileLock&) = delete;

    void release();
    const std::filesystem::path& lock_path() const { return lock_path_; }

private:
    explicit FileLock(std::filesystem::path lock_path);
    std::filesystem::path lock_path_;
    bool held_ = false;
};

}  // namespace pie::io
