#include "pie/io/fs_watcher.hpp"

#include <sys/inotify.h>
#include <poll.h>
#include <unistd.h>

#include <chrono>
#include <cstring>

namespace pie::io {

FsWatcher::FsWatcher(const std::filesystem::path& dir, FsWatchCallback callback)
    : dir_(dir), callback_(std::move(callback)) {
    inotify_fd_ = inotify_init1(IN_NONBLOCK);
    if (inotify_fd_ >= 0) {
        inotify_add_watch(inotify_fd_, dir_.c_str(),
                          IN_MODIFY | IN_CREATE | IN_DELETE | IN_MOVED_TO);
        thread_ = std::thread(&FsWatcher::watch_loop, this);
    }
}

FsWatcher::~FsWatcher() { stop(); }

void FsWatcher::stop() {
    running_ = false;
    if (thread_.joinable()) thread_.join();
    if (inotify_fd_ >= 0) { close(inotify_fd_); inotify_fd_ = -1; }
}

void FsWatcher::watch_loop() {
    constexpr auto debounce = std::chrono::milliseconds(200);
    alignas(inotify_event) char buf[4096];

    while (running_) {
        pollfd pfd{inotify_fd_, POLLIN, 0};
        int ret = poll(&pfd, 1, 100);  // 100ms poll timeout
        if (ret <= 0) continue;

        ssize_t len = read(inotify_fd_, buf, sizeof(buf));
        if (len <= 0) continue;

        // Debounce: collect the last filename, wait, then fire
        std::string last_name;
        for (char* ptr = buf; ptr < buf + len;) {
            auto* event = reinterpret_cast<inotify_event*>(ptr);
            if (event->len > 0) last_name = event->name;
            ptr += sizeof(inotify_event) + event->len;
        }

        std::this_thread::sleep_for(debounce);
        if (!last_name.empty() && running_) {
            callback_(dir_ / last_name);
        }
    }
}

}  // namespace pie::io
