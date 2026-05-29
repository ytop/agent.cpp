// Integration tests: Compactor with mock provider
// Task 93: Requirements 8.3, 8.4, 8.6

#include <catch2/catch_test_macros.hpp>
#include "pie/compaction/compactor.hpp"
#include "pie/session/session_manager.hpp"
#include "pie/agent/agent.hpp"
#include <string>

TEST_CASE("Integration: Compactor can be constructed", "[integration][compactor]") {
    auto agent = std::make_shared<pie::Agent>();
    auto sm = std::make_shared<pie::session::SessionManager>(
        pie::session::SessionManager::in_memory());

    pie::compaction::Compactor compactor(agent, sm);
    // Must be constructable without error
    CHECK(true);
}

TEST_CASE("Integration: Compactor threshold check for empty session", "[integration][compactor]") {
    auto agent = std::make_shared<pie::Agent>();
    auto sm = std::make_shared<pie::session::SessionManager>(
        pie::session::SessionManager::in_memory());

    pie::compaction::Compactor compactor(agent, sm);
    // Empty session should not trigger threshold compaction
    bool needs_compact = compactor.needs_compaction();
    CHECK(needs_compact == false);
}

TEST_CASE("Integration: Compactor handles empty context gracefully", "[integration][compactor]") {
    auto agent = std::make_shared<pie::Agent>();
    auto sm = std::make_shared<pie::session::SessionManager>(
        pie::session::SessionManager::in_memory());

    pie::compaction::Compactor compactor(agent, sm);
    auto result = compactor.get_last_compaction_summary();
    // For fresh session, no compaction summary exists
    CHECK(!result.has_value());
}
