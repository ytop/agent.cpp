#include "pie/modes/modes.hpp"
#include <iostream>

namespace pie::modes {

int JsonMode::run(AgentSession& session, const std::string& initial_prompt) {
    // Subscribe to all session events and print them as JSONL on stdout
    auto sub = session.subscribe([](const AgentSessionEvent& ev) {
        std::cout << core::to_json_string(ev) << "\n" << std::flush;
    });

    asio::io_context ctx;
    bool success = false;
    std::string error_msg;

    try {
        auto run_prompt = [&]() -> asio::awaitable<void> {
            auto res = co_await session.prompt(initial_prompt);
            if (res.has_value()) {
                success = true;
            } else {
                error_msg = res.error();
            }
            co_return;
        };

        asio::co_spawn(ctx, run_prompt(), [](std::exception_ptr ex) {
            if (ex) std::rethrow_exception(ex);
        });

        ctx.run();
    } catch (const std::exception& e) {
        error_msg = e.what();
    }

    if (!success) {
        std::cerr << "error [JsonMode]: " << error_msg << "\n";
        return 1;
    }

    return 0;
}

} // namespace pie::modes
