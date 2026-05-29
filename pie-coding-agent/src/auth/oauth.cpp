#include "pie/auth/oauth.hpp"
#include "pie/io/http_client.hpp"
#include <chrono>
#include <thread>

namespace pie::auth {

Result<OAuthCredentials> oauth_device_flow(
    const OAuthConfig& config,
    DeviceCodeDisplay display_fn) {

    io::HttpClient http;
    http.set_timeout_ms(10000);

    // Step 1: Request device code
    auto resp = http.post(config.device_auth_url,
        "client_id=" + config.client_id,
        {{"Content-Type", "application/x-www-form-urlencoded"}});
    if (!resp) return std::unexpected(resp.error());
    if (resp->status_code != 200)
        return std::unexpected("device auth failed: HTTP " + std::to_string(resp->status_code));

    auto body = core::JsonValue::parse(resp->body);
    auto device_code = body.value("device_code", "");
    auto user_code = body.value("user_code", "");
    auto verification_url = body.value("verification_uri", body.value("verification_url", ""));
    auto interval = body.value("interval", config.poll_interval_s);

    if (device_code.empty())
        return std::unexpected("no device_code in response");

    display_fn(verification_url, user_code);

    // Step 2: Poll for token
    auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(config.timeout_s);

    while (std::chrono::steady_clock::now() < deadline) {
        std::this_thread::sleep_for(std::chrono::seconds(interval));

        auto token_resp = http.post(config.token_url,
            "client_id=" + config.client_id +
            "&device_code=" + device_code +
            "&grant_type=urn:ietf:params:oauth:grant-type:device_code",
            {{"Content-Type", "application/x-www-form-urlencoded"}});

        if (!token_resp) continue;
        if (token_resp->status_code == 200) {
            auto token_body = core::JsonValue::parse(token_resp->body);
            auto now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::system_clock::now().time_since_epoch()).count();
            auto expires_in = token_body.value("expires_in", 3600);

            return OAuthCredentials{
                token_body.value("access_token", ""),
                token_body.value("refresh_token", ""),
                now_ms + expires_in * 1000
            };
        }
        // authorization_pending — keep polling
        auto err_body = core::JsonValue::parse(token_resp->body);
        auto error = err_body.value("error", "");
        if (error != "authorization_pending" && error != "slow_down") {
            return std::unexpected("OAuth error: " + error);
        }
        if (error == "slow_down") interval += 1;
    }

    return std::unexpected("OAuth device flow timed out");
}

Result<OAuthCredentials> refresh_if_needed(
    AuthStorage& storage,
    const std::string& provider,
    const std::string& token_url,
    const std::string& client_id) {

    auto creds = storage.get_oauth(provider);
    if (!creds) return std::unexpected("no OAuth credentials for " + provider);

    auto now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();

    // Only refresh if within 60s of expiry
    if (creds->expires_at_ms - now_ms > 60000) return *creds;
    if (creds->refresh_token.empty()) return *creds;

    io::HttpClient http;
    http.set_timeout_ms(10000);

    auto resp = http.post(token_url,
        "client_id=" + client_id +
        "&refresh_token=" + creds->refresh_token +
        "&grant_type=refresh_token",
        {{"Content-Type", "application/x-www-form-urlencoded"}});

    if (!resp) return *creds;  // Return unrefreshed on failure
    if (resp->status_code != 200) return *creds;

    auto body = core::JsonValue::parse(resp->body);
    auto expires_in = body.value("expires_in", 3600);

    OAuthCredentials refreshed{
        body.value("access_token", ""),
        body.value("refresh_token", creds->refresh_token),
        now_ms + expires_in * 1000
    };

    storage.store_oauth(provider, refreshed);
    storage.save();
    return refreshed;
}

}  // namespace pie::auth
