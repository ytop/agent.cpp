#pragma once

#include "pie/auth/auth_storage.hpp"
#include "pie/core/result.hpp"
#include <functional>
#include <string>

namespace pie::auth {

// Callback to display device code URL to user
using DeviceCodeDisplay = std::function<void(const std::string& url, const std::string& code)>;

struct OAuthConfig {
    std::string provider_id;
    std::string device_auth_url;
    std::string token_url;
    std::string client_id;
    int poll_interval_s = 5;
    int timeout_s = 300;
};

// Perform OAuth device-code flow. Blocks until complete/timeout/cancel.
Result<OAuthCredentials> oauth_device_flow(
    const OAuthConfig& config,
    DeviceCodeDisplay display_fn);

// Refresh token if within 60s of expiry. Returns refreshed creds or error.
Result<OAuthCredentials> refresh_if_needed(
    AuthStorage& storage,
    const std::string& provider,
    const std::string& token_url,
    const std::string& client_id);

}  // namespace pie::auth
