#pragma once

#include <nlohmann/json.hpp>
#include <string>

namespace pie::core {

// JsonValue wraps nlohmann::ordered_json to preserve insertion order
using JsonValue = nlohmann::ordered_json;

// Parse JSON from string, returns nullopt on failure
inline std::optional<JsonValue> parse_json(const std::string& str) {
    try {
        return JsonValue::parse(str);
    } catch (...) {
        return std::nullopt;
    }
}

// Serialize JSON to string (compact, no trailing newline)
inline std::string to_json_string(const JsonValue& val) {
    return val.dump(-1);
}

// Serialize JSON to string (pretty)
inline std::string to_json_pretty(const JsonValue& val) {
    return val.dump(2);
}

}  // namespace pie::core
