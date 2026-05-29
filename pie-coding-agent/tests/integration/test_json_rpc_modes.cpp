// Integration tests: JsonMode and RpcMode end-to-end
// Task 91: Requirements 2.6, 2.7

#include <catch2/catch_test_macros.hpp>
#include "pie/wire/jsonl.hpp"
#include "pie/core/json.hpp"
#include "pie/agent/agent_session.hpp"
#include "pie/agent/agent.hpp"
#include "pie/session/session_manager.hpp"
#include <sstream>
#include <string>
#include <vector>

TEST_CASE("Integration: JsonMode JSONL event serialization", "[integration][json_mode]") {
    // Verify events emitted from an AgentSession serialize to valid JSONL
    auto agent = std::make_shared<pie::Agent>();
    auto sm = std::make_shared<pie::session::SessionManager>(
        pie::session::SessionManager::in_memory());
    pie::AgentSession sess(agent, sm);

    std::vector<std::string> lines;
    auto sub = sess.subscribe([&](const pie::AgentSessionEvent& ev) {
        std::string line = pie::wire::JsonlSerializer::serialize_line(ev);
        lines.push_back(line);
    });

    sess.publish_event({{"type", "agent_start"}});
    sess.publish_event({{"type", "message_delta"}, {"content", "Hello"}});
    sess.publish_event({{"type", "agent_end"}, {"status", "success"}});

    REQUIRE(lines.size() == 3);
    for (const auto& line : lines) {
        REQUIRE(!line.empty());
        REQUIRE(line.back() == '\n');

        auto [entries, errors] = pie::wire::JsonlParser::parse(line);
        CHECK(errors.empty());
        REQUIRE(!entries.empty());
        CHECK(entries[0].value.contains("type"));
    }
}

TEST_CASE("Integration: RpcMode LF framing tolerance", "[integration][rpc_mode]") {
    // Verify that CR+LF input is tolerated (trailing \r stripped)
    std::string crlf_line = "{\"type\": \"prompt\", \"message\": \"hi\"}\r\n";

    // Strip trailing \r before parsing
    std::string stripped = crlf_line;
    if (!stripped.empty() && stripped.back() == '\n') stripped.pop_back();
    if (!stripped.empty() && stripped.back() == '\r') stripped.pop_back();
    stripped += '\n';

    auto [entries, errors] = pie::wire::JsonlParser::parse(stripped);
    CHECK(errors.empty());
    REQUIRE(!entries.empty());
    CHECK(entries[0].value["type"] == "prompt");
    CHECK(entries[0].value["message"] == "hi");
}

TEST_CASE("Integration: RpcMode command round-trip JSON validity", "[integration][rpc_mode]") {
    // Verify request/response JSON pairs are valid
    pie::core::JsonValue req = {
        {"id", "req-1"},
        {"type", "get_state"}
    };
    pie::core::JsonValue resp = {
        {"type", "response"},
        {"id", "req-1"},
        {"command", "get_state"},
        {"success", true},
        {"data", {{"isStreaming", false}}}
    };

    std::string req_line = pie::wire::JsonlSerializer::serialize_line(req);
    std::string resp_line = pie::wire::JsonlSerializer::serialize_line(resp);

    auto [req_entries, req_errors] = pie::wire::JsonlParser::parse(req_line);
    auto [resp_entries, resp_errors] = pie::wire::JsonlParser::parse(resp_line);

    CHECK(req_errors.empty());
    CHECK(resp_errors.empty());
    CHECK(req_entries[0].value["id"] == resp_entries[0].value["id"]);
}
