#include "pie/agent/agent_session.hpp"
#include "pie/providers/provider.hpp"
#include "pie/core/utils.hpp"
#include "pie/compaction/compactor.hpp"
#include <algorithm>
#include <stdexcept>

namespace pie {

AgentSession::AgentSession(std::shared_ptr<Agent> agent, std::shared_ptr<session::SessionManager> session_mgr)
    : agent_(std::move(agent)), session_manager_(std::move(session_mgr)) {
    if (session_manager_) {
        session_id_ = session_manager_->session_id();
    } else {
        session_id_ = core::uuid_v4();
    }
}

asio::awaitable<Result<void>> AgentSession::prompt(std::string text, PromptOptions opts) {
    if (is_disposed_) {
        co_return std::unexpected("session is disposed");
    }

    is_streaming_ = true;
    publish_event({{"type", "session_start"}, {"sessionId", session_id_}});

    // Append the user message
    session_manager_->append_user_message(text);
    publish_event({{"type", "message_received"}, {"role", "user"}, {"content", text}});

    // 1. Resolve current model
    models::ModelInfo model_info;
    if (model_) {
        model_info = *model_;
    } else {
        // Fallback or registry default
        if (agent_ && agent_->model_registry) {
            auto models = agent_->model_registry->all();
            if (!models.empty()) {
                model_info = models[0];
            } else {
                model_info = models::ModelInfo{"claude-sonnet-4-20250514", "anthropic", "Claude 3.5 Sonnet", 200000, true};
            }
        } else {
            model_info = models::ModelInfo{"claude-sonnet-4-20250514", "anthropic", "Claude 3.5 Sonnet", 200000, true};
        }
    }

    // 2. Resolve credentials
    std::string api_key;
    if (agent_ && agent_->auth_storage) {
        auto key_opt = agent_->auth_storage->resolve_api_key(model_info.provider);
        if (key_opt) {
            api_key = *key_opt;
        }
    }

    // 3. Create LLM provider client
    auto provider = providers::create_provider(model_info.provider, api_key);
    if (!provider) {
        is_streaming_ = false;
        publish_event({{"type", "session_error"}, {"error", "Failed to create LLM provider client"}});
        co_return std::unexpected("failed to create provider client");
    }

    // 4. Construct request
    providers::ProviderRequest req;
    req.model = model_info.id;
    
    // Walk messages and add to context
    auto context = session_manager_->build_session_context();
    req.messages = context;
    req.max_tokens = 4096;
    req.thinking = (thinking_level_ != ThinkingLevel::Off) && model_info.supports_thinking;
    req.thinking_budget = (thinking_level_ == ThinkingLevel::High) ? 8192 : 2048;

    publish_event({{"type", "agent_start"}, {"model", model_info.id}});

    // 5. Stream response
    std::string full_response;
    std::string thinking_content;
    
    providers::StreamCallback stream_cb = [&](const providers::StreamEvent& ev) {
        if (ev.kind == providers::EventKind::TextDelta) {
            full_response += ev.data;
            publish_event({{"type", "message_delta"}, {"role", "assistant"}, {"content", ev.data}});
        } else if (ev.kind == providers::EventKind::ThinkingDelta) {
            thinking_content += ev.data;
            publish_event({{"type", "thinking_delta"}, {"content", ev.data}});
        } else if (ev.kind == providers::EventKind::Done) {
            publish_event({{"type", "message_done"}});
        } else if (ev.kind == providers::EventKind::Error) {
            publish_event({{"type", "message_error"}, {"error", ev.data}});
        }
    };

    auto stream_res = provider->stream(req, stream_cb);
    if (!stream_res) {
        is_streaming_ = false;
        publish_event({{"type", "agent_end"}, {"status", "error"}, {"error", stream_res.error().message}});
        co_return std::unexpected("LLM streaming failed: " + stream_res.error().message);
    }

    session_manager_->append_assistant_message(full_response);
    publish_event({{"type", "agent_end"}, {"status", "success"}});

    is_streaming_ = false;
    co_return Result<void>{};
}

asio::awaitable<Result<void>> AgentSession::steer(std::string text, SteerOptions opts) {
    if (is_disposed_) {
        co_return std::unexpected("session is disposed");
    }
    // Stub or simple steering behavior
    co_return Result<void>{};
}

asio::awaitable<Result<void>> AgentSession::follow_up(std::string text, FollowUpOptions opts) {
    if (is_disposed_) {
        co_return std::unexpected("session is disposed");
    }
    co_return Result<void>{};
}

Subscription AgentSession::subscribe(std::function<void(const AgentSessionEvent&)> cb) {
    std::lock_guard<std::mutex> lock(subscribers_mu_);
    uint64_t sub_id = next_sub_id_++;
    subscribers_.push_back({sub_id, std::move(cb)});

    return Subscription([this, sub_id]() {
        std::lock_guard<std::mutex> lock(this->subscribers_mu_);
        this->subscribers_.erase(
            std::remove_if(this->subscribers_.begin(), this->subscribers_.end(),
                           [sub_id](const auto& pair) { return pair.first == sub_id; }),
            this->subscribers_.end());
    });
}

asio::awaitable<Result<void>> AgentSession::set_model(const models::ModelInfo& model) {
    if (is_disposed_) {
        co_return std::unexpected("session is disposed");
    }
    model_ = model;
    co_return Result<void>{};
}

void AgentSession::set_thinking_level(ThinkingLevel level) {
    if (is_disposed_) return;
    thinking_level_ = level;
}

asio::awaitable<Result<std::optional<ModelCycleResult>>> AgentSession::cycle_model() {
    if (is_disposed_) {
        co_return std::unexpected("session is disposed");
    }
    if (!agent_ || !agent_->model_registry) {
        co_return std::nullopt;
    }
    auto all_models = agent_->model_registry->all();
    if (all_models.empty()) {
        co_return std::nullopt;
    }

    size_t next_idx = 0;
    if (model_) {
        for (size_t i = 0; i < all_models.size(); ++i) {
            if (all_models[i].id == model_->id) {
                next_idx = (i + 1) % all_models.size();
                break;
            }
        }
    }
    model_ = all_models[next_idx];
    co_return ModelCycleResult{*model_};
}

std::optional<ThinkingLevel> AgentSession::cycle_thinking_level() {
    if (is_disposed_) return std::nullopt;
    switch (thinking_level_) {
        case ThinkingLevel::Off:    thinking_level_ = ThinkingLevel::Low; break;
        case ThinkingLevel::Low:    thinking_level_ = ThinkingLevel::Medium; break;
        case ThinkingLevel::Medium: thinking_level_ = ThinkingLevel::High; break;
        case ThinkingLevel::High:   thinking_level_ = ThinkingLevel::Off; break;
    }
    return thinking_level_;
}

asio::awaitable<Result<core::JsonValue>> AgentSession::compact(std::optional<std::string> custom_instructions) {
    if (is_disposed_) {
        co_return std::unexpected("session is disposed");
    }

    publish_event({{"type", "compaction_start"}, {"reason", "manual"}});

    compaction::Compactor compactor;
    compaction::CompactionRequest req;
    req.reason = compaction::CompactionReason::Manual;
    req.messages = session_manager_->build_session_context();
    req.summarize_fn = [](const std::vector<core::JsonValue>&) -> Result<std::string> {
        return std::string("Compacted conversation history summary");
    };

    if (abort_compaction_) {
        abort_compaction_ = false;
        publish_event({{"type", "compaction_end"}, {"aborted", true}});
        co_return std::unexpected("compaction aborted");
    }

    auto comp_res = compactor.run(req);
    if (!comp_res) {
        publish_event({{"type", "compaction_end"}, {"aborted", true}, {"error_message", comp_res.error()}});
        co_return std::unexpected("compaction failed: " + comp_res.error());
    }

    core::JsonValue result = {
        {"summary", comp_res->summary},
        {"cutPointIndex", comp_res->cut_point_index},
        {"aborted", comp_res->aborted}
    };

    publish_event({{"type", "compaction_end"}, {"aborted", false}, {"summary", comp_res->summary}});
    co_return result;
}

void AgentSession::abort_compaction() {
    abort_compaction_ = true;
}

asio::awaitable<void> AgentSession::abort() {
    abort_compaction_ = true;
    co_return;
}

void AgentSession::dispose() {
    is_disposed_ = true;
    std::lock_guard<std::mutex> lock(subscribers_mu_);
    subscribers_.clear();
}

asio::awaitable<Result<NavigateTreeResult>> AgentSession::navigate_tree(std::string target, NavigateTreeOptions opts) {
    if (is_disposed_) {
        co_return std::unexpected("session is disposed");
    }

    session_manager_->tree().set_leaf(target);
    NavigateTreeResult res;
    res.editor_text = "Navigated to entry: " + target;
    res.cancelled = false;
    co_return res;
}

Agent& AgentSession::agent() noexcept {
    return *agent_;
}

std::optional<models::ModelInfo> AgentSession::model() const {
    return model_;
}

ThinkingLevel AgentSession::thinking_level() const {
    return thinking_level_;
}

std::vector<core::JsonValue> AgentSession::messages() const {
    if (session_manager_) {
        return session_manager_->build_session_context();
    }
    return {};
}

bool AgentSession::is_streaming() const {
    return is_streaming_;
}

std::optional<std::filesystem::path> AgentSession::session_file() const {
    if (session_manager_) {
        return session_manager_->path();
    }
    return std::nullopt;
}

std::string AgentSession::session_id() const {
    return session_id_;
}

void AgentSession::publish_event(const AgentSessionEvent& ev) {
    std::vector<std::function<void(const AgentSessionEvent&)>> active_subs;
    {
        std::lock_guard<std::mutex> lock(subscribers_mu_);
        for (const auto& pair : subscribers_) {
            active_subs.push_back(pair.second);
        }
    }
    for (const auto& cb : active_subs) {
        try {
            cb(ev);
        } catch (...) {}
    }
}

} // namespace pie
