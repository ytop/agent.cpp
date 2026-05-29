#pragma once

#include "pie/core/json.hpp"
#include <functional>
#include <utility>

namespace pie {

enum class ThinkingLevel {
    Off,
    Low,
    Medium,
    High
};

using AgentSessionEvent = core::JsonValue;

class Subscription {
public:
    Subscription() = default;
    Subscription(std::function<void()> unsubscribe_fn) : unsubscribe_fn_(std::move(unsubscribe_fn)) {}
    ~Subscription() { reset(); }

    Subscription(const Subscription&) = delete;
    Subscription& operator=(const Subscription&) = delete;

    Subscription(Subscription&& other) noexcept : unsubscribe_fn_(std::move(other.unsubscribe_fn_)) {}
    Subscription& operator=(Subscription&& other) noexcept {
        if (this != &other) {
            reset();
            unsubscribe_fn_ = std::move(other.unsubscribe_fn_);
        }
        return *this;
    }

    void reset() {
        if (unsubscribe_fn_) {
            unsubscribe_fn_();
            unsubscribe_fn_ = nullptr;
        }
    }

private:
    std::function<void()> unsubscribe_fn_;
};

} // namespace pie
