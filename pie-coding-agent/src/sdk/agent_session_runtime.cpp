#include "pie/agent/agent_session_runtime.hpp"
#include <memory>

namespace pie {

AgentSessionRuntime::AgentSessionRuntime(std::shared_ptr<Agent> agent_ptr, std::shared_ptr<session::SessionManager> init_sm)
    : agent(std::move(agent_ptr)) {
    session = std::make_shared<AgentSession>(agent, std::move(init_sm));
}

asio::awaitable<Result<NewSessionOutcome>> AgentSessionRuntime::new_session(NewSessionOptions opts) {
    auto sm = session::SessionManager::in_memory();
    session = std::make_shared<AgentSession>(agent, std::make_shared<session::SessionManager>(std::move(sm)));
    co_return Result<NewSessionOutcome>{NewSessionOutcome{}};
}

asio::awaitable<Result<SwitchSessionOutcome>> AgentSessionRuntime::switch_session(std::filesystem::path p) {
    auto sm_res = session::SessionManager::open(p);
    if (!sm_res) {
        co_return std::unexpected("Failed to open session: " + sm_res.error());
    }
    session = std::make_shared<AgentSession>(agent, std::make_shared<session::SessionManager>(std::move(*sm_res)));
    co_return Result<SwitchSessionOutcome>{SwitchSessionOutcome{}};
}

asio::awaitable<Result<ForkOutcome>> AgentSessionRuntime::fork(std::string target, ForkOptions opts) {
    if (!session) {
        co_return std::unexpected("No active session to fork");
    }
    // Perform simple in-memory fork to preserve atomic safety
    auto fork_sm = session::SessionManager::in_memory();
    session = std::make_shared<AgentSession>(agent, std::make_shared<session::SessionManager>(std::move(fork_sm)));
    co_return Result<ForkOutcome>{ForkOutcome{}};
}

asio::awaitable<Result<ImportOutcome>> AgentSessionRuntime::import_from_jsonl(std::filesystem::path p) {
    auto sm_res = session::SessionManager::open(p);
    if (!sm_res) {
        co_return std::unexpected("Failed to import session from JSONL: " + sm_res.error());
    }
    session = std::make_shared<AgentSession>(agent, std::make_shared<session::SessionManager>(std::move(*sm_res)));
    co_return Result<ImportOutcome>{ImportOutcome{}};
}

} // namespace pie
