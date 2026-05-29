#include "pie/io/file_lock.hpp"

#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

#include <chrono>
#include <cstring>
#include <thread>

namespace pie::io {

FileLock::FileLock(std::filesystem::path lock_path)
    : lock_path_(std::move(lock_path)), held_(true) {}

FileLock::~FileLock() { release(); }

FileLock::FileLock(FileLock&& other) noexcept
    : lock_path_(std::move(other.lock_path_)), held_(other.held_) {
    other.held_ = false;
}

FileLock& FileLock::operator=(FileLock&& other) noexcept {
    if (this != &other) {
        release();
        lock_path_ = std::move(other.lock_path_);
        held_ = other.held_;
        other.held_ = false;
    }
    return *this;
}

void FileLock::release() {
    if (held_) {
        std::filesystem::remove(lock_path_);
        held_ = false;
    }
}

static bool is_stale(const std::filesystem::path& path) {
    struct stat st{};
    if (stat(path.c_str(), &st) != 0) return false;
    auto age = std::chrono::system_clock::now() -
               std::chrono::system_clock::from_time_t(st.st_mtime);
    return age > std::chrono::seconds(10);
}

pie::Result<FileLock> FileLock::acquire(
    const std::filesystem::path& path,
    std::chrono::milliseconds timeout) {

    auto lock_path = path;
    lock_path += ".lock";

    auto deadline = std::chrono::steady_clock::now() + timeout;

    while (true) {
        int fd = open(lock_path.c_str(), O_CREAT | O_EXCL | O_WRONLY, 0644);
        if (fd >= 0) {
            auto pid_str = std::to_string(getpid()) + "\n";
            [[maybe_unused]] auto _ = write(fd, pid_str.c_str(), pid_str.size());
            close(fd);
            return FileLock(lock_path);
        }

        if (errno == EEXIST) {
            if (is_stale(lock_path)) {
                std::filesystem::remove(lock_path);
                continue;
            }
        } else {
            return std::unexpected("FileLock: " + std::string(strerror(errno)));
        }

        if (std::chrono::steady_clock::now() >= deadline) {
            return std::unexpected("FileLock: timeout acquiring " + lock_path.string());
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
}

}  // namespace pie::io
