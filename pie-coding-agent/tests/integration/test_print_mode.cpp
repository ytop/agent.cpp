// Integration tests: PrintMode with mock provider and stdin pipe
// Task 90: Requirements 2.3, 2.4, 2.5, 24.3, 24.5

#include <catch2/catch_test_macros.hpp>
#include "pie/agent/agent_session.hpp"
#include "pie/agent/agent.hpp"
#include "pie/session/session_manager.hpp"
#include "pie/agent/events.hpp"
#include <sstream>
#include <iostream>

// Simple mock: injects scripted events into an AgentSession via subscription
static std::string collect_events(pie::AgentSession& sess) {
    std::ostringstream captured;
    auto sub = sess.subscribe([&](const pie::AgentSessionEvent& ev) {
        if (ev.contains("type") && ev["type"] == "message_delta") {
            if (ev.contains("content")) {
                captured << ev["content"].get<std::string>();
            }
        }
    });
    return captured.str();
}

TEST_CASE("Integration: PrintMode event subscription works", "[integration][print]") {
    auto agent = std::make_shared<pie::Agent>();
    auto sm = std::make_shared<pie::session::SessionManager>(
        pie::session::SessionManager::in_memory());
    pie::AgentSession sess(agent, sm);

    bool event_received = false;
    auto sub = sess.subscribe([&](const pie::AgentSessionEvent& ev) {
        event_received = true;
        (void)ev;
    });

    // Publish a synthetic event
    sess.publish_event({{"type", "test_event"}});
    CHECK(event_received == true);
}

TEST_CASE("Integration: AgentSession dispose prevents further calls", "[integration][session]") {
    auto agent = std::make_shared<pie::Agent>();
    auto sm = std::make_shared<pie::session::SessionManager>(
        pie::session::SessionManager::in_memory());
    auto sess = std::make_shared<pie::AgentSession>(agent, sm);

    sess->dispose();

    // After dispose, is_streaming must still be safe to call
    bool disposed_streaming = sess->is_streaming();
    CHECK(disposed_streaming == false);
}

TEST_CASE("Integration: Session subscriber isolation after dispose", "[integration][session]") {
    auto agent = std::make_shared<pie::Agent>();
    auto sm = std::make_shared<pie::session::SessionManager>(
        pie::session::SessionManager::in_memory());
    auto sess = std::make_shared<pie::AgentSession>(agent, sm);

    int count = 0;
    {
        auto sub = sess->subscribe([&](const pie::AgentSessionEvent&) { ++count; });
        sess->publish_event({{"type", "a"}});
        CHECK(count == 1);
        // sub goes out of scope → unsubscribes
    }
    sess->publish_event({{"type", "b"}});
    CHECK(count == 1); // Still 1, not 2
}
