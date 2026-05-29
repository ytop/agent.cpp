#include <catch2/catch_test_macros.hpp>
#include <filesystem>
#include <fstream>

#include "pie/auth/auth_storage.hpp"
#include "pie/auth/providers.hpp"
#include "pie/models/model_registry.hpp"
#include "pie/resources/prompt_loader.hpp"
#include "pie/resources/resource_loader.hpp"
#include "pie/session/migration.hpp"
#include "pie/session/session_manager.hpp"
#include "pie/settings/first_run_import.hpp"
#include "pie/settings/settings_manager.hpp"

namespace fs = std::filesystem;

// --- AuthStorage ---

TEST_CASE("AuthStorage API-key precedence chain", "[auth]") {
    auto tmp = fs::temp_directory_path() / "pie_test_auth";
    fs::create_directories(tmp);

    auto storage = pie::auth::AuthStorage::create(tmp);
    REQUIRE(storage.has_value());

    // No key initially
    REQUIRE_FALSE(storage->resolve_api_key("anthropic").has_value());

    // Store a key
    storage->store_api_key("anthropic", "stored-key");
    REQUIRE(storage->resolve_api_key("anthropic") == "stored-key");

    // Runtime override takes precedence
    storage->set_runtime_api_key("anthropic", "runtime-key");
    REQUIRE(storage->resolve_api_key("anthropic") == "runtime-key");

    // Save and reload
    REQUIRE(storage->save().has_value());
    auto reloaded = pie::auth::AuthStorage::create(tmp);
    REQUIRE(reloaded.has_value());
    REQUIRE(reloaded->resolve_api_key("anthropic") == "stored-key");

    fs::remove_all(tmp);
}

TEST_CASE("AuthStorage auth.json round-trip", "[auth]") {
    auto tmp = fs::temp_directory_path() / "pie_test_auth_rt";
    fs::create_directories(tmp);

    auto storage = pie::auth::AuthStorage::create(tmp);
    REQUIRE(storage.has_value());

    storage->store_oauth("openai", {"access-tok", "refresh-tok", 9999999});
    storage->save();

    auto reloaded = pie::auth::AuthStorage::create(tmp);
    auto creds = reloaded->get_oauth("openai");
    REQUIRE(creds.has_value());
    REQUIRE(creds->access_token == "access-tok");
    REQUIRE(creds->refresh_token == "refresh-tok");

    fs::remove_all(tmp);
}

// --- Provider descriptors ---

TEST_CASE("Provider descriptors has 29 entries", "[auth]") {
    REQUIRE(pie::auth::all_providers().size() == 29);
    REQUIRE(pie::auth::find_provider("anthropic") != nullptr);
    REQUIRE(pie::auth::find_provider("nonexistent") == nullptr);
}

// --- SessionManager ---

TEST_CASE("SessionManager append and build context", "[session]") {
    auto tmp = fs::temp_directory_path() / "pie_test_sessions";
    fs::create_directories(tmp);

    auto sm = pie::session::SessionManager::create(tmp);
    REQUIRE(sm.has_value());

    auto id1 = sm->append_user_message("hello");
    REQUIRE(id1.size() == 8);

    auto id2 = sm->append_assistant_message("hi there");
    REQUIRE(id2.size() == 8);

    auto context = sm->build_session_context();
    REQUIRE(context.size() == 2);
    REQUIRE(context[0]["role"] == "user");
    REQUIRE(context[1]["role"] == "assistant");

    fs::remove_all(tmp);
}

// --- SettingsManager ---

TEST_CASE("SettingsManager deep merge", "[settings]") {
    using pie::settings::SettingsManager;
    using pie::core::JsonValue;

    JsonValue base = {{"a", {{"x", 1}, {"y", 2}}}, {"b", "hello"}};
    JsonValue overlay = {{"a", {{"x", 99}, {"z", 3}}}, {"c", true}};

    auto merged = SettingsManager::deep_merge(base, overlay);
    REQUIRE(merged["a"]["x"] == 99);
    REQUIRE(merged["a"]["y"] == 2);
    REQUIRE(merged["a"]["z"] == 3);
    REQUIRE(merged["b"] == "hello");
    REQUIRE(merged["c"] == true);
}

TEST_CASE("EnvResolver tristate parsing", "[settings]") {
    using pie::settings::EnvResolver;
    using pie::settings::Tristate;

    REQUIRE(EnvResolver::parse_bool(nullptr) == Tristate::Unset);
    REQUIRE(EnvResolver::parse_bool("") == Tristate::Unset);
    REQUIRE(EnvResolver::parse_bool("true") == Tristate::Truthy);
    REQUIRE(EnvResolver::parse_bool("TRUE") == Tristate::Truthy);
    REQUIRE(EnvResolver::parse_bool("1") == Tristate::Truthy);
    REQUIRE(EnvResolver::parse_bool("false") == Tristate::Falsy);
    REQUIRE(EnvResolver::parse_bool("0") == Tristate::Falsy);
    REQUIRE(EnvResolver::parse_bool("no") == Tristate::Falsy);
}

// --- ModelRegistry ---

TEST_CASE("ModelRegistry find and parse_selector", "[models]") {
    auto reg = pie::models::ModelRegistry::create();

    auto model = reg.find("claude-sonnet-4-20250514");
    REQUIRE(model.has_value());
    REQUIRE(model->provider == "anthropic");

    REQUIRE_FALSE(reg.find("nonexistent").has_value());

    auto sel = pie::models::ModelRegistry::parse_selector("anthropic/claude-sonnet-4-20250514:thinking");
    REQUIRE(sel.has_value());
    REQUIRE(sel->provider == "anthropic");
    REQUIRE(sel->model_id == "claude-sonnet-4-20250514");
    REQUIRE(sel->thinking == true);

    auto sel2 = pie::models::ModelRegistry::parse_selector("gpt-4o");
    REQUIRE(sel2.has_value());
    REQUIRE(sel2->provider.empty());
    REQUIRE(sel2->model_id == "gpt-4o");
    REQUIRE(sel2->thinking == false);
}

// --- ResourceLoader ---

TEST_CASE("ResourceLoader discovers AGENTS.md", "[resources]") {
    auto tmp = fs::temp_directory_path() / "pie_test_resources";
    auto agent_dir = tmp / "agent";
    fs::create_directories(agent_dir);
    std::ofstream(agent_dir / "AGENTS.md") << "# Agent instructions\n";

    pie::resources::ResourceLoader loader(agent_dir, tmp);
    auto content = loader.load_context_files();
    REQUIRE(content.find("Agent instructions") != std::string::npos);

    fs::remove_all(tmp);
}

TEST_CASE("ResourceLoader SYSTEM.md override", "[resources]") {
    auto tmp = fs::temp_directory_path() / "pie_test_system";
    auto agent_dir = tmp / "agent";
    fs::create_directories(agent_dir);
    std::ofstream(agent_dir / "SYSTEM.md") << "global system";

    auto project_dir = tmp / "project" / ".pie";
    fs::create_directories(project_dir);
    std::ofstream(project_dir / "SYSTEM.md") << "project system";

    pie::resources::ResourceLoader loader(agent_dir, tmp / "project");
    auto prompt = loader.load_system_prompt();
    REQUIRE(prompt == "project system");

    fs::remove_all(tmp);
}

// --- PromptLoader ---

TEST_CASE("PromptLoader variable substitution", "[resources]") {
    auto result = pie::resources::PromptLoader::substitute(
        "Hello {{name}}, welcome to {{place}}!",
        {{"name", "World"}, {"place", "Pie"}});
    REQUIRE(result.has_value());
    REQUIRE(*result == "Hello World, welcome to Pie!");

    auto err = pie::resources::PromptLoader::substitute(
        "Hello {{missing}}!", {});
    REQUIRE_FALSE(err.has_value());
    REQUIRE(err.error().find("missing") != std::string::npos);
}

// --- FirstRunImport ---

TEST_CASE("FirstRunImport creates marker file", "[settings]") {
    auto tmp = fs::temp_directory_path() / "pie_test_import";
    auto ts_dir = tmp / "pi_agent";
    auto pie_dir = tmp / "pie_agent";
    fs::create_directories(ts_dir);
    std::ofstream(ts_dir / "settings.json") << "{}";

    auto result = pie::settings::FirstRunImport::run(pie_dir, ts_dir);
    REQUIRE(result.has_value());
    REQUIRE(*result == true);
    REQUIRE(fs::exists(pie_dir / ".import-from-pi.json"));

    // Second run should be no-op
    auto result2 = pie::settings::FirstRunImport::run(pie_dir, ts_dir);
    REQUIRE(result2.has_value());
    REQUIRE(*result2 == false);

    fs::remove_all(tmp);
}
