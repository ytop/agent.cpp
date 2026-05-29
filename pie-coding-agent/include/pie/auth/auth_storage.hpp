#pragma once

#include "pie/core/json.hpp"
#include "pie/core/result.hpp"
#include <filesystem>
#include <map>
#include <optional>
#include <string>

namespace pie::auth {

struct OAuthCredentials {
    std::string access_token;
    std::string refresh_token;
    int64_t expires_at_ms = 0;
};

class AuthStorage {
public:
    static Result<AuthStorage> create(const std::filesystem::path& agent_dir);
    static AuthStorage in_memory();
    static AuthStorage from_json(const core::JsonValue& json) {
        AuthStorage s;
        s.data_ = json;
        return s;
    }

    // API key resolution: runtime > stored > env > fallback
    std::optional<std::string> resolve_api_key(const std::string& provider) const;

    void set_runtime_api_key(const std::string& provider, const std::string& key);
    void store_api_key(const std::string& provider, const std::string& key);
    void store_oauth(const std::string& provider, const OAuthCredentials& creds);
    std::optional<OAuthCredentials> get_oauth(const std::string& provider) const;
    void remove_provider(const std::string& provider);

    Result<void> save();
    const std::filesystem::path& path() const { return path_; }

private:
    AuthStorage() = default;
    Result<void> load();

    std::filesystem::path path_;
    core::JsonValue data_ = core::JsonValue::object();
    std::map<std::string, std::string> runtime_keys_;
};

}  // namespace pie::auth
