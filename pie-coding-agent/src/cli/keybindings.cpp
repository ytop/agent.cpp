#include "pie/cli/keybindings.hpp"
#include <iostream>
#include <map>
#include <set>

namespace pie::cli {

KeybindingRegistry& KeybindingRegistry::instance() {
    static KeybindingRegistry reg;
    return reg;
}

void KeybindingRegistry::load_defaults() {
    bindings_["app.clear"] = "Ctrl+C";
    bindings_["app.exit"] = "Ctrl+C";
    bindings_["app.interrupt"] = "Esc";
    bindings_["doubleEscapeAction"] = "tree";
    bindings_["app.model.select"] = "Ctrl+L";
    bindings_["app.model.cycleForward"] = "Ctrl+P";
    bindings_["app.model.cycleBackward"] = "Shift+Ctrl+P";
    bindings_["app.thinking.cycle"] = "Shift+Tab";
    bindings_["app.tools.expand"] = "Ctrl+O";
    bindings_["app.thinking.toggle"] = "Ctrl+T";
    bindings_["editor.cursor.up"] = "Up";
    bindings_["editor.cursor.down"] = "Down";
}

void KeybindingRegistry::load_from_json(const core::JsonValue& json, const std::string& source_path) {
    if (!json.is_object()) {
        std::cerr << "error: [" << source_path << "] keybindings file must be a JSON object.\n";
        return;
    }

    std::map<std::string, std::string> migration_map = {
        {"cursorUp", "editor.cursor.up"},
        {"cursorDown", "editor.cursor.down"},
        {"expandTools", "app.tools.expand"},
        {"toggleThinking", "app.thinking.toggle"},
        {"cycleThinking", "app.thinking.cycle"},
        {"cycleModel", "app.model.cycleForward"},
        {"cycleModelBack", "app.model.cycleBackward"},
        {"selectModel", "app.model.select"}
    };

    std::set<std::string> valid_ids = {
        "app.clear", "app.exit", "app.interrupt", "doubleEscapeAction",
        "app.model.select", "app.model.cycleForward", "app.model.cycleBackward",
        "app.thinking.cycle", "app.tools.expand", "app.thinking.toggle",
        "editor.cursor.up", "editor.cursor.down"
    };

    for (auto it = json.begin(); it != json.end(); ++it) {
        std::string raw_id = it.key();
        std::string resolved_id = raw_id;

        // Apply legacy name migration
        auto mig_it = migration_map.find(raw_id);
        if (mig_it != migration_map.end()) {
            resolved_id = mig_it->second;
        }

        // Validate action ID
        if (valid_ids.find(resolved_id) == valid_ids.end()) {
            std::cerr << "error: [" << source_path << "] undefined keybinding action '" << raw_id << "'\n";
            continue;
        }

        // Validate and read key value (string or array of strings)
        std::string val;
        if (it.value().is_string()) {
            val = it.value().get<std::string>();
        } else if (it.value().is_array()) {
            bool all_strings = true;
            std::string combined;
            for (const auto& item : it.value()) {
                if (!item.is_string()) {
                    all_strings = false;
                    break;
                }
                if (!combined.empty()) combined += ",";
                combined += item.get<std::string>();
            }
            if (!all_strings) {
                std::cerr << "error: [" << source_path << "] keybinding for '" << raw_id << "' must be a string or array of strings.\n";
                continue;
            }
            val = combined;
        } else {
            std::cerr << "error: [" << source_path << "] keybinding for '" << raw_id << "' must be a string or array of strings.\n";
            continue;
        }

        bindings_[resolved_id] = val;
    }
}

std::string KeybindingRegistry::get_key(const std::string& action_id) const {
    auto it = bindings_.find(action_id);
    if (it != bindings_.end()) {
        return it->second;
    }
    return "";
}

} // namespace pie::cli
