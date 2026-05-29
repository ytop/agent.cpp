#include "pie/cli/slash_commands.hpp"
#include <iostream>
#include <sstream>
#include <algorithm>

namespace pie::cli {

CommandRegistry& CommandRegistry::instance() {
    static CommandRegistry reg;
    return reg;
}

CommandRegistry::CommandRegistry() {
    register_builtins();
}

void CommandRegistry::register_builtin(SlashCommand cmd) {
    builtins_[cmd.name] = std::move(cmd);
}

void CommandRegistry::register_extension(SlashCommand cmd) {
    extensions_[cmd.name] = std::move(cmd);
}

void CommandRegistry::register_prompt_template(SlashCommand cmd) {
    templates_[cmd.name] = std::move(cmd);
}

bool CommandRegistry::execute_command(AgentSessionRuntime& runtime, const std::string& raw_line) {
    if (raw_line.empty() || raw_line[0] != '/') {
        return false;
    }

    std::string line = raw_line.substr(1); // Strip leading '/'
    std::string name;
    std::string args;

    size_t space_idx = line.find(' ');
    if (space_idx != std::string::npos) {
        name = line.substr(0, space_idx);
        args = line.substr(space_idx + 1);
        // Trim leading space from args
        size_t first_non_space = args.find_first_not_of(" \t");
        if (first_non_space != std::string::npos) {
            args = args.substr(first_non_space);
        } else {
            args.clear();
        }
    } else {
        name = line;
    }

    // Resolve name collisions: built-in > extension > prompt-template
    auto it_builtin = builtins_.find(name);
    if (it_builtin != builtins_.end()) {
        it_builtin->second.execute(runtime, args);
        return true;
    }

    auto it_ext = extensions_.find(name);
    if (it_ext != extensions_.end()) {
        it_ext->second.execute(runtime, args);
        return true;
    }

    auto it_temp = templates_.find(name);
    if (it_temp != templates_.end()) {
        it_temp->second.execute(runtime, args);
        return true;
    }

    std::cerr << "error: unrecognized command /" << name << "\n";
    return false;
}

std::vector<SlashCommand> CommandRegistry::get_all_commands() const {
    std::vector<SlashCommand> cmds;
    for (auto const& [name, cmd] : builtins_) cmds.push_back(cmd);
    for (auto const& [name, cmd] : extensions_) {
        if (builtins_.find(name) == builtins_.end()) {
            cmds.push_back(cmd);
        }
    }
    for (auto const& [name, cmd] : templates_) {
        if (builtins_.find(name) == builtins_.end() && extensions_.find(name) == extensions_.end()) {
            cmds.push_back(cmd);
        }
    }
    return cmds;
}

void CommandRegistry::register_builtins() {
    register_builtin({
        "login", "Subscription OAuth provider login", "[provider]",
        [](AgentSessionRuntime&, const std::string&) {
            std::cout << "[command] /login initiated\n";
        }
    });

    register_builtin({
        "logout", "Log out of provider authentication", "[provider]",
        [](AgentSessionRuntime&, const std::string&) {
            std::cout << "[command] /logout completed\n";
        }
    });

    register_builtin({
        "model", "Switch model", "[model-name]",
        [](AgentSessionRuntime&, const std::string&) {
            std::cout << "[command] /model completed\n";
        }
    });

    register_builtin({
        "scoped-models", "Configure models cycling subset", "[models-list]",
        [](AgentSessionRuntime&, const std::string&) {
            std::cout << "[command] /scoped-models completed\n";
        }
    });

    register_builtin({
        "settings", "Modify global/project settings", "[key] [value]",
        [](AgentSessionRuntime&, const std::string&) {
            std::cout << "[command] /settings completed\n";
        }
    });

    register_builtin({
        "resume", "Resume recent session", "[session-id]",
        [](AgentSessionRuntime&, const std::string&) {
            std::cout << "[command] /resume completed\n";
        }
    });

    register_builtin({
        "new", "Start a new session", "",
        [](AgentSessionRuntime&, const std::string&) {
            std::cout << "[command] /new completed\n";
        }
    });

    register_builtin({
        "name", "Rename current session", "<new-name>",
        [](AgentSessionRuntime&, const std::string&) {
            std::cout << "[command] /name completed\n";
        }
    });

    register_builtin({
        "session", "Show/select sessions list", "[search-term]",
        [](AgentSessionRuntime&, const std::string&) {
            std::cout << "[command] /session completed\n";
        }
    });

    register_builtin({
        "tree", "Show current session conversation tree", "",
        [](AgentSessionRuntime&, const std::string&) {
            std::cout << "[command] /tree completed\n";
        }
    });

    register_builtin({
        "fork", "Fork current session to a new branch", "[branch-name]",
        [](AgentSessionRuntime&, const std::string&) {
            std::cout << "[command] /fork completed\n";
        }
    });

    register_builtin({
        "clone", "Clone current session fully", "",
        [](AgentSessionRuntime&, const std::string&) {
            std::cout << "[command] /clone completed\n";
        }
    });

    register_builtin({
        "compact", "Compact previous context manually", "[prompt-instructions]",
        [](AgentSessionRuntime&, const std::string&) {
            std::cout << "[command] /compact completed\n";
        }
    });

    register_builtin({
        "copy", "Copy last assistant response or selection to clipboard", "",
        [](AgentSessionRuntime&, const std::string&) {
            std::cout << "[command] /copy completed\n";
        }
    });

    register_builtin({
        "export", "Export session context to static self-contained HTML", "[file-path]",
        [](AgentSessionRuntime&, const std::string&) {
            std::cout << "[command] /export completed\n";
        }
    });

    register_builtin({
        "share", "Share session context as an anonymous private GitHub gist", "",
        [](AgentSessionRuntime&, const std::string&) {
            std::cout << "[command] /share completed\n";
        }
    });

    register_builtin({
        "reload", "Reload configuration files hot-swap", "",
        [](AgentSessionRuntime&, const std::string&) {
            std::cout << "[command] /reload completed\n";
        }
    });

    register_builtin({
        "hotkeys", "Display list of active keybindings shortcuts", "",
        [](AgentSessionRuntime&, const std::string&) {
            std::cout << "[command] /hotkeys completed\n";
        }
    });

    register_builtin({
        "changelog", "Display latest application changes and version releases", "",
        [](AgentSessionRuntime&, const std::string&) {
            std::cout << "[command] /changelog completed\n";
        }
    });

    register_builtin({
        "quit", "Exit application", "",
        [](AgentSessionRuntime&, const std::string&) {
            std::cout << "[command] /quit initiated\n";
            std::exit(0);
        }
    });
}

} // namespace pie::cli
