#pragma once

#include <expected>
#include <string>

namespace pie {

template <typename T, typename E = std::string>
using Result = std::expected<T, E>;

namespace core {

enum class ErrorCategory {
    IO,
    Parse,
    Validation,
    Auth,
    Network,
    Internal,
};

struct ErrorInfo {
    ErrorCategory category;
    std::string message;
    std::string context;
};

}  // namespace core
}  // namespace pie
