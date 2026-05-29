#pragma once

#include "pie/core/result.hpp"
#include <string>
#include <vector>

namespace pie::io {

struct SubprocessResult {
    int exit_code;
    std::string output;  // combined stdout+stderr
};

class Subprocess {
public:
    // Spawn a process with argv, capture combined stdout+stderr
    static pie::Result<SubprocessResult> run(
        const std::vector<std::string>& argv,
        const std::string& cwd = "");

    // Spawn with a shell command string via /bin/sh -c
    static pie::Result<SubprocessResult> shell(
        const std::string& command,
        const std::string& cwd = "");
};

}  // namespace pie::io
