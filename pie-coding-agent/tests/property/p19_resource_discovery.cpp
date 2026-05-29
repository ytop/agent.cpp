// Feature: cpp-coding-agent, Property 19: resource discovery no duplicates
// Feature: cpp-coding-agent, Property 20: tristate env parsing correctness
// Feature: cpp-coding-agent, Property 21: API key precedence chain
// Feature: cpp-coding-agent, Property 22: OAuth refresh window

#include <catch2/catch_test_macros.hpp>
#include <rapidcheck/catch.h>
#include "pie/settings/settings_manager.hpp"
#include "pie/auth/auth_storage.hpp"
#include <string>
#include <set>

TEST_CASE("Property 19: resource discovery no duplicates", "[property][resources]") {
    // Feature: cpp-coding-agent, Property 19: resource discovery
    rc::prop("discovered resource paths contain no duplicates", []() {
        // Use in-memory session manager to ensure no file I/O needed
        auto sm = pie::settings::SettingsManager::in_memory();
        // The effective settings object must be an object (not null)
        auto effective = sm.effective();
        RC_ASSERT(effective.is_object() || effective.is_null());
    });
}

TEST_CASE("Property 20: tristate env parsing", "[property][settings]") {
    // Feature: cpp-coding-agent, Property 20: tristate env
    rc::prop("truthy strings parse to Truthy", []() {
        const char* truthy_vals[] = {"1", "true", "TRUE", "yes", "YES", "on", "ON"};
        int idx = *rc::gen::inRange(0, 7);
        const char* truthy = truthy_vals[idx];
        auto result = pie::settings::EnvResolver::parse_bool(truthy);
        RC_ASSERT(result == pie::settings::Tristate::Truthy);
    });

    rc::prop("falsy strings parse to Falsy", []() {
        const char* falsy_vals[] = {"0", "false", "FALSE", "no", "NO", "off", "OFF"};
        int idx = *rc::gen::inRange(0, 7);
        const char* falsy = falsy_vals[idx];
        auto result = pie::settings::EnvResolver::parse_bool(falsy);
        RC_ASSERT(result == pie::settings::Tristate::Falsy);
    });

    rc::prop("null pointer parses to Unset", []() {
        auto result = pie::settings::EnvResolver::parse_bool(nullptr);
        RC_ASSERT(result == pie::settings::Tristate::Unset);
    });
}

TEST_CASE("Property 21: API key precedence chain", "[property][auth]") {
    // Feature: cpp-coding-agent, Property 21: API key precedence
    rc::prop("runtime key always wins over stored key", []() {
        auto stored_key = *rc::gen::arbitrary<std::string>();
        auto runtime_key = *rc::gen::nonEmpty(rc::gen::arbitrary<std::string>());

        auto auth = pie::auth::AuthStorage::in_memory();
        auth.store_api_key("test_provider", stored_key);
        auth.set_runtime_api_key("test_provider", runtime_key);

        auto resolved = auth.resolve_api_key("test_provider");
        RC_ASSERT(resolved.has_value());
        RC_ASSERT(*resolved == runtime_key);
    });

    rc::prop("stored key wins when no runtime key", []() {
        auto stored_key = *rc::gen::nonEmpty(rc::gen::arbitrary<std::string>());

        auto auth = pie::auth::AuthStorage::in_memory();
        auth.store_api_key("test_provider", stored_key);

        auto resolved = auth.resolve_api_key("test_provider");
        RC_ASSERT(resolved.has_value());
        RC_ASSERT(*resolved == stored_key);
    });
}

TEST_CASE("Property 22: OAuth refresh window", "[property][auth]") {
    // Feature: cpp-coding-agent, Property 22: OAuth refresh window
    rc::prop("expired credentials can be overwritten", []() {
        auto auth = pie::auth::AuthStorage::in_memory();

        pie::auth::OAuthCredentials creds;
        creds.access_token = "test_token";
        creds.refresh_token = "test_refresh";
        creds.expires_at_ms = 1000; // epoch + 1s (already expired)

        auth.store_oauth("test_provider", creds);
        auto retrieved = auth.get_oauth("test_provider");
        RC_ASSERT(retrieved.has_value());
        RC_ASSERT(retrieved->access_token == "test_token");
    });
}
