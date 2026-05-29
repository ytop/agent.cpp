#pragma once

#include "pie/core/json.hpp"
#include "pie/core/result.hpp"
#include <filesystem>
#include <string>
#include <vector>

namespace pie::settings {

enum class Tristate { Unset, Truthy, Falsy };

class EnvResolver {
public:
    std::filesystem::path agent_dir() const;
    std::filesystem::path session_dir() const;
    std::filesystem::path package_dir() const;
    Tristate offline() const;
    Tristate skip_version_check() const;
    Tristate telemetry() const;
    std::string cache_retention() const;
    std::string editor() const;
    std::string share_viewer_url() const;

    static Tristate parse_bool(const char* val);
};

class SettingsManager {
public:
    static Result<SettingsManager> create(
        const std::filesystem::path& global_path,
        const std::filesystem::path& project_path = "");
    static SettingsManager in_memory();

    // Deep merge: project overrides global, preserving nested structure
    static core::JsonValue deep_merge(const core::JsonValue& base, const core::JsonValue& overlay);

    core::JsonValue effective() const;
    core::JsonValue get(const std::string& key) const;
    void set(const std::string& key, const core::JsonValue& value);
    void apply_overrides(const core::JsonValue& overrides);

    Result<void> flush();
    const EnvResolver& env() const { return env_; }

private:
    SettingsManager() = default;
    Result<void> load_file(const std::filesystem::path& path, core::JsonValue& target);

    core::JsonValue global_ = core::JsonValue::object();
    core::JsonValue project_ = core::JsonValue::object();
    core::JsonValue overrides_ = core::JsonValue::object();
    std::filesystem::path global_path_;
    std::filesystem::path project_path_;
    EnvResolver env_;
};

}  // namespace pie::settings
