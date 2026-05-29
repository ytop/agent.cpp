#include "pie/modes/modes.hpp"
#include <iostream>
#include <sstream>
#include <thread>
#include <asio.hpp>

namespace pie::modes {

int RpcMode::run(AgentSessionRuntime& runtime) {
    // strict LF-only record framing on stdin/stdout, optional \r\n tolerance
    std::string line;

    // Subscribe to runtime session events to stream them immediately on stdout as they happen
    auto subscribe_session = [&](std::shared_ptr<AgentSession> sess) -> Subscription {
        return sess->subscribe([](const AgentSessionEvent& ev) {
            std::cout << core::to_json_string(ev) << "\n" << std::flush;
        });
    };

    auto current_sub = std::make_shared<Subscription>(subscribe_session(runtime.session));

    asio::io_context ctx;

    auto process_command = [&](const core::JsonValue& cmd) -> core::JsonValue {
        std::string req_id = cmd.value("id", "");
        std::string cmd_type = cmd.value("type", "");

        core::JsonValue resp = {
            {"type", "response"},
            {"command", cmd_type}
        };
        if (!req_id.empty()) {
            resp["id"] = req_id;
        }

        if (cmd_type == "prompt") {
            std::string msg = cmd.value("message", "");
            // Run prompt asynchronously
            asio::co_spawn(ctx, [sess = runtime.session, msg]() -> asio::awaitable<void> {
                co_await sess->prompt(msg);
                co_return;
            }, [](std::exception_ptr ex) {
                if (ex) std::rethrow_exception(ex);
            });
            resp["success"] = true;
            return resp;
        }
        else if (cmd_type == "steer") {
            std::string msg = cmd.value("message", "");
            asio::co_spawn(ctx, [sess = runtime.session, msg]() -> asio::awaitable<void> {
                co_await sess->steer(msg);
                co_return;
            }, [](std::exception_ptr ex) {
                if (ex) std::rethrow_exception(ex);
            });
            resp["success"] = true;
            return resp;
        }
        else if (cmd_type == "follow_up") {
            std::string msg = cmd.value("message", "");
            asio::co_spawn(ctx, [sess = runtime.session, msg]() -> asio::awaitable<void> {
                co_await sess->follow_up(msg);
                co_return;
            }, [](std::exception_ptr ex) {
                if (ex) std::rethrow_exception(ex);
            });
            resp["success"] = true;
            return resp;
        }
        else if (cmd_type == "abort") {
            asio::co_spawn(ctx, [sess = runtime.session]() -> asio::awaitable<void> {
                co_await sess->abort();
                co_return;
            }, [](std::exception_ptr ex) {
                if (ex) std::rethrow_exception(ex);
            });
            resp["success"] = true;
            return resp;
        }
        else if (cmd_type == "new_session") {
            asio::co_spawn(ctx, [&runtime]() -> asio::awaitable<void> {
                co_await runtime.new_session();
                co_return;
            }, [](std::exception_ptr ex) {
                if (ex) std::rethrow_exception(ex);
            });
            // Re-subscribe to the new session
            *current_sub = subscribe_session(runtime.session);

            resp["success"] = true;
            resp["data"] = {{"cancelled", false}};
            return resp;
        }
        else if (cmd_type == "get_state") {
            auto model = runtime.session->model();
            core::JsonValue model_obj = nullptr;
            if (model) {
                model_obj = {
                    {"id", model->id},
                    {"provider", model->provider},
                    {"name", model->name}
                };
            }

            resp["success"] = true;
            resp["data"] = {
                {"model", model_obj},
                {"thinkingLevel", "medium"},
                {"isStreaming", runtime.session->is_streaming()},
                {"isCompacting", false},
                {"steeringMode", "one-at-a-time"},
                {"followUpMode", "one-at-a-time"},
                {"sessionId", runtime.session->session_id()},
                {"autoCompactionEnabled", true},
                {"messageCount", runtime.session->messages().size()},
                {"pendingMessageCount", 0}
            };
            auto path = runtime.session->session_file();
            if (path) resp["data"]["sessionFile"] = path->string();
            else resp["data"]["sessionFile"] = nullptr;
            return resp;
        }
        else if (cmd_type == "get_messages") {
            resp["success"] = true;
            resp["data"] = {
                {"messages", runtime.session->messages()}
            };
            return resp;
        }
        else if (cmd_type == "set_model") {
            std::string provider = cmd.value("provider", "");
            std::string model_id = cmd.value("modelId", "");
            models::ModelInfo info{model_id, provider, model_id, 200000, true};
            asio::co_spawn(ctx, [sess = runtime.session, info]() -> asio::awaitable<void> {
                co_await sess->set_model(info);
                co_return;
            }, [](std::exception_ptr ex) {
                if (ex) std::rethrow_exception(ex);
            });
            resp["success"] = true;
            resp["data"] = {
                {"id", model_id},
                {"provider", provider}
            };
            return resp;
        }
        else if (cmd_type == "cycle_model") {
            asio::co_spawn(ctx, [sess = runtime.session]() -> asio::awaitable<void> {
                co_await sess->cycle_model();
                co_return;
            }, [](std::exception_ptr ex) {
                if (ex) std::rethrow_exception(ex);
            });
            resp["success"] = true;
            resp["data"] = nullptr;
            return resp;
        }
        else if (cmd_type == "compact") {
            std::optional<std::string> instructions;
            if (cmd.contains("customInstructions")) {
                instructions = cmd["customInstructions"].get<std::string>();
            }
            asio::co_spawn(ctx, [sess = runtime.session, instructions]() -> asio::awaitable<void> {
                co_await sess->compact(instructions);
                co_return;
            }, [](std::exception_ptr ex) {
                if (ex) std::rethrow_exception(ex);
            });
            resp["success"] = true;
            return resp;
        }
        else if (cmd_type == "get_session_stats") {
            resp["success"] = true;
            resp["data"] = {
                {"sessionId", runtime.session->session_id()},
                {"totalMessages", runtime.session->messages().size()},
                {"cost", 0.0}
            };
            return resp;
        }
        else if (cmd_type == "quit") {
            std::exit(0);
        }

        resp["success"] = false;
        resp["error"] = "unknown or unsupported command";
        return resp;
    };

    while (std::getline(std::cin, line)) {
        // Strip trailing \r if present
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }
        if (line.empty()) continue;

        auto parsed = core::parse_json(line);
        if (!parsed) {
            core::JsonValue err = {
                {"type", "response"},
                {"success", false},
                {"error", "invalid json record"}
            };
            std::cout << core::to_json_string(err) << "\n" << std::flush;
            continue;
        }

        core::JsonValue resp = process_command(*parsed);
        std::cout << core::to_json_string(resp) << "\n" << std::flush;

        // Poll/run event loop to execute any spawned coroutines
        ctx.poll();
        ctx.restart();
    }

    return 0;
}

} // namespace pie::modes
