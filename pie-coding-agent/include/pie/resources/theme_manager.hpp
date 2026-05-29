#pragma once

#include "pie/core/json.hpp"
#include "pie/core/result.hpp"
#include <filesystem>
#include <functional>
#include <map>
#include <string>
#include <vector>

namespace pie::resources {

struct Theme {
    std::string name;
    core::JsonValue colors;
    std::filesystem::path source;
    bool builtin = false;
};

using ThemeChangeCallback = std::function<void(const Theme&)>;

class ThemeManager {
public:
    ThemeManager(const std::filesystem::path& agent_dir,
                 const std::filesystem::path& cwd);

    void discover();
    std::vector<std::string> list() const;
    std::optional<Theme> get(const std::string& name) const;
    Result<void> set_active(const std::string& name);
    const Theme& active() const { return active_; }

    void on_change(ThemeChangeCallback cb) { callbacks_.push_back(std::move(cb)); }

    // Hot reload: call when a theme file changes
    void reload(const std::filesystem::path& changed_path);

private:
    std::map<std::string, Theme> themes_;
    Theme active_;
    std::vector<std::filesystem::path> theme_dirs_;
    std::vector<ThemeChangeCallback> callbacks_;
};

}  // namespace pie::resources
