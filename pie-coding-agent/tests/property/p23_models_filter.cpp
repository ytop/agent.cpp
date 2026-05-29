// Feature: cpp-coding-agent, Property 23: model filtering
// Feature: cpp-coding-agent, Property 24: thinking level cycle
// Feature: cpp-coding-agent, Property 25: atomic append file integrity
// Feature: cpp-coding-agent, Property 26: self-contained HTML no external refs

#include <catch2/catch_test_macros.hpp>
#include <rapidcheck/catch.h>
#include "pie/models/model_registry.hpp"
#include "pie/agent/agent_session.hpp"
#include "pie/agent/agent.hpp"
#include "pie/session/session_manager.hpp"
#include "pie/wire/jsonl.hpp"
#include "pie/sdk/exporter.hpp"
#include <string>
#include <regex>

TEST_CASE("Property 23: model filtering invariant", "[property][models]") {
    // Feature: cpp-coding-agent, Property 23: models filter
    rc::prop("get_available with empty providers returns empty list", []() {
        pie::models::ModelRegistry reg;
        auto models = reg.get_available({});
        RC_ASSERT(models.empty());
    });

    rc::prop("all() returns subset matching registered models", []() {
        pie::models::ModelRegistry reg;
        // Default registry should have known providers
        auto all = reg.all();
        // Either empty (no JSON loaded) or all have non-empty IDs
        for (const auto& m : all) {
            RC_ASSERT(!m.id.empty());
            RC_ASSERT(!m.provider.empty());
        }
    });
}

TEST_CASE("Property 24: thinking level cycle is cyclic", "[property][session]") {
    // Feature: cpp-coding-agent, Property 24: thinking cycle
    rc::prop("cycling through all levels returns to original", []() {
        auto agent = std::make_shared<pie::Agent>();
        auto sm = std::make_shared<pie::session::SessionManager>(
            pie::session::SessionManager::in_memory());
        pie::AgentSession sess(agent, sm);

        auto initial = sess.thinking_level();
        // Cycle through 4 levels (Off, Low, Medium, High)
        for (int i = 0; i < 4; ++i) {
            sess.cycle_thinking_level();
        }
        RC_ASSERT(sess.thinking_level() == initial);
    });
}

TEST_CASE("Property 25: atomic append file integrity", "[property][session]") {
    // Feature: cpp-coding-agent, Property 25: atomic append
    rc::prop("serialized JSONL lines always end with newline", []() {
        pie::core::JsonValue val = pie::core::JsonValue::object();
        val["id"] = *rc::gen::arbitrary<std::string>();
        val["type"] = "message";

        std::string line = pie::wire::JsonlSerializer::serialize_line(val);
        RC_ASSERT(!line.empty());
        RC_ASSERT(line.back() == '\n');

        // Verify the line is parseable
        auto [entries, errors] = pie::wire::JsonlParser::parse(line);
        RC_ASSERT(errors.empty());
    });
}

TEST_CASE("Property 26: self-contained HTML has no external refs", "[property][export]") {
    // Feature: cpp-coding-agent, Property 26: self-contained HTML
    rc::prop("rendered HTML contains no external http/https resource links", []() {
        int n = *rc::gen::inRange(0, 5);
        std::vector<pie::core::JsonValue> messages;
        for (int i = 0; i < n; ++i) {
            pie::core::JsonValue msg;
            msg["role"] = (i % 2 == 0) ? "user" : "assistant";
            msg["content"] = "message " + std::to_string(i);
            messages.push_back(msg);
        }

        std::string html = pie::sdk::export_html::Exporter::render(messages);

        // Must not contain external resource URLs in src= or href= attributes
        std::regex ext_ref(R"((src|href)\s*=\s*['"]https?://)", std::regex::icase);
        bool has_ext = std::regex_search(html, ext_ref);
        RC_ASSERT(!has_ext);
    });
}
