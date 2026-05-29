#include "pie/resources/theme_manager.hpp"
#include <fstream>

namespace pie::resources {

namespace fs = std::filesystem;

static const core::JsonValue kDarkTheme = core::JsonValue::parse(R"({
    "background": "#1e1e2e",
    "foreground": "#cdd6f4",
    "accent": "#89b4fa",
    "error": "#f38ba8",
    "warning": "#fab387",
    "success": "#a6e3a1"
})");

static const core::JsonValue kLightTheme = core::JsonValue::parse(R"({
    "background": "#eff1f5",
    "foreground": "#4c4f69",
    "accent": "#1e66f5",
    "error": "#d20f39",
    "warning": "#df8e1d",
    "success": "#40a02b"
})");

ThemeManager::ThemeManager(const fs::path& agent_dir, const fs::path& cwd) {
    theme_dirs_.push_back(cwd / ".pie" / "themes");
    theme_dirs_.push_back(agent_dir / "themes");

    // Register built-in themes
    themes_["dark"] = {"dark", kDarkTheme, "", true};
    themes_["light"] = {"light", kLightTheme, "", true};
    active_ = themes_["dark"];
}

void ThemeManager::discover() {
    for (const auto& dir : theme_dirs_) {
        if (!fs::is_directory(dir)) continue;
        for (auto& entry : fs::directory_iterator(dir)) {
            if (entry.path().extension() != ".json") continue;
            auto name = entry.path().stem().string();
            if (themes_.contains(name)) continue;  // first-wins

            std::ifstream f(entry.path());
            if (!f) continue;
            try {
                auto colors = core::JsonValue::parse(f);
                themes_[name] = {name, colors, entry.path(), false};
            } catch (...) {}
        }
    }
}

std::vector<std::string> ThemeManager::list() const {
    std::vector<std::string> names;
    for (const auto& [name, _] : themes_) names.push_back(name);
    return names;
}

std::optional<Theme> ThemeManager::get(const std::string& name) const {
    auto it = themes_.find(name);
    if (it == themes_.end()) return std::nullopt;
    return it->second;
}

Result<void> ThemeManager::set_active(const std::string& name) {
    auto it = themes_.find(name);
    if (it == themes_.end()) return std::unexpected("unknown theme: " + name);
    active_ = it->second;
    for (auto& cb : callbacks_) cb(active_);
    return {};
}

void ThemeManager::reload(const fs::path& changed_path) {
    auto name = changed_path.stem().string();
    std::ifstream f(changed_path);
    if (!f) return;  // Retain previous on failure

    try {
        auto colors = core::JsonValue::parse(f);
        themes_[name] = {name, colors, changed_path, false};
        if (active_.name == name) {
            active_ = themes_[name];
            for (auto& cb : callbacks_) cb(active_);
        }
    } catch (...) {
        // Retain previous theme on reload failure
    }
}

}  // namespace pie::resources
