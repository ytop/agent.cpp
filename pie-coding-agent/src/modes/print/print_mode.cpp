#include "pie/modes/modes.hpp"
#include <iostream>
#include <unistd.h>

namespace pie::modes {

int PrintMode::run(AgentSession& session, const std::string& initial_prompt) {
    std::string final_prompt = initial_prompt;

    #ifdef _WIN32
        bool is_stdin_tty = true;
    #else
        bool is_stdin_tty = (isatty(fileno(stdin)) != 0);
    #endif

    if (!is_stdin_tty) {
        std::string stdin_content;
        std::string line;
        while (std::getline(std::cin, line)) {
            stdin_content += line + "\n";
        }
        if (!stdin_content.empty()) {
            // Trim trailing newline if present
            if (stdin_content.back() == '\n') {
                stdin_content.pop_back();
            }
            if (!final_prompt.empty()) {
                final_prompt = stdin_content + "\n" + final_prompt;
            } else {
                final_prompt = stdin_content;
            }
        }
    }

    // Subscribe to stream the response text
    auto sub = session.subscribe([](const AgentSessionEvent& ev) {
        if (ev.value("type", "") == "message_delta") {
            std::cout << ev.value("content", "") << std::flush;
        }
    });

    asio::io_context ctx;
    bool success = false;
    std::string error_msg;

    try {
        auto run_prompt = [&]() -> asio::awaitable<void> {
            auto res = co_await session.prompt(final_prompt);
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
        std::cerr << "error [PrintMode]: " << error_msg << "\n";
        return 1;
    }

    std::cout << "\n";
    return 0;
}

} // namespace pie::modes
