#include "pie/cli/cli_invocation.hpp"
#include "pie/cli/keybindings.hpp"
#include "pie/modes/modes.hpp"
#include "pie/agent/agent_session.hpp"
#include "pie/agent/agent_session_runtime.hpp"
#include "pie/settings/settings_manager.hpp"
#include "pie/auth/auth_storage.hpp"
#include "pie/sdk/package_manager.hpp"
#include "pie/io/image_pipeline.hpp"
#include <iostream>
#include <fstream>
#include <filesystem>
#include <algorithm>

static constexpr const char* kVersion = "pie 0.1.0";

std::string build_final_prompt(const pie::cli::CliInvocation& cli) {
    std::string prompt;
    for (const auto& p : cli.at_files) {
        std::string ext = p.extension().string();
        std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
        if (ext == ".png" || ext == ".jpg" || ext == ".jpeg" || ext == ".gif" || ext == ".webp") {
            std::ifstream f(p, std::ios::binary);
            std::vector<uint8_t> bytes((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
            std::string b64 = pie::io::ImagePipeline::base64_encode(bytes);
            std::string mime = "image/png";
            if (ext == ".jpg" || ext == ".jpeg") mime = "image/jpeg";
            else if (ext == ".gif") mime = "image/gif";
            else if (ext == ".webp") mime = "image/webp";
            
            prompt += "[image: " + mime + ";base64," + b64 + "]\n";
        } else {
            std::ifstream f(p);
            std::string content((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
            prompt += "<<<file: " + p.string() + ">>>\n" + content + "\n<<<end>>>\n";
        }
    }

    std::string msgs;
    for (size_t i = 0; i < cli.message_tokens.size(); ++i) {
        if (i > 0) msgs += " ";
        msgs += cli.message_tokens[i];
    }

    if (!prompt.empty()) {
        if (!msgs.empty()) {
            return prompt + msgs;
        } else {
            return prompt;
        }
    }
    return msgs;
}

int main(int argc, char* argv[]) {
    auto parse_res = pie::cli::parse_args(argc, argv);
    if (!parse_res) {
        std::cerr << parse_res.error() << "\n";
        return 2; // Exit code 2 for CLI arg parse/unknown errors
    }

    const auto& cli = *parse_res;

    if (cli.mode == pie::cli::CliInvocation::Mode::Help) {
        return EXIT_SUCCESS;
    }

    if (cli.mode == pie::cli::CliInvocation::Mode::Version) {
        std::cout << kVersion << "\n";
        return EXIT_SUCCESS;
    }

    if (cli.mode == pie::cli::CliInvocation::Mode::ConflictError) {
        std::cerr << "error: conflicting mode options specified.\n"
                  << "Ensure at most one of --print, --mode json, --mode rpc, or --export is set.\n";
        return 2; // Conflict errors are exit code 2
    }

    // Resolve directories & environment
    pie::settings::EnvResolver env;
    auto agent_dir = env.agent_dir();
    std::filesystem::create_directories(agent_dir);

    // Initialize Settings Manager & Auth Storage
    auto settings_res = pie::settings::SettingsManager::create(agent_dir / "config.json");
    auto settings_mgr = settings_res
        ? std::make_shared<pie::settings::SettingsManager>(std::move(*settings_res))
        : std::make_shared<pie::settings::SettingsManager>(pie::settings::SettingsManager::in_memory());

    auto auth_res = pie::auth::AuthStorage::create(agent_dir);
    auto auth_storage = auth_res
        ? std::make_shared<pie::auth::AuthStorage>(std::move(*auth_res))
        : std::make_shared<pie::auth::AuthStorage>(pie::auth::AuthStorage::in_memory());

    // Apply CLI credentials overrides
    if (cli.api_key && cli.provider) {
        auth_storage->set_runtime_api_key(*cli.provider, *cli.api_key);
    }

    // Load keybindings
    auto keybindings_path = agent_dir / "keybindings.json";
    if (std::filesystem::exists(keybindings_path)) {
        std::ifstream f(keybindings_path);
        std::string content((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
        auto parsed_json = pie::core::parse_json(content);
        if (parsed_json) {
            pie::cli::KeybindingRegistry::instance().load_from_json(*parsed_json, keybindings_path.string());
        }
    }

    // Initialize Package Manager for subcommands
    pie::sdk::packages::PackageManager pm(env.package_dir(), std::filesystem::current_path());
    pm.set_offline(cli.offline);

    // Subcommand routing
    switch (cli.mode) {
        case pie::cli::CliInvocation::Mode::PackageInstall: {
            if (cli.package_self) {
                auto res = pm.install(".");
                if (!res) {
                    std::cerr << "error: package self-install failed: " << res.error() << "\n";
                    return 4;
                }
            } else {
                for (const auto& src : cli.package_sources) {
                    auto res = pm.install(src);
                    if (!res) {
                        std::cerr << "error: package installation failed: " << res.error() << "\n";
                        return 4; // Exit 4 for package manager errors
                    }
                }
            }
            return EXIT_SUCCESS;
        }
        case pie::cli::CliInvocation::Mode::PackageRemove: {
            for (const auto& src : cli.package_sources) {
                auto res = pm.remove(src);
                if (!res) {
                    std::cerr << "error: package removal failed: " << res.error() << "\n";
                    return 4;
                }
            }
            return EXIT_SUCCESS;
        }
        case pie::cli::CliInvocation::Mode::PackageUpdate: {
            if (cli.package_sources.empty()) {
                auto res = pm.update();
                if (!res) {
                    std::cerr << "error: package update failed: " << res.error() << "\n";
                    return 4;
                }
            } else {
                for (const auto& src : cli.package_sources) {
                    auto res = pm.update(src);
                    if (!res) {
                        std::cerr << "error: package update failed: " << res.error() << "\n";
                        return 4;
                    }
                }
            }
            return EXIT_SUCCESS;
        }
        case pie::cli::CliInvocation::Mode::PackageList: {
            auto list = pm.list();
            for (const auto& pkg : list) {
                std::cout << pkg.name << " (" << pkg.version << ") - " << pkg.path.string() << "\n";
            }
            return EXIT_SUCCESS;
        }
        case pie::cli::CliInvocation::Mode::PackageConfig: {
            std::cout << "Package Manager Config:\n";
            std::cout << "  Global Store: " << env.package_dir().string() << "\n";
            return EXIT_SUCCESS;
        }
        case pie::cli::CliInvocation::Mode::Export: {
            return pie::modes::ExportMode{}.run(*cli.export_in, cli.export_out);
        }
        default:
            break;
    }

    // Initialize Session Manager
    std::shared_ptr<pie::session::SessionManager> session_mgr;
    if (cli.no_session) {
        session_mgr = std::make_shared<pie::session::SessionManager>(pie::session::SessionManager::in_memory());
    } else if (cli.session) {
        auto open_res = pie::session::SessionManager::open(*cli.session);
        if (open_res) {
            session_mgr = std::make_shared<pie::session::SessionManager>(std::move(*open_res));
        } else {
            std::cerr << "error: failed to open session: " << open_res.error() << "\n";
            return 2;
        }
    } else {
        auto create_res = pie::session::SessionManager::create(env.session_dir());
        if (create_res) {
            session_mgr = std::make_shared<pie::session::SessionManager>(std::move(*create_res));
        } else {
            session_mgr = std::make_shared<pie::session::SessionManager>(pie::session::SessionManager::in_memory());
        }
    }

    // Initialize core agent containers
    auto agent = std::make_shared<pie::Agent>();
    agent->settings_manager = settings_mgr;
    agent->auth_storage = auth_storage;
    agent->model_registry = std::make_shared<pie::models::ModelRegistry>();
    agent->resource_loader = std::make_shared<pie::resources::ResourceLoader>(agent_dir, std::filesystem::current_path());
    agent->tool_host = std::make_shared<pie::tools::ToolHost>();

    pie::AgentSessionRuntime runtime(agent, session_mgr);

    // Configure overrides (model, thinking, etc.)
    if (cli.model) {
        pie::models::ModelInfo info{*cli.model, cli.provider.value_or("default"), *cli.model, 200000, true};
        asio::io_context ctx;
        asio::co_spawn(ctx, [&]() -> asio::awaitable<void> {
            co_await runtime.session->set_model(info);
            co_return;
        }, [](std::exception_ptr ex) {
            if (ex) std::rethrow_exception(ex);
        });
        ctx.run();
    }

    if (cli.thinking) {
        pie::ThinkingLevel lvl = pie::ThinkingLevel::Medium;
        std::string lvl_str = *cli.thinking;
        if (lvl_str == "off") lvl = pie::ThinkingLevel::Off;
        else if (lvl_str == "low") lvl = pie::ThinkingLevel::Low;
        else if (lvl_str == "medium") lvl = pie::ThinkingLevel::Medium;
        else if (lvl_str == "high") lvl = pie::ThinkingLevel::High;
        runtime.session->set_thinking_level(lvl);
    }

    // Mode Dispatching
    std::string prompt = build_final_prompt(cli);

    if (cli.mode == pie::cli::CliInvocation::Mode::ListModels) {
        auto models = agent->model_registry->all();
        std::string search = cli.list_models_search.value_or("");
        std::transform(search.begin(), search.end(), search.begin(), ::tolower);
        for (const auto& m : models) {
            std::string id_lower = m.id;
            std::transform(id_lower.begin(), id_lower.end(), id_lower.begin(), ::tolower);
            if (search.empty() || id_lower.find(search) != std::string::npos) {
                std::cout << m.provider << "/" << m.id << " (" << m.name << ")\n";
            }
        }
        return EXIT_SUCCESS;
    }

    switch (cli.mode) {
        case pie::cli::CliInvocation::Mode::Print:
            return pie::modes::PrintMode{}.run(*runtime.session, prompt);
        case pie::cli::CliInvocation::Mode::Json:
            return pie::modes::JsonMode{}.run(*runtime.session, prompt);
        case pie::cli::CliInvocation::Mode::Rpc:
            return pie::modes::RpcMode{}.run(runtime);
        case pie::cli::CliInvocation::Mode::Interactive:
            return pie::modes::InteractiveMode{}.run(runtime, cli);
        default:
            break;
    }

    return EXIT_SUCCESS;
}
