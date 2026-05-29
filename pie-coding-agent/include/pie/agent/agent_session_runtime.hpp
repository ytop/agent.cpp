#pragma once

#include "pie/agent/agent_session.hpp"
#include "pie/agent/agent.hpp"
#include <asio.hpp>
#include <filesystem>
#include <memory>

namespace pie {

struct NewSessionOptions {};
struct ForkOptions {};

struct NewSessionOutcome {};
struct SwitchSessionOutcome {};
struct ForkOutcome {};
struct ImportOutcome {};

class AgentSessionRuntime {
public:
    AgentSessionRuntime(std::shared_ptr<Agent> agent, std::shared_ptr<session::SessionManager> init_sm);
    ~AgentSessionRuntime() = default;

    // Session replacement APIs
    asio::awaitable<Result<NewSessionOutcome>> new_session(NewSessionOptions opts = {});
    asio::awaitable<Result<SwitchSessionOutcome>> switch_session(std::filesystem::path p);
    asio::awaitable<Result<ForkOutcome>> fork(std::string target, ForkOptions opts = {});
    asio::awaitable<Result<ImportOutcome>> import_from_jsonl(std::filesystem::path p);

    std::shared_ptr<AgentSession> session;
    std::shared_ptr<Agent> agent;
};

} // namespace pie
