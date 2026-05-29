// Conformance tests: settings, auth, models, keybindings, and theme fixtures
// Task 97: Requirements 5.15, 6.14, 15.8, 18.7, 23.6, 23.7

#include <catch2/catch_test_macros.hpp>
#include "pie/wire/jsonl.hpp"
#include "pie/core/json.hpp"
#include "pie/settings/settings_manager.hpp"
#include "pie/auth/auth_storage.hpp"
#include "pie/cli/keybindings.hpp"
#include <filesystem>
#include <fstream>
#include <string>

#ifndef PIE_FIXTURES_DIR
#define PIE_FIXTURES_DIR "tests/fixtures"
#endif

static std::string read_file(const std::filesystem::path& path) {
    std::ifstream f(path);
    return {std::istreambuf_iterator<char>(f), std::istreambuf_iterator<char>()};
}

static pie::core::JsonValue parse_json_file(const std::filesystem::path& path) {
    return pie::core::parse_json(read_file(path));
}

TEST_CASE("Conformance: settings fixture preserves all known keys", "[conformance][settings]") {
    auto fixture_path = std::filesystem::path(PIE_FIXTURES_DIR) /
                        "pi-ts-output/settings/global.json";
    if (!std::filesystem::exists(fixture_path)) {
        SKIP("Settings fixture not found");
    }

    auto json = parse_json_file(fixture_path);
    REQUIRE(json.is_object());

    // Known top-level keys that must be present
    CHECK(json.contains("version"));
    CHECK(json.contains("autoCompact"));
    CHECK(json.contains("theme"));

    // Load via SettingsManager and verify it round-trips
    auto sm = pie::settings::SettingsManager::from_json(json);
    auto effective = sm.effective();
    REQUIRE(effective.is_object());

    // All keys from the fixture must survive the round-trip
    for (auto it = json.begin(); it != json.end(); ++it) {
        INFO("Checking key: " << it.key());
        CHECK(effective.contains(it.key()));
    }
}

TEST_CASE("Conformance: auth fixture parses without error", "[conformance][auth]") {
    auto fixture_path = std::filesystem::path(PIE_FIXTURES_DIR) /
                        "pi-ts-output/auth/empty.json";
    if (!std::filesystem::exists(fixture_path)) {
        SKIP("Auth fixture not found");
    }

    auto json = parse_json_file(fixture_path);
    REQUIRE(json.is_object());
    CHECK(json.contains("version"));
    CHECK(json.contains("providers"));
    CHECK(json["providers"].is_object());

    // Load via AuthStorage
    auto auth = pie::auth::AuthStorage::from_json(json);
    // Empty provider list — resolve_api_key must return nullopt for any key
    auto key = auth.resolve_api_key("anthropic");
    CHECK(!key.has_value());
}

TEST_CASE("Conformance: models fixture parses without error", "[conformance][models]") {
    auto fixture_path = std::filesystem::path(PIE_FIXTURES_DIR) /
                        "pi-ts-output/models/registry.json";
    if (!std::filesystem::exists(fixture_path)) {
        SKIP("Models fixture not found");
    }

    auto json = parse_json_file(fixture_path);
    REQUIRE(json.is_object());
    CHECK(json.contains("version"));
    CHECK(json.contains("custom"));
    CHECK(json["custom"].is_array());
}

TEST_CASE("Conformance: keybindings fixture loads without error", "[conformance][keybindings]") {
    auto fixture_path = std::filesystem::path(PIE_FIXTURES_DIR) /
                        "pi-ts-output/keybindings/default.json";
    if (!std::filesystem::exists(fixture_path)) {
        SKIP("Keybindings fixture not found");
    }

    auto json = parse_json_file(fixture_path);
    REQUIRE(json.is_object());

    auto& reg = pie::cli::KeybindingRegistry::instance();
    reg.load_from_json(json, fixture_path.string());

    auto bindings = reg.bindings();
    CHECK(bindings.count("app.exit") > 0);
    CHECK(bindings.count("app.clear") > 0);
    CHECK(bindings.count("app.interrupt") > 0);
}

TEST_CASE("Conformance: theme fixture parses and has required fields", "[conformance][themes]") {
    auto fixture_path = std::filesystem::path(PIE_FIXTURES_DIR) /
                        "pi-ts-output/themes/dark.json";
    if (!std::filesystem::exists(fixture_path)) {
        SKIP("Theme fixture not found");
    }

    auto json = parse_json_file(fixture_path);
    REQUIRE(json.is_object());
    CHECK(json.contains("name"));
    CHECK(json.contains("version"));
    CHECK(json.contains("colors"));
    CHECK(json["colors"].is_object());

    // Known required color keys
    auto& colors = json["colors"];
    CHECK(colors.contains("primary"));
    CHECK(colors.contains("background"));
    CHECK(colors.contains("text"));
}

TEST_CASE("Conformance: unknown keys preserved across JSON round-trip", "[conformance][unknown_keys]") {
    // Task 97: Req 23.7 — unknown keys must survive round-trip
    pie::core::JsonValue obj = {
        {"type", "settings"},
        {"version", 1},
        {"knownField", "value"},
        {"unknownFutureField", "should_survive"},
        {"anotherUnknown", 42}
    };

    std::string serialized = pie::wire::JsonlSerializer::serialize_line(obj);
    auto [entries, errors] = pie::wire::JsonlParser::parse(serialized);

    REQUIRE(errors.empty());
    REQUIRE(!entries.empty());

    CHECK(entries[0].value.contains("unknownFutureField"));
    CHECK(entries[0].value["unknownFutureField"] == "should_survive");
    CHECK(entries[0].value.contains("anotherUnknown"));
    CHECK(entries[0].value["anotherUnknown"] == 42);
}
