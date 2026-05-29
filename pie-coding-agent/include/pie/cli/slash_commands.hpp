#pragma once

#include "pie/agent/agent_session_runtime.hpp"
#include <string>
#include <functional>
#include <map>
#include <vector>

namespace pie::cli {

struct SlashCommand {
    std::string name;
    std::string description;
    std::string argument_hint;
    std::function<void(AgentSessionRuntime&, const std::string&)> execute;
};

class CommandRegistry {
public:
    static CommandRegistry& instance();

    void register_builtin(SlashCommand cmd);
    void register_extension(SlashCommand cmd);
    void register_prompt_template(SlashCommand cmd);

    bool execute_command(AgentSessionRuntime& runtime, const std::string& raw_line);
    std::vector<SlashCommand> get_all_commands() const;

private:
    CommandRegistry();
    void register_builtins();

    std::map<std::string, SlashCommand> builtins_;
    std::map<std::string, SlashCommand> extensions_;
    std::map<std::string, SlashCommand> templates_;
};

} // namespace pie::cli
