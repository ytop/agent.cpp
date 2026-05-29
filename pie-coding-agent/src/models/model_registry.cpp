#include "pie/models/model_registry.hpp"

namespace pie::models {

ModelRegistry ModelRegistry::create() {
    ModelRegistry reg;
    // Built-in models (subset — full list would come from codegen)
    reg.models_ = {
        {"claude-sonnet-4-20250514", "anthropic", "Claude Sonnet 4", 200000, true},
        {"claude-3-5-haiku-20241022", "anthropic", "Claude 3.5 Haiku", 200000, false},
        {"gpt-4o", "openai", "GPT-4o", 128000, false},
        {"gpt-4o-mini", "openai", "GPT-4o Mini", 128000, false},
        {"o3", "openai", "o3", 200000, true},
        {"o4-mini", "openai", "o4-mini", 200000, true},
        {"gemini-2.5-pro", "google-gemini", "Gemini 2.5 Pro", 1000000, true},
        {"gemini-2.5-flash", "google-gemini", "Gemini 2.5 Flash", 1000000, true},
        {"deepseek-chat", "deepseek", "DeepSeek Chat", 64000, false},
        {"deepseek-reasoner", "deepseek", "DeepSeek Reasoner", 64000, true},
    };
    return reg;
}

void ModelRegistry::load_models_json(const core::JsonValue& models_json) {
    if (!models_json.is_array()) return;
    for (const auto& m : models_json) {
        models_.push_back({
            m.value("id", ""),
            m.value("provider", ""),
            m.value("name", ""),
            m.value("contextWindow", 0),
            m.value("supportsThinking", false)
        });
    }
}

std::optional<ModelInfo> ModelRegistry::find(const std::string& id) const {
    for (const auto& m : models_) {
        if (m.id == id) return m;
    }
    return std::nullopt;
}

std::vector<ModelInfo> ModelRegistry::get_available(const std::vector<std::string>& valid_providers) const {
    std::vector<ModelInfo> result;
    for (const auto& m : models_) {
        for (const auto& p : valid_providers) {
            if (m.provider == p) { result.push_back(m); break; }
        }
    }
    return result;
}

std::optional<ModelSelector> ModelRegistry::parse_selector(const std::string& selector) {
    if (selector.empty()) return std::nullopt;

    ModelSelector ms;
    std::string remaining = selector;

    // Check for :thinking suffix
    if (remaining.ends_with(":thinking")) {
        ms.thinking = true;
        remaining = remaining.substr(0, remaining.size() - 9);
    }

    // Check for provider/ prefix
    auto slash = remaining.find('/');
    if (slash != std::string::npos) {
        ms.provider = remaining.substr(0, slash);
        ms.model_id = remaining.substr(slash + 1);
    } else {
        ms.model_id = remaining;
    }

    return ms;
}

}  // namespace pie::models
