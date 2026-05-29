#pragma once

#include "pie/core/json.hpp"
#include "pie/core/result.hpp"
#include "pie/agent/events.hpp"
#include <filesystem>
#include <optional>
#include <string>
#include <vector>

namespace pie::cli {

struct CliInvocation {
    enum class Mode {
        Interactive,
        Print,
        Json,
        Rpc,
        Export,
        PackageInstall,
        PackageRemove,
        PackageUpdate,
        PackageList,
        PackageConfig,
        ListModels,
        Help,
        Version,
        ConflictError
    };

    Mode mode = Mode::Interactive;

    // Mode-specific
    bool print = false;
    bool mode_json = false;
    bool mode_rpc = false;
    std::optional<std::filesystem::path> export_in;
    std::optional<std::filesystem::path> export_out;

    // Model
    std::optional<std::string> provider;
    std::optional<std::string> model;
    std::optional<std::string> api_key;
    std::optional<std::string> thinking;
    std::optional<std::string> models_csv;
    std::optional<std::string> list_models_search;

    // Session
    bool continue_recent = false;
    bool resume = false;
    std::optional<std::string> session;
    std::optional<std::string> fork;
    std::optional<std::filesystem::path> session_dir;
    bool no_session = false;

    // Tools
    std::vector<std::string> tools;
    bool no_builtin_tools = false;
    bool no_tools = false;

    // Resources
    std::vector<std::string> extensions;
    bool no_extensions = false;
    std::vector<std::string> skills;
    bool no_skills = false;
    std::vector<std::string> prompt_templates;
    bool no_prompt_templates = false;
    std::vector<std::string> themes;
    bool no_themes = false;
    bool no_context_files = false;

    // System prompt
    std::optional<std::string> system_prompt;
    std::optional<std::string> append_system_prompt;

    // Misc
    bool verbose = false;
    bool offline = false;
    bool show_version = false;

    // Positional / @files
    std::vector<std::filesystem::path> at_files;
    std::vector<std::string> message_tokens;

    // Package subcommand args
    std::vector<std::string> package_sources;
    bool package_local = false;
    bool package_self = false;
    bool package_force = false;
    bool package_self_only = false;
    bool package_extensions_only = false;
};

// Main entry point for CLI parsing
Result<CliInvocation> parse_args(int argc, char* argv[]);

// Mode detection utility
CliInvocation::Mode detect_mode(const CliInvocation& cli);

} // namespace pie::cli
