#include "pie/auth/auth_storage.hpp"
#include <cstdlib>
#include <fstream>
#include <sys/stat.h>

namespace pie::auth {

Result<AuthStorage> AuthStorage::create(const std::filesystem::path& agent_dir) {
    AuthStorage s;
    s.path_ = agent_dir / "auth.json";
    if (std::filesystem::exists(s.path_)) {
        auto r = s.load();
        if (!r) return std::unexpected(r.error());
    }
    return s;
}

AuthStorage AuthStorage::in_memory() { return AuthStorage{}; }

Result<void> AuthStorage::load() {
    std::ifstream f(path_);
    if (!f) return std::unexpected("cannot open " + path_.string());
    try {
        data_ = core::JsonValue::parse(f);
    } catch (const std::exception& e) {
        return std::unexpected(std::string("parse error: ") + e.what());
    }
    return {};
}

Result<void> AuthStorage::save() {
    if (path_.empty()) return {};
    std::filesystem::create_directories(path_.parent_path());
    auto tmp = path_.string() + ".tmp";
    {
        std::ofstream f(tmp);
        if (!f) return std::unexpected("cannot write " + tmp);
        f << data_.dump(2) << "\n";
    }
    chmod(tmp.c_str(), 0600);
    std::filesystem::rename(tmp, path_);
    return {};
}

std::optional<std::string> AuthStorage::resolve_api_key(const std::string& provider) const {
    // 1. Runtime override
    if (auto it = runtime_keys_.find(provider); it != runtime_keys_.end())
        return it->second;
    // 2. Stored in auth.json
    if (data_.contains(provider) && data_[provider].contains("apiKey"))
        return data_[provider]["apiKey"].get<std::string>();
    // 3. Environment variable (provider-specific)
    std::string env_var;
    if (provider == "anthropic") env_var = "ANTHROPIC_API_KEY";
    else if (provider == "openai") env_var = "OPENAI_API_KEY";
    else if (provider == "deepseek") env_var = "DEEPSEEK_API_KEY";
    else if (provider == "google-gemini") env_var = "GEMINI_API_KEY";
    
    if (!env_var.empty()) {
        if (const char* env_val = std::getenv(env_var.c_str())) {
            return std::string(env_val);
        }
    }
    return std::nullopt;
}

void AuthStorage::set_runtime_api_key(const std::string& provider, const std::string& key) {
    runtime_keys_[provider] = key;
}

void AuthStorage::store_api_key(const std::string& provider, const std::string& key) {
    if (!data_.contains(provider)) data_[provider] = core::JsonValue::object();
    data_[provider]["apiKey"] = key;
}

void AuthStorage::store_oauth(const std::string& provider, const OAuthCredentials& creds) {
    if (!data_.contains(provider)) data_[provider] = core::JsonValue::object();
    data_[provider]["accessToken"] = creds.access_token;
    data_[provider]["refreshToken"] = creds.refresh_token;
    data_[provider]["expiresAtMs"] = creds.expires_at_ms;
}

std::optional<OAuthCredentials> AuthStorage::get_oauth(const std::string& provider) const {
    if (!data_.contains(provider)) return std::nullopt;
    auto& p = data_[provider];
    if (!p.contains("accessToken")) return std::nullopt;
    return OAuthCredentials{
        p["accessToken"].get<std::string>(),
        p.value("refreshToken", ""),
        p.value("expiresAtMs", int64_t{0})
    };
}

void AuthStorage::remove_provider(const std::string& provider) {
    data_.erase(provider);
}

}  // namespace pie::auth
