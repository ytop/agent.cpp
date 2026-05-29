#include <catch2/catch_test_macros.hpp>
#include "pie/cli/cli_invocation.hpp"
#include "pie/cli/slash_commands.hpp"
#include "pie/cli/keybindings.hpp"
#include <filesystem>
#include <fstream>

using namespace pie;

TEST_CASE("CliInvocation parsing and validation", "[cli][args]") {
    SECTION("Normal print mode parsing") {
        char* argv[] = { (char*)"pie", (char*)"-p", (char*)"hello", (char*)"world" };
        int argc = 4;
        auto res = cli::parse_args(argc, argv);
        REQUIRE(res.has_value());
        CHECK(res->print == true);
        CHECK(res->mode == cli::CliInvocation::Mode::Print);
        REQUIRE(res->message_tokens.size() == 2);
        CHECK(res->message_tokens[0] == "hello");
        CHECK(res->message_tokens[1] == "world");
    }

    SECTION("Conflicting mode option validation") {
        char* argv[] = { (char*)"pie", (char*)"-p", (char*)"--mode", (char*)"json" };
        int argc = 4;
        auto res = cli::parse_args(argc, argv);
        REQUIRE(res.has_value());
        CHECK(res->mode == cli::CliInvocation::Mode::ConflictError);
    }

    SECTION("Repeatable extensions and skills parsing") {
        char* argv[] = { 
            (char*)"pie", 
            (char*)"--extension", (char*)"/path/a", 
            (char*)"--extension", (char*)"/path/b",
            (char*)"--skill", (char*)"/skill/c",
            (char*)"--theme", (char*)"nord"
        };
        int argc = 9;
        auto res = cli::parse_args(argc, argv);
        REQUIRE(res.has_value());
        REQUIRE(res->extensions.size() == 2);
        CHECK(res->extensions[0] == "/path/a");
        CHECK(res->extensions[1] == "/path/b");
        REQUIRE(res->skills.size() == 1);
        CHECK(res->skills[0] == "/skill/c");
        REQUIRE(res->themes.size() == 1);
        CHECK(res->themes[0] == "nord");
    }
}

TEST_CASE("CommandRegistry builtin commands precedence", "[cli][commands]") {
    auto& registry = cli::CommandRegistry::instance();

    SECTION("Login command execution") {
        auto agent = std::make_shared<pie::Agent>();
        auto sm = std::make_shared<pie::session::SessionManager>(pie::session::SessionManager::in_memory());
        pie::AgentSessionRuntime runtime(agent, sm);

        auto exec_res = registry.execute_command(runtime, "/login");
        CHECK(exec_res == true);
    }

    SECTION("Unknown slash command fails gracefully") {
        auto agent = std::make_shared<pie::Agent>();
        auto sm = std::make_shared<pie::session::SessionManager>(pie::session::SessionManager::in_memory());
        pie::AgentSessionRuntime runtime(agent, sm);

        auto exec_res = registry.execute_command(runtime, "/non_existent_command");
        CHECK(exec_res == false);
    }
}

TEST_CASE("KeybindingRegistry override and resolution", "[cli][keybindings]") {
    auto& registry = cli::KeybindingRegistry::instance();

    SECTION("Load valid JSON keybindings") {
        core::JsonValue kb_json = {
            {"app.exit", "ctrl+q"},
            {"app.clear", "ctrl+c"}
        };
        
        registry.load_from_json(kb_json, "keybindings.json");
        auto b = registry.bindings();
        CHECK(b["app.exit"] == "ctrl+q");
        CHECK(b["app.clear"] == "ctrl+c");
    }
}
