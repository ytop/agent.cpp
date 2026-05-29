// Conformance tests: export-html, CLI events, and RPC traces
// Task 98: Requirements 2.6, 2.7, 12.8, 19.3

#include <catch2/catch_test_macros.hpp>
#include "pie/wire/jsonl.hpp"
#include "pie/sdk/exporter.hpp"
#include "pie/core/json.hpp"
#include <filesystem>
#include <fstream>
#include <regex>
#include <string>

#ifndef PIE_FIXTURES_DIR
#define PIE_FIXTURES_DIR "tests/fixtures"
#endif

static std::string read_file(const std::filesystem::path& path) {
    std::ifstream f(path);
    return {std::istreambuf_iterator<char>(f), std::istreambuf_iterator<char>()};
}

TEST_CASE("Conformance: export-html fixture is self-contained", "[conformance][export]") {
    auto fixture_path = std::filesystem::path(PIE_FIXTURES_DIR) /
                        "pi-ts-output/export-html/simple.html";
    if (!std::filesystem::exists(fixture_path)) {
        SKIP("Export HTML fixture not found");
    }

    std::string html = read_file(fixture_path);
    REQUIRE(!html.empty());

    // Must not reference external resources via src= or href= with http/https
    std::regex ext_ref(R"((src|href)\s*=\s*['"]https?://)", std::regex::icase);
    bool has_ext = std::regex_search(html, ext_ref);
    CHECK(!has_ext);

    // Must have basic HTML structure
    CHECK(html.find("<!DOCTYPE html>") != std::string::npos);
    CHECK(html.find("<html") != std::string::npos);
    CHECK(html.find("</html>") != std::string::npos);
}

TEST_CASE("Conformance: CLI events fixture has correct event sequence", "[conformance][cli_events]") {
    auto fixture_path = std::filesystem::path(PIE_FIXTURES_DIR) /
                        "pi-ts-output/cli-events/simple.jsonl";
    if (!std::filesystem::exists(fixture_path)) {
        SKIP("CLI events fixture not found");
    }

    std::string content = read_file(fixture_path);
    auto [entries, errors] = pie::wire::JsonlParser::parse(content);

    REQUIRE(errors.empty());
    REQUIRE(!entries.empty());

    // Req 2.6: First line MUST be session_header
    CHECK(entries[0].value["type"] == "session_header");

    // All event types must have a "type" field
    for (const auto& e : entries) {
        CHECK(e.value.contains("type"));
        CHECK(e.value.contains("timestamp"));
    }

    // Verify agent_start comes before agent_end
    int start_idx = -1, end_idx = -1;
    for (int i = 0; i < static_cast<int>(entries.size()); ++i) {
        std::string t = entries[i].value["type"].get<std::string>();
        if (t == "agent_start") start_idx = i;
        if (t == "agent_end") end_idx = i;
    }
    if (start_idx >= 0 && end_idx >= 0) {
        CHECK(start_idx < end_idx);
    }
}

TEST_CASE("Conformance: RPC traces fixture has valid command/response pairs", "[conformance][rpc]") {
    auto fixture_path = std::filesystem::path(PIE_FIXTURES_DIR) /
                        "pi-ts-output/rpc-traces/command_round_trip.jsonl";
    if (!std::filesystem::exists(fixture_path)) {
        SKIP("RPC traces fixture not found");
    }

    std::string content = read_file(fixture_path);
    auto [entries, errors] = pie::wire::JsonlParser::parse(content);

    REQUIRE(errors.empty());
    REQUIRE(entries.size() >= 2);

    // Verify request/response id matching
    // Collect all request IDs and response IDs
    std::vector<std::string> req_ids, resp_ids;
    for (const auto& e : entries) {
        if (!e.value.contains("type") || !e.value.contains("id")) continue;
        std::string t = e.value["type"].get<std::string>();
        std::string id = e.value["id"].get<std::string>();
        if (t == "response") {
            resp_ids.push_back(id);
        } else {
            req_ids.push_back(id);
        }
    }

    // Every response ID must match a request ID
    for (const auto& rid : resp_ids) {
        auto found = std::find(req_ids.begin(), req_ids.end(), rid);
        CHECK(found != req_ids.end());
    }
}

TEST_CASE("Conformance: Exporter produces self-contained HTML for sample messages", "[conformance][export]") {
    std::vector<pie::core::JsonValue> messages = {
        {{"role", "user"}, {"content", "Hello"}},
        {{"role", "assistant"}, {"content", "Hi there!"}}
    };

    std::string html = pie::sdk::export_html::Exporter::render(messages);

    REQUIRE(!html.empty());

    // Self-contained: no external resource refs
    std::regex ext_ref(R"((src|href)\s*=\s*['"]https?://)", std::regex::icase);
    bool has_ext = std::regex_search(html, ext_ref);
    CHECK(!has_ext);

    // Must contain the message content
    CHECK(html.find("Hello") != std::string::npos);
    CHECK(html.find("Hi there!") != std::string::npos);
}
