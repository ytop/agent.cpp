#include "pie/io/subprocess.hpp"

#include <spawn.h>
#include <sys/wait.h>
#include <unistd.h>

#include <cstring>

extern char** environ;

namespace pie::io {

pie::Result<SubprocessResult> Subprocess::run(
    const std::vector<std::string>& argv,
    const std::string& cwd) {

    if (argv.empty()) return std::unexpected("empty argv");

    int pipefd[2];
    if (pipe(pipefd) != 0)
        return std::unexpected("pipe: " + std::string(strerror(errno)));

    posix_spawn_file_actions_t actions;
    posix_spawn_file_actions_init(&actions);
    posix_spawn_file_actions_adddup2(&actions, pipefd[1], STDOUT_FILENO);
    posix_spawn_file_actions_adddup2(&actions, pipefd[1], STDERR_FILENO);
    posix_spawn_file_actions_addclose(&actions, pipefd[0]);
    posix_spawn_file_actions_addclose(&actions, pipefd[1]);

    if (!cwd.empty()) {
        posix_spawn_file_actions_addchdir_np(&actions, cwd.c_str());
    }

    std::vector<char*> c_argv;
    for (const auto& a : argv) c_argv.push_back(const_cast<char*>(a.c_str()));
    c_argv.push_back(nullptr);

    pid_t pid;
    int rc = posix_spawnp(&pid, c_argv[0], &actions, nullptr, c_argv.data(), environ);
    posix_spawn_file_actions_destroy(&actions);
    close(pipefd[1]);

    if (rc != 0) {
        close(pipefd[0]);
        return std::unexpected("posix_spawnp: " + std::string(strerror(rc)));
    }

    std::string output;
    char buf[4096];
    ssize_t n;
    while ((n = read(pipefd[0], buf, sizeof(buf))) > 0) {
        output.append(buf, static_cast<size_t>(n));
    }
    close(pipefd[0]);

    int status;
    waitpid(pid, &status, 0);
    int exit_code = WIFEXITED(status) ? WEXITSTATUS(status) : 128 + WTERMSIG(status);

    return SubprocessResult{exit_code, std::move(output)};
}

pie::Result<SubprocessResult> Subprocess::shell(
    const std::string& command,
    const std::string& cwd) {
    return run({"/bin/sh", "-c", command}, cwd);
}

}  // namespace pie::io
