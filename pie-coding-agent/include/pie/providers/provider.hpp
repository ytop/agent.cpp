#pragma once

#include "pie/core/json.hpp"
#include "pie/core/result.hpp"
#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace pie::providers {

enum class EventKind { TextDelta, ThinkingDelta, ToolCallStart, ToolCallDelta, ToolCallEnd, Done, Error };

struct StreamEvent {
    EventKind kind;
    std::string data;
    std::string tool_call_id;
    std::string tool_name;
};

using StreamCallback = std::function<void(const StreamEvent&)>;

struct ProviderRequest {
    std::string model;
    std::vector<core::JsonValue> messages;
    std::vector<core::JsonValue> tools;
    int max_tokens = 4096;
    bool thinking = false;
    int thinking_budget = 0;
};

struct ProviderError {
    enum Kind { Generic, Overflow, RateLimit, Auth } kind = Generic;
    std::string message;
};

class Provider {
public:
    virtual ~Provider() = default;
    virtual std::string id() const = 0;
    virtual Result<void, ProviderError> stream(const ProviderRequest& req, StreamCallback cb) = 0;
};

// Factory for all providers
std::unique_ptr<Provider> create_provider(
    const std::string& provider_id,
    const std::string& api_key,
    const std::string& endpoint = "");

}  // namespace pie::providers
