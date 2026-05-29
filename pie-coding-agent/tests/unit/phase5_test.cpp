#include <catch2/catch_test_macros.hpp>
#include "pie/pie.hpp"
#include <asio.hpp>
#include <exception>
#include <stdexcept>

// Helper to run coroutines in catch2
template <typename T>
T run_coro(asio::io_context& ctx, asio::awaitable<T> awaitable) {
    T result;
    bool completed = false;
    asio::co_spawn(ctx, std::move(awaitable), [&](std::exception_ptr ex, T res) {
        completed = true;
        if (ex) std::rethrow_exception(ex);
        result = std::move(res);
    });
    ctx.run();
    ctx.restart();
    if (!completed) throw std::runtime_error("Coroutine did not complete");
    return result;
}

TEST_CASE("define_tool parameters_schema validation", "[sdk][tools]") {
    // 1. Valid JSON Schema
    pie::ToolDefinition def;
    def.name = "test_tool";
    def.label = "Test Tool";
    def.description = "A test tool";
    def.parameters_schema = {
        {"type", "object"},
        {"properties", {
            {"input", {{"type", "string"}}}
        }},
        {"required", {"input"}}
    };
    def.execute = [](const pie::core::JsonValue& params) -> pie::Result<pie::core::JsonValue> {
        return pie::core::JsonValue{{"result", params["input"].get<std::string>() + "_processed"}};
    };

    auto tool_res = pie::define_tool(def);
    REQUIRE(tool_res.has_value());
    REQUIRE((*tool_res)->name() == "test_tool");
    REQUIRE((*tool_res)->label() == "Test Tool");
    REQUIRE((*tool_res)->description() == "A test tool");

    // 2. Invalid JSON Schema
    pie::ToolDefinition invalid_def = def;
    invalid_def.parameters_schema = {
        {"type", "invalid_type"} // Invalid type schema
    };
    auto invalid_res = pie::define_tool(invalid_def);
    REQUIRE_FALSE(invalid_res.has_value());
}

TEST_CASE("AgentSession Subscription integrity", "[sdk][session]") {
    auto agent = std::make_shared<pie::Agent>();
    auto sm = std::make_shared<pie::SessionManager>(pie::SessionManager::in_memory());
    auto session = std::make_shared<pie::AgentSession>(agent, sm);

    int event_count = 0;
    std::string last_event_type;

    {
        auto sub = session->subscribe([&](const pie::AgentSessionEvent& ev) {
            event_count++;
            last_event_type = ev.value("type", "");
        });

        asio::io_context ctx;
        // Mock prompt or simple model set
        auto set_model_res = run_coro(ctx, session->set_model(pie::models::ModelInfo{
            "claude-sonnet-4-20250514", "anthropic", "Claude 3.5 Sonnet", 200000, true
        }));
        REQUIRE(set_model_res.has_value());

        // Run prompt
        auto prompt_res = run_coro(ctx, session->prompt("hello"));
        
        // We check if event_count > 0.
        REQUIRE(event_count > 0);
        REQUIRE(last_event_type != "");
        
        // Reset/unsubscribe
        sub.reset();
        event_count = 0;
        
        auto prompt_res2 = run_coro(ctx, session->prompt("world"));
        // Subscribed handle has been reset, so no events should be received!
        REQUIRE(event_count == 0);
    }
}

TEST_CASE("AgentSession Disposal guards subsequent calls", "[sdk][session]") {
    auto agent = std::make_shared<pie::Agent>();
    auto sm = std::make_shared<pie::SessionManager>(pie::SessionManager::in_memory());
    auto session = std::make_shared<pie::AgentSession>(agent, sm);

    session->dispose();

    asio::io_context ctx;
    auto prompt_res = run_coro(ctx, session->prompt("hello"));
    REQUIRE_FALSE(prompt_res.has_value());
    REQUIRE(prompt_res.error().find("disposed") != std::string::npos);

    auto steer_res = run_coro(ctx, session->steer("steer"));
    REQUIRE_FALSE(steer_res.has_value());

    auto set_model_res = run_coro(ctx, session->set_model(pie::models::ModelInfo{
        "claude-sonnet-4-20250514", "anthropic", "Claude 3.5 Sonnet", 200000, true
    }));
    REQUIRE_FALSE(set_model_res.has_value());
}

TEST_CASE("AgentSessionRuntime Switching Isolation", "[sdk][runtime]") {
    auto agent = std::make_shared<pie::Agent>();
    auto sm = std::make_shared<pie::SessionManager>(pie::SessionManager::in_memory());
    pie::AgentSessionRuntime runtime(agent, sm);

    auto initial_session = runtime.session;
    REQUIRE(initial_session != nullptr);

    int initial_event_count = 0;
    auto sub = initial_session->subscribe([&](const pie::AgentSessionEvent&) {
        initial_event_count++;
    });

    asio::io_context ctx;
    auto switch_res = run_coro(ctx, runtime.new_session());
    REQUIRE(switch_res.has_value());

    auto new_session = runtime.session;
    REQUIRE(new_session != nullptr);
    REQUIRE(new_session != initial_session);

    // Prompting the new session should NOT notify the subscriber on the initial session
    auto prompt_res = run_coro(ctx, new_session->prompt("test"));
    REQUIRE(initial_event_count == 0);
}
