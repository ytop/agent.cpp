#include "pie/providers/provider.hpp"
#include "pie/io/http_client.hpp"

namespace pie::providers {

// --- Base HTTP provider with SSE streaming ---
class HttpProvider : public Provider {
public:
    HttpProvider(std::string id, std::string api_key, std::string endpoint)
        : id_(std::move(id)), api_key_(std::move(api_key)), endpoint_(std::move(endpoint)) {}

    std::string id() const override { return id_; }

protected:
    std::string id_, api_key_, endpoint_;
};

// --- Anthropic ---
class AnthropicProvider : public HttpProvider {
public:
    using HttpProvider::HttpProvider;
    Result<void, ProviderError> stream(const ProviderRequest& req, StreamCallback cb) override {
        io::HttpClient http;
        core::JsonValue body = {{"model", req.model}, {"max_tokens", req.max_tokens}, {"stream", true}};
        core::JsonValue msgs = core::JsonValue::array();
        for (auto& m : req.messages) msgs.push_back(m);
        body["messages"] = msgs;
        if (req.thinking && req.thinking_budget > 0)
            body["thinking"] = {{"type", "enabled"}, {"budget_tokens", req.thinking_budget}};

        auto result = http.post_sse(endpoint_ + "/v1/messages", body.dump(),
            {{"x-api-key", api_key_}, {"anthropic-version", "2023-06-01"}, {"content-type", "application/json"}},
            [&](const std::string& event, const std::string& data) {
                if (event == "content_block_delta") {
                    auto j = core::JsonValue::parse(data);
                    auto type = j.value("delta", core::JsonValue::object()).value("type", "");
                    if (type == "text_delta")
                        cb({EventKind::TextDelta, j["delta"].value("text", ""), "", ""});
                    else if (type == "thinking_delta")
                        cb({EventKind::ThinkingDelta, j["delta"].value("thinking", ""), "", ""});
                } else if (event == "message_stop") {
                    cb({EventKind::Done, "", "", ""});
                } else if (event == "error") {
                    cb({EventKind::Error, data, "", ""});
                }
            });
        if (!result) return std::unexpected(ProviderError{ProviderError::Generic, result.error()});
        return {};
    }
};

// --- OpenAI-compatible provider ---
class OpenAIProvider : public HttpProvider {
public:
    using HttpProvider::HttpProvider;
    Result<void, ProviderError> stream(const ProviderRequest& req, StreamCallback cb) override {
        io::HttpClient http;
        core::JsonValue body = {{"model", req.model}, {"max_tokens", req.max_tokens}, {"stream", true}};
        core::JsonValue msgs = core::JsonValue::array();
        for (auto& m : req.messages) msgs.push_back(m);
        body["messages"] = msgs;

        auto result = http.post_sse(endpoint_ + "/v1/chat/completions", body.dump(),
            {{"Authorization", "Bearer " + api_key_}, {"Content-Type", "application/json"}},
            [&](const std::string&, const std::string& data) {
                if (data == "[DONE]") { cb({EventKind::Done, "", "", ""}); return; }
                try {
                    auto j = core::JsonValue::parse(data);
                    auto& choices = j["choices"];
                    if (!choices.empty()) {
                        auto& delta = choices[0]["delta"];
                        if (delta.contains("content"))
                            cb({EventKind::TextDelta, delta["content"].get<std::string>(), "", ""});
                    }
                } catch (...) {}
            });
        if (!result) return std::unexpected(ProviderError{ProviderError::Generic, result.error()});
        return {};
    }
};

// --- Factory ---
std::unique_ptr<Provider> create_provider(
    const std::string& provider_id,
    const std::string& api_key,
    const std::string& endpoint) {

    if (provider_id == "anthropic")
        return std::make_unique<AnthropicProvider>(provider_id, api_key,
            endpoint.empty() ? "https://api.anthropic.com" : endpoint);

    // All OpenAI-compatible providers
    static const std::map<std::string, std::string> openai_endpoints = {
        {"openai", "https://api.openai.com"},
        {"openai-codex", "https://api.openai.com"},
        {"azure-openai", ""},
        {"deepseek", "https://api.deepseek.com"},
        {"google-gemini", "https://generativelanguage.googleapis.com"},
        {"mistral", "https://api.mistral.ai"},
        {"groq", "https://api.groq.com/openai"},
        {"cerebras", "https://api.cerebras.ai"},
        {"xai", "https://api.x.ai"},
        {"openrouter", "https://openrouter.ai/api"},
        {"huggingface", "https://api-inference.huggingface.co"},
        {"fireworks", "https://api.fireworks.ai/inference"},
        {"together", "https://api.together.xyz"},
        {"kimi", "https://api.moonshot.cn"},
        {"minimax", "https://api.minimax.chat"},
    };

    auto ep = endpoint;
    if (ep.empty()) {
        auto it = openai_endpoints.find(provider_id);
        if (it != openai_endpoints.end()) ep = it->second;
    }
    return std::make_unique<OpenAIProvider>(provider_id, api_key, ep);
}

}  // namespace pie::providers
