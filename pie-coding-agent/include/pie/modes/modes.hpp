#pragma once

#include "pie/cli/cli_invocation.hpp"
#include "pie/agent/agent_session_runtime.hpp"
#include "pie/agent/agent_session.hpp"
#include <filesystem>
#include <optional>
#include <string>

namespace pie::modes {

class InteractiveMode {
public:
    int run(AgentSessionRuntime& runtime, const cli::CliInvocation& cli);
};

class PrintMode {
public:
    int run(AgentSession& session, const std::string& initial_prompt);
};

class JsonMode {
public:
    int run(AgentSession& session, const std::string& initial_prompt);
};

class RpcMode {
public:
    int run(AgentSessionRuntime& runtime);
};

class ExportMode {
public:
    int run(const std::filesystem::path& in_path, const std::optional<std::filesystem::path>& out_path);
};

} // namespace pie::modes
