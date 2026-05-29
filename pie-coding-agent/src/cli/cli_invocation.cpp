#include "pie/cli/cli_invocation.hpp"
#include <CLI/CLI.hpp>
#include <iostream>
#include <fstream>
#include <filesystem>
#include <algorithm>

namespace pie::cli {

Result<CliInvocation> parse_args(int argc, char* argv[]) {
    CliInvocation cli;

    CLI::App app{"pie — C++20 coding agent"};
    app.allow_extras(true); // to capture unknown flags

    // Mode-specific options
    app.add_flag("-p,--print", cli.print, "Print mode");
    
    std::string mode_str;
    app.add_option("--mode", mode_str, "Execution mode (json, rpc)");

    std::vector<std::string> export_args;
    app.add_option("--export", export_args, "Export session HTML: <in> [out]")->expected(1, 2);

    // Model options
    app.add_option("--provider", cli.provider, "Model provider");
    app.add_option("--model", cli.model, "Model name");
    app.add_option("--api-key", cli.api_key, "API key override");
    app.add_option("--thinking", cli.thinking, "Thinking level");
    app.add_option("--models", cli.models_csv, "Comma-separated active models");
    
    bool list_models_requested = false;
    auto* opt_list = app.add_option("--list-models", cli.list_models_search, "List available models")->expected(0, 1);

    // Session options
    app.add_flag("-c,--continue", cli.continue_recent, "Continue most recent session");
    app.add_flag("-r,--resume", cli.resume, "Resume session");
    app.add_option("--session", cli.session, "Session UUID or path");
    app.add_option("--fork", cli.fork, "Fork session UUID or path");
    app.add_option("--session-dir", cli.session_dir, "Session directory override");
    app.add_flag("--no-session", cli.no_session, "Do not persist session");

    // Tools options
    app.add_option("--tools,-t", cli.tools, "Allowed tools list")->delimiter(',');
    app.add_flag("--no-builtin-tools,--nbt", cli.no_builtin_tools, "Disable built-in tools");
    app.add_flag("--no-tools,--nt", cli.no_tools, "Disable all tools");

    // Resources options
    app.add_option("--extension,-e", cli.extensions, "Repeatable extension path");
    app.add_flag("--no-extensions", cli.no_extensions, "Disable extensions");
    app.add_option("--skill", cli.skills, "Repeatable skill path");
    app.add_flag("--no-skills", cli.no_skills, "Disable skills");
    app.add_option("--prompt-template", cli.prompt_templates, "Repeatable prompt template");
    app.add_flag("--no-prompt-templates", cli.no_prompt_templates, "Disable prompt templates");
    app.add_option("--theme", cli.themes, "Repeatable theme name");
    app.add_flag("--no-themes", cli.no_themes, "Disable themes");
    app.add_flag("--no-context-files,--nc", cli.no_context_files, "Disable context files");

    // System prompt options
    app.add_option("--system-prompt", cli.system_prompt, "System prompt override");
    app.add_option("--append-system-prompt", cli.append_system_prompt, "Append to system prompt");

    // Misc options
    app.add_flag("--verbose", cli.verbose, "Verbose startup diagnostic output");
    app.add_flag("--offline", cli.offline, "Offline mode");
    app.add_flag("-v,--version", cli.show_version, "Print version and exit");

    // Subcommands for Package Manager
    auto* install_sub = app.add_subcommand("install", "Install package");
    install_sub->add_option("sources", cli.package_sources, "Package sources");
    install_sub->add_flag("-l,--local", cli.package_local, "Local project installation");
    install_sub->add_flag("--self", cli.package_self, "Install self");
    install_sub->add_flag("--force", cli.package_force, "Force installation");

    auto* remove_sub = app.add_subcommand("remove", "Remove package");
    remove_sub->add_option("sources", cli.package_sources, "Package sources");

    auto* uninstall_sub = app.add_subcommand("uninstall", "Uninstall package");
    uninstall_sub->add_option("sources", cli.package_sources, "Package sources");

    auto* update_sub = app.add_subcommand("update", "Update package");
    update_sub->add_option("sources", cli.package_sources, "Package sources");

    auto* list_sub = app.add_subcommand("list", "List packages");

    auto* config_sub = app.add_subcommand("config", "Configure packages");

    // Catch generic positionals
    std::vector<std::string> positionals;
    app.add_option("positionals", positionals, "Message prompts and @files");

    try {
        app.parse(argc, argv);
    } catch (const CLI::CallForHelp& e) {
        std::cout << app.help() << "\n";
        cli.mode = CliInvocation::Mode::Help;
        return cli;
    } catch (const CLI::ParseError& e) {
        // Option parsing failed (e.g. invalid arguments types, etc.)
        // In this case, app.exit(e) would print and exit, but we want to return a Result.
        std::stringstream ss;
        ss << "error: " << e.what() << "\nTry 'pie --help' for more information.";
        return std::unexpected(ss.str());
    }

    // Check for unrecognized options that allow_extras gathered
    if (!app.remaining().empty()) {
        std::string unrecognized = app.remaining()[0];
        return std::unexpected("error: unrecognized option '" + unrecognized + "'\nTry 'pie --help' for more information.");
    }

    // Set mode subcommands
    if (app.got_subcommand(install_sub)) cli.mode = CliInvocation::Mode::PackageInstall;
    else if (app.got_subcommand(remove_sub)) cli.mode = CliInvocation::Mode::PackageRemove;
    else if (app.got_subcommand(uninstall_sub)) cli.mode = CliInvocation::Mode::PackageRemove;
    else if (app.got_subcommand(update_sub)) cli.mode = CliInvocation::Mode::PackageUpdate;
    else if (app.got_subcommand(list_sub)) cli.mode = CliInvocation::Mode::PackageList;
    else if (app.got_subcommand(config_sub)) cli.mode = CliInvocation::Mode::PackageConfig;
    else if (opt_list->count() > 0 || list_models_requested) cli.mode = CliInvocation::Mode::ListModels;
    else if (cli.show_version) cli.mode = CliInvocation::Mode::Version;
    else {
        // Map mode strings
        if (mode_str == "json") cli.mode_json = true;
        if (mode_str == "rpc") cli.mode_rpc = true;

        if (!export_args.empty()) {
            cli.export_in = export_args[0];
            if (export_args.size() > 1) cli.export_out = export_args[1];
        }

        cli.mode = detect_mode(cli);
    }

    // Split positionals into message tokens vs @files
    for (const auto& token : positionals) {
        if (!token.empty() && token[0] == '@') {
            cli.at_files.push_back(token.substr(1));
        } else {
            cli.message_tokens.push_back(token);
        }
    }

    // Perform validation on @files
    for (const auto& p : cli.at_files) {
        if (!std::filesystem::exists(p)) {
            return std::unexpected("error: file '" + p.string() + "' does not exist");
        }
        std::ifstream f(p, std::ios::binary);
        if (!f.is_open()) {
            return std::unexpected("error: file '" + p.string() + "' is not readable");
        }
    }

    return cli;
}

CliInvocation::Mode detect_mode(const CliInvocation& cli) {
    int set_count = (cli.print ? 1 : 0) + (cli.mode_json ? 1 : 0)
                  + (cli.mode_rpc ? 1 : 0) + (cli.export_in.has_value() ? 1 : 0);
    if (set_count > 1) return CliInvocation::Mode::ConflictError;
    if (cli.print) return CliInvocation::Mode::Print;
    if (cli.mode_json) return CliInvocation::Mode::Json;
    if (cli.mode_rpc) return CliInvocation::Mode::Rpc;
    if (cli.export_in) return CliInvocation::Mode::Export;
    
    // TTY detection
    #ifdef _WIN32
        bool is_stdin_tty = true; // placeholder
    #else
        bool is_stdin_tty = isatty(fileno(stdin)) && isatty(fileno(stdout));
    #endif
    return is_stdin_tty ? CliInvocation::Mode::Interactive : CliInvocation::Mode::Print;
}

} // namespace pie::cli
