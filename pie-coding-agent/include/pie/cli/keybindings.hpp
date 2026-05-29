#pragma once

#include "pie/core/json.hpp"
#include <string>
#include <map>
#include <vector>

namespace pie::cli {

class KeybindingRegistry {
public:
    static KeybindingRegistry& instance();

    void load_defaults();
    void load_from_json(const core::JsonValue& json, const std::string& source_path = "keybindings.json");
    std::string get_key(const std::string& action_id) const;
    
    const std::map<std::string, std::string>& bindings() const { return bindings_; }

private:
    KeybindingRegistry() { load_defaults(); }
    std::map<std::string, std::string> bindings_;
};

} // namespace pie::cli
