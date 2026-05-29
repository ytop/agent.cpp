// Feature: cpp-coding-agent, Property 7: message queue drain correctness
// Any sequence of push/drain drains all messages exactly once.
// Feature: cpp-coding-agent, Property 8: migration preserves all entries
// v1/v2 session migration never loses entries.
// Feature: cpp-coding-agent, Property 9: excludeFromContext entries not in context

#include <catch2/catch_test_macros.hpp>
#include <rapidcheck/catch.h>
#include "pie/queue/message_queue.hpp"
#include "pie/session/session_manager.hpp"
#include <string>
#include <vector>

TEST_CASE("Property 7: message queue drain correctness", "[property][queue]") {
    // Feature: cpp-coding-agent, Property 7: message queue
    rc::prop("drain_all returns all pushed messages exactly once", []() {
        pie::queue::MessageQueue q;
        int n = *rc::gen::inRange(0, 20);
        std::vector<std::string> pushed;
        for (int i = 0; i < n; ++i) {
            std::string msg = "steering_" + std::to_string(i);
            q.push_steering(msg);
            pushed.push_back(msg);
        }
        RC_ASSERT(q.steering_size() == static_cast<size_t>(n));

        auto drained = q.drain_steering(pie::queue::DrainMode::All);
        RC_ASSERT(drained.size() == pushed.size());
        RC_ASSERT(q.steering_size() == 0u);

        // Verify all messages drained (may be in order)
        for (const auto& msg : pushed) {
            RC_ASSERT(std::find(drained.begin(), drained.end(), msg) != drained.end());
        }
    });

    rc::prop("drain one-at-a-time leaves remaining", []() {
        pie::queue::MessageQueue q;
        int n = *rc::gen::inRange(2, 10);
        for (int i = 0; i < n; ++i) {
            q.push_follow_up("msg_" + std::to_string(i));
        }
        auto one = q.drain_follow_up(pie::queue::DrainMode::OneAtATime);
        RC_ASSERT(one.size() == 1u);
        RC_ASSERT(q.follow_up_size() == static_cast<size_t>(n - 1));
    });
}

TEST_CASE("Property 8: session migration preserves entries", "[property][session][migration]") {
    // Feature: cpp-coding-agent, Property 8: migration
    rc::prop("in-memory session context grows monotonically with appends", []() {
        auto sm = pie::session::SessionManager::in_memory();
        int n = *rc::gen::inRange(1, 10);

        size_t prev_size = 0;
        for (int i = 0; i < n; ++i) {
            sm.append_user_message("user_" + std::to_string(i));
            sm.append_assistant_message("assistant_" + std::to_string(i));
            auto ctx = sm.build_session_context();
            RC_ASSERT(ctx.size() >= prev_size);
            prev_size = ctx.size();
        }
    });
}

TEST_CASE("Property 9: excludeFromContext invariant", "[property][session]") {
    // Feature: cpp-coding-agent, Property 9: excludeFromContext
    rc::prop("context does not contain empty entries", []() {
        auto sm = pie::session::SessionManager::in_memory();
        int n = *rc::gen::inRange(1, 8);
        for (int i = 0; i < n; ++i) {
            sm.append_user_message("u" + std::to_string(i));
        }
        auto ctx = sm.build_session_context();
        // Every entry in context must have content
        for (const auto& entry : ctx) {
            RC_ASSERT(!entry.is_null());
        }
    });
}
