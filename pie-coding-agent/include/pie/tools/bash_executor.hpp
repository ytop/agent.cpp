#pragma once

#include "pie/core/result.hpp"
#include <cstdint>
#include <string>

namespace pie::tools {

struct BashRequest {
    std::string command;
    std::string shell_path = "/bin/sh";
    std::string shell_prefix = "-c";
    int timeout_ms = 0;  // 0 = no timeout
    bool exclude_from_context = false;  // !! prefix
};

struct BashResult {
    int exit_code;
    std::string output;
    std::string overflow_file;  // non-empty if truncated
    bool truncated = false;
    bool exclude_from_context = false;
};

class BashExecutor {
public:
    static constexpr size_t kTruncationThreshold = 32 * 1024;  // 32 KiB

    Result<BashResult> run(const BashRequest& req);
};

}  // namespace pie::tools
