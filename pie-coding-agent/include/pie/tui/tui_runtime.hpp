#pragma once

#include "pie/core/json.hpp"
#include "pie/core/result.hpp"
#include "pie/resources/theme_manager.hpp"
#include <functional>
#include <string>
#include <vector>

namespace pie::tui {

// TUI is stubbed until FTXUI is available. The interface is defined
// so downstream code can compile against it.

struct EditorState {
    std::string text;
    std::vector<std::string> attachments;
};

using SubmitCallback = std::function<void(const std::string& input)>;
using KeyCallback = std::function<void(const std::string& key)>;

class TuiRuntime {
public:
    TuiRuntime() = default;

    void apply_theme(const resources::Theme& theme) { theme_ = theme; }
    void set_on_submit(SubmitCallback cb) { on_submit_ = std::move(cb); }
    void set_on_key(KeyCallback cb) { on_key_ = std::move(cb); }

    void append_message(const core::JsonValue& msg) { messages_.push_back(msg); }
    void set_streaming(bool s) { streaming_ = s; }
    void show_modal(const std::string& /*name*/) {}

    // In a real implementation this would run the FTXUI event loop
    void run() {}
    void stop() { running_ = false; }

    bool running() const { return running_; }
    const std::vector<core::JsonValue>& messages() const { return messages_; }

private:
    resources::Theme theme_;
    SubmitCallback on_submit_;
    KeyCallback on_key_;
    std::vector<core::JsonValue> messages_;
    bool streaming_ = false;
    bool running_ = true;
};

}  // namespace pie::tui
