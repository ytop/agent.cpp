#include "pie/settings/settings_manager.hpp"
#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <fstream>

namespace pie::settings {

// --- EnvResolver ---

static const char* getenv_safe(const char* name) {
    const char* v = std::getenv(name);
    return v ? v : "";
}

Tristate EnvResolver::parse_bool(const char* val) {
    if (!val || val[0] == '\0') return Tristate::Unset;
    std::string s(val);
    std::transform(s.begin(), s.end(), s.begin(), ::tolower);
    if (s == "1" || s == "true" || s == "yes" || s == "on") return Tristate::Truthy;
    if (s == "0" || s == "false" || s == "no" || s == "off") return Tristate::Falsy;
    return Tristate::Unset;
}

std::filesystem::path EnvResolver::agent_dir() const {
    auto v = getenv_safe("PIE_CODING_AGENT_DIR");
    if (v[0]) return v;
    auto home = getenv_safe("HOME");
    return std::filesystem::path(home) / ".pie" / "agent";
}

std::filesystem::path EnvResolver::session_dir() const {
    auto v = getenv_safe("PIE_CODING_AGENT_SESSION_DIR");
    if (v[0]) return v;
    return agent_dir() / "sessions";
}

std::filesystem::path EnvResolver::package_dir() const {
    auto v = getenv_safe("PIE_PACKAGE_DIR");
    if (v[0]) return v;
    return agent_dir() / "packages";
}

Tristate EnvResolver::offline() const { return parse_bool(std::getenv("PIE_OFFLINE")); }
Tristate EnvResolver::skip_version_check() const { return parse_bool(std::getenv("PIE_SKIP_VERSION_CHECK")); }
Tristate EnvResolver::telemetry() const { return parse_bool(std::getenv("PIE_TELEMETRY")); }
std::string EnvResolver::cache_retention() const { return getenv_safe("PIE_CACHE_RETENTION"); }

std::string EnvResolver::editor() const {
    auto v = getenv_safe("VISUAL");
    if (v[0]) return v;
    return getenv_safe("EDITOR");
}

std::string EnvResolver::share_viewer_url() const { return getenv_safe("PIE_SHARE_VIEWER_URL"); }

// --- SettingsManager ---

Result<SettingsManager> SettingsManager::create(
    const std::filesystem::path& global_path,
    const std::filesystem::path& project_path) {

    SettingsManager sm;
    sm.global_path_ = global_path;
    sm.project_path_ = project_path;

    if (!global_path.empty() && std::filesystem::exists(global_path))
        sm.load_file(global_path, sm.global_);
    if (!project_path.empty() && std::filesystem::exists(project_path))
        sm.load_file(project_path, sm.project_);

    return sm;
}

SettingsManager SettingsManager::in_memory() { return SettingsManager{}; }

Result<void> SettingsManager::load_file(const std::filesystem::path& path, core::JsonValue& target) {
    std::ifstream f(path);
    if (!f) return std::unexpected("cannot open " + path.string());
    try {
        target = core::JsonValue::parse(f);
    } catch (const std::exception& e) {
        return std::unexpected(std::string("parse error: ") + e.what());
    }
    return {};
}

core::JsonValue SettingsManager::deep_merge(const core::JsonValue& base, const core::JsonValue& overlay) {
    if (!base.is_object() || !overlay.is_object()) return overlay;

    auto result = base;
    for (auto& [key, val] : overlay.items()) {
        if (result.contains(key) && result[key].is_object() && val.is_object()) {
            result[key] = deep_merge(result[key], val);
        } else {
            result[key] = val;
        }
    }
    return result;
}

core::JsonValue SettingsManager::effective() const {
    auto merged = deep_merge(global_, project_);
    return deep_merge(merged, overrides_);
}

core::JsonValue SettingsManager::get(const std::string& key) const {
    auto eff = effective();
    if (eff.contains(key)) return eff[key];
    return nullptr;
}

void SettingsManager::set(const std::string& key, const core::JsonValue& value) {
    project_[key] = value;
}

void SettingsManager::apply_overrides(const core::JsonValue& overrides) {
    overrides_ = deep_merge(overrides_, overrides);
}

Result<void> SettingsManager::flush() {
    if (project_path_.empty()) return {};
    std::filesystem::create_directories(project_path_.parent_path());
    auto tmp = project_path_.string() + ".tmp";
    {
        std::ofstream f(tmp);
        if (!f) return std::unexpected("cannot write " + tmp);
        f << project_.dump(2) << "\n";
    }
    std::filesystem::rename(tmp, project_path_);
    return {};
}

}  // namespace pie::settings
