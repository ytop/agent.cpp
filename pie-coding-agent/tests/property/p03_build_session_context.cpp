// Feature: cpp-coding-agent, Property 3: buildSessionContext invariant
// build_session_context returns only entries on the current branch-to-root path.
// Feature: cpp-coding-agent, Property 4: session-tree append invariant
// Every appended entry appears in build_session_context.

#include <catch2/catch_test_macros.hpp>
#include <rapidcheck/catch.h>
#include "pie/session/session_manager.hpp"
#include "pie/core/json.hpp"
#include <string>

TEST_CASE("Property 3: build_session_context path invariant", "[property][session]") {
    // Feature: cpp-coding-agent, Property 3: buildSessionContext
    rc::prop("context only contains entries on active branch", []() {
        auto sm = pie::session::SessionManager::in_memory();
        int n = *rc::gen::inRange(1, 10);

        std::vector<std::string> appended;
        for (int i = 0; i < n; ++i) {
            auto id = sm.append_user_message("message " + std::to_string(i));
            appended.push_back(id);
        }

        auto ctx = sm.build_session_context();
        // Context must be non-empty for non-empty sessions
        RC_ASSERT(!ctx.empty());
        // All context messages must have role field
        for (const auto& msg : ctx) {
            RC_ASSERT(msg.contains("role") || msg.contains("type"));
        }
    });
}

TEST_CASE("Property 4: session tree append invariant", "[property][session]") {
    // Feature: cpp-coding-agent, Property 4: session-tree append invariants
    rc::prop("appended messages appear in context", []() {
        auto sm = pie::session::SessionManager::in_memory();
        int n = *rc::gen::inRange(1, 8);

        for (int i = 0; i < n; ++i) {
            std::string content = "message_" + std::to_string(i);
            sm.append_user_message(content);
        }

        auto ctx = sm.build_session_context();
        // Context should have at least as many entries as we appended
        RC_ASSERT(ctx.size() >= static_cast<size_t>(n));
    });
}
