// Conformance tests: Pi_TS-produced session fixtures
// Task 96: Requirements 3.11, 3.12, 23.1, 23.2, 23.3

#include <catch2/catch_test_macros.hpp>
#include "pie/wire/jsonl.hpp"
#include "pie/core/json.hpp"
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

#ifndef PIE_FIXTURES_DIR
#define PIE_FIXTURES_DIR "tests/fixtures"
#endif

static std::string read_file(const std::filesystem::path& path) {
    std::ifstream f(path);
    return {std::istreambuf_iterator<char>(f), std::istreambuf_iterator<char>()};
}

static bool structurally_equal(const pie::core::JsonValue& a, const pie::core::JsonValue& b) {
    if (a.type() != b.type()) return false;
    if (a.is_object()) {
        for (auto it = a.begin(); it != a.end(); ++it) {
            if (!b.contains(it.key())) return false;
            if (!structurally_equal(it.value(), b[it.key()])) return false;
        }
        return true;
    }
    if (a.is_array()) {
        if (a.size() != b.size()) return false;
        for (size_t i = 0; i < a.size(); ++i) {
            if (!structurally_equal(a[i], b[i])) return false;
        }
        return true;
    }
    return a == b;
}

TEST_CASE("Conformance: v3 simple session fixture round-trips", "[conformance][session]") {
    auto fixture_path = std::filesystem::path(PIE_FIXTURES_DIR) /
                        "pi-ts-output/sessions/v3/simple.jsonl";

    if (!std::filesystem::exists(fixture_path)) {
        SKIP("Fixture not found — run scripts/generate_fixtures.sh first");
    }

    std::string content = read_file(fixture_path);
    auto [entries, errors] = pie::wire::JsonlParser::parse(content);

    REQUIRE(errors.empty());
    REQUIRE(entries.size() == 3);

    // First entry must be session header with version:3
    CHECK(entries[0].value["type"] == "session");
    CHECK(entries[0].value["version"] == 3);
    CHECK(entries[0].value.contains("id"));
    CHECK(entries[0].value.contains("timestamp"));

    // All subsequent entries are messages
    CHECK(entries[1].value["type"] == "message");
    CHECK(entries[2].value["type"] == "message");

    // Round-trip: re-serialize each entry and parse again
    for (size_t i = 0; i < entries.size(); ++i) {
        std::string re_serialized = pie::wire::JsonlSerializer::serialize_line(entries[i].value);
        auto [re_entries, re_errors] = pie::wire::JsonlParser::parse(re_serialized);

        INFO("Entry " << i << " re-serialize round-trip");
        REQUIRE(re_errors.empty());
        REQUIRE(!re_entries.empty());

        // All known keys must survive
        for (auto it = entries[i].value.begin(); it != entries[i].value.end(); ++it) {
            CHECK(re_entries[0].value.contains(it.key()));
        }
    }
}

TEST_CASE("Conformance: v3 tool-use session fixture parses correctly", "[conformance][session]") {
    auto fixture_path = std::filesystem::path(PIE_FIXTURES_DIR) /
                        "pi-ts-output/sessions/v3/with_tool_use.jsonl";

    if (!std::filesystem::exists(fixture_path)) {
        SKIP("Fixture not found — run scripts/generate_fixtures.sh first");
    }

    std::string content = read_file(fixture_path);
    auto [entries, errors] = pie::wire::JsonlParser::parse(content);

    REQUIRE(errors.empty());
    REQUIRE(entries.size() == 4); // header + 3 messages

    // Verify parentId chain is intact
    CHECK(entries[1].value.contains("parentId"));
    CHECK(entries[2].value.contains("parentId"));
    CHECK(entries[3].value.contains("parentId"));

    // Tool use message has array content
    auto& tool_msg = entries[2].value;
    CHECK(tool_msg["message"]["content"].is_array());
    CHECK(tool_msg["message"]["content"][0]["type"] == "tool_use");
}

TEST_CASE("Conformance: CLI events fixture has session_header first", "[conformance][cli_events]") {
    auto fixture_path = std::filesystem::path(PIE_FIXTURES_DIR) /
                        "pi-ts-output/cli-events/simple.jsonl";

    if (!std::filesystem::exists(fixture_path)) {
        SKIP("Fixture not found — run scripts/generate_fixtures.sh first");
    }

    std::string content = read_file(fixture_path);
    auto [entries, errors] = pie::wire::JsonlParser::parse(content);

    REQUIRE(errors.empty());
    REQUIRE(!entries.empty());

    // First event MUST be session_header (JSON mode requirement)
    CHECK(entries[0].value["type"] == "session_header");
    CHECK(entries[0].value.contains("sessionId"));
    CHECK(entries[0].value.contains("model"));
}
