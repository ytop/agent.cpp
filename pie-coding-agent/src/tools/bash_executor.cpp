#include "pie/tools/bash_executor.hpp"

#include <chrono>
#include <cstdio>
#include <cstring>
#include <fcntl.h>
#include <poll.h>
#include <signal.h>
#include <spawn.h>
#include <sys/wait.h>
#include <unistd.h>

extern char** environ;

namespace pie::tools {

Result<BashResult> BashExecutor::run(const BashRequest& req) {
    int pipefd[2];
    if (pipe(pipefd) != 0)
        return std::unexpected("pipe: " + std::string(strerror(errno)));

    posix_spawn_file_actions_t actions;
    posix_spawn_file_actions_init(&actions);
    posix_spawn_file_actions_adddup2(&actions, pipefd[1], STDOUT_FILENO);
    posix_spawn_file_actions_adddup2(&actions, pipefd[1], STDERR_FILENO);
    posix_spawn_file_actions_addclose(&actions, pipefd[0]);
    posix_spawn_file_actions_addclose(&actions, pipefd[1]);

    const char* argv[] = {req.shell_path.c_str(), req.shell_prefix.c_str(), req.command.c_str(), nullptr};

    pid_t pid;
    int rc = posix_spawnp(&pid, argv[0], &actions, nullptr, const_cast<char**>(argv), environ);
    posix_spawn_file_actions_destroy(&actions);
    close(pipefd[1]);

    if (rc != 0) {
        close(pipefd[0]);
        return BashResult{127, "", "", false, req.exclude_from_context};
    }

    // Read output with optional timeout
    std::string output;
    char buf[4096];
    auto deadline = req.timeout_ms > 0
        ? std::chrono::steady_clock::now() + std::chrono::milliseconds(req.timeout_ms)
        : std::chrono::steady_clock::time_point::max();

    fcntl(pipefd[0], F_SETFL, O_NONBLOCK);
    bool timed_out = false;

    while (true) {
        int remaining_ms = -1;
        if (req.timeout_ms > 0) {
            auto left = std::chrono::duration_cast<std::chrono::milliseconds>(
                deadline - std::chrono::steady_clock::now()).count();
            if (left <= 0) { timed_out = true; break; }
            remaining_ms = static_cast<int>(left);
        }

        pollfd pfd{pipefd[0], POLLIN, 0};
        int ret = poll(&pfd, 1, std::min(remaining_ms, 100));
        if (ret > 0) {
            ssize_t n = read(pipefd[0], buf, sizeof(buf));
            if (n <= 0) break;
            output.append(buf, static_cast<size_t>(n));
        } else if (ret == 0 && remaining_ms >= 0) {
            continue;
        } else if (ret < 0) {
            break;
        }
    }
    close(pipefd[0]);

    if (timed_out) {
        kill(pid, SIGTERM);
        usleep(100000);  // 100ms grace
        kill(pid, SIGKILL);
    }

    int status;
    waitpid(pid, &status, 0);
    int exit_code = WIFEXITED(status) ? WEXITSTATUS(status) : 128 + WTERMSIG(status);

    // Truncation
    BashResult result{exit_code, "", "", false, req.exclude_from_context};
    if (output.size() > kTruncationThreshold) {
        result.output = output.substr(0, kTruncationThreshold);
        result.truncated = true;
        // Write overflow to temp file
        char tmpl[] = "/tmp/pie_bash_XXXXXX";
        int fd = mkstemp(tmpl);
        if (fd >= 0) {
            auto overflow = output.substr(kTruncationThreshold);
            [[maybe_unused]] auto _ = write(fd, overflow.c_str(), overflow.size());
            close(fd);
            result.overflow_file = tmpl;
        }
    } else {
        result.output = std::move(output);
    }

    return result;
}

}  // namespace pie::tools
