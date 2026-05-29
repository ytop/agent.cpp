#pragma once

#include "pie/core/json.hpp"
#include "pie/core/result.hpp"
#include <optional>
#include <string>
#include <vector>

namespace pie::models {

struct ModelInfo {
    std::string id;
    std::string provider;
    std::string name;
    int context_window = 0;
    bool supports_thinking = false;
};

struct ModelSelector {
    std::string provider;  // optional
    std::string model_id;
    bool thinking = false;
};

class ModelRegistry {
public:
    static ModelRegistry create();

    void load_models_json(const core::JsonValue& models_json);
    std::optional<ModelInfo> find(const std::string& id) const;
    std::vector<ModelInfo> all() const { return models_; }
    std::vector<ModelInfo> get_available(const std::vector<std::string>& valid_providers) const;

    static std::optional<ModelSelector> parse_selector(const std::string& selector);

private:
    std::vector<ModelInfo> models_;
};

}  // namespace pie::models
