#pragma once

#include <atomic>
#include <filesystem>
#include <functional>
#include <string>
#include <thread>

namespace pie::io {

using FsWatchCallback = std::function<void(const std::filesystem::path& changed)>;

class FsWatcher {
public:
    // Watch a directory for changes, debounced at 200ms
    explicit FsWatcher(const std::filesystem::path& dir, FsWatchCallback callback);
    ~FsWatcher();

    FsWatcher(const FsWatcher&) = delete;
    FsWatcher& operator=(const FsWatcher&) = delete;

    void stop();

private:
    void watch_loop();

    std::filesystem::path dir_;
    FsWatchCallback callback_;
    std::atomic<bool> running_{true};
    std::thread thread_;
    int inotify_fd_ = -1;
};

}  // namespace pie::io
