#pragma once

#include "pie/core/json.hpp"
#include "pie/core/result.hpp"
#include "pie/agent/events.hpp"
#include "pie/agent/agent.hpp"
#include "pie/models/model_registry.hpp"
#include "pie/session/session_manager.hpp"
#include "pie/settings/settings_manager.hpp"
#include "pie/auth/auth_storage.hpp"
#include "pie/resources/resource_loader.hpp"
#include "pie/tools/tool_host.hpp"

#include <asio.hpp>
#include <filesystem>
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <vector>
#include <optional>

namespace pie {

struct PromptOptions {};
struct SteerOptions {};
struct FollowUpOptions {};

struct ModelCycleResult {
    models::ModelInfo model;
};

struct NavigateTreeOptions {
    bool summarize = false;
    std::optional<std::string> custom_instructions;
    bool replace_instructions = false;
    std::optional<std::string> label;
};

struct NavigateTreeResult {
    std::optional<std::string> editor_text;
    bool cancelled = false;
};

class AgentSession : public std::enable_shared_from_this<AgentSession> {
public:
    AgentSession(std::shared_ptr<Agent> agent, std::shared_ptr<session::SessionManager> session_mgr);
    ~AgentSession() = default;

    // Core Loop & Input
    asio::awaitable<Result<void>> prompt(std::string text, PromptOptions opts = {});
    asio::awaitable<Result<void>> steer(std::string text, SteerOptions opts = {});
    asio::awaitable<Result<void>> follow_up(std::string text, FollowUpOptions opts = {});

    // Event Subscriptions
    [[nodiscard]] Subscription subscribe(std::function<void(const AgentSessionEvent&)> cb);

    // Model & Thinking Controls
    asio::awaitable<Result<void>> set_model(const models::ModelInfo& model);
    void set_thinking_level(ThinkingLevel level);
    asio::awaitable<Result<std::optional<ModelCycleResult>>> cycle_model();
    std::optional<ThinkingLevel> cycle_thinking_level();

    // Compaction Subsystem
    asio::awaitable<Result<core::JsonValue>> compact(std::optional<std::string> custom_instructions = {});
    void abort_compaction();

    // Cancellation
    asio::awaitable<void> abort();

    // Resource Disposal
    void dispose();

    // Tree Navigation
    asio::awaitable<Result<NavigateTreeResult>> navigate_tree(std::string target, NavigateTreeOptions opts = {});

    // State Accessors
    Agent& agent() noexcept;
    std::optional<models::ModelInfo> model() const;
    ThinkingLevel thinking_level() const;
    std::vector<core::JsonValue> messages() const;
    bool is_streaming() const;
    std::optional<std::filesystem::path> session_file() const;
    std::string session_id() const;

private:
    void publish_event(const AgentSessionEvent& ev);

    std::shared_ptr<Agent> agent_;
    std::shared_ptr<session::SessionManager> session_manager_;
    std::string session_id_;
    
    mutable std::mutex subscribers_mu_;
    std::vector<std::pair<uint64_t, std::function<void(const AgentSessionEvent&)>>> subscribers_;
    uint64_t next_sub_id_ = 1;

    std::optional<models::ModelInfo> model_;
    ThinkingLevel thinking_level_ = ThinkingLevel::Medium;
    bool is_streaming_ = false;
    bool is_disposed_ = false;
    bool abort_compaction_ = false;
};

} // namespace pie
