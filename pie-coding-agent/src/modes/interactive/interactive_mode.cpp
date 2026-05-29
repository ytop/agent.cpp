#include "pie/modes/modes.hpp"
#include "pie/tui/tui_runtime.hpp"
#include "pie/cli/slash_commands.hpp"
#include <iostream>

namespace pie::modes {

int InteractiveMode::run(AgentSessionRuntime& runtime, const cli::CliInvocation&) {
    tui::TuiRuntime tui_rt;

    tui_rt.set_on_submit([&](const std::string& input) {
        if (input.empty()) return;

        if (input[0] == '/') {
            cli::CommandRegistry::instance().execute_command(runtime, input);
        } else {
            // Spawn a task to prompt the agent
            asio::io_context ctx;
            asio::co_spawn(ctx, [sess = runtime.session, input]() -> asio::awaitable<void> {
                co_await sess->prompt(input);
                co_return;
            }, [](std::exception_ptr ex) {
                if (ex) std::rethrow_exception(ex);
            });
            ctx.run();
        }
    });

    tui_rt.set_on_key([&](const std::string&) {
        // Handle global TUI shortcuts via KeybindingRegistry
    });

    // Seed TUI with any existing session messages
    for (const auto& msg : runtime.session->messages()) {
        tui_rt.append_message(msg);
    }

    tui_rt.run();
    return 0;
}

} // namespace pie::modes
