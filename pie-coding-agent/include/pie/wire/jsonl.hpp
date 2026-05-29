#pragma once

#include "pie/core/json.hpp"
#include "pie/core/result.hpp"
#include <string>
#include <vector>

namespace pie::wire {

struct JsonlEntry {
    int line_no;
    core::JsonValue value;
};

struct JsonlError {
    int line_no;
    std::string message;
};

class JsonlParser {
public:
    // Parse JSONL content, returning entries and errors separately
    static std::pair<std::vector<JsonlEntry>, std::vector<JsonlError>>
    parse(const std::string& content);
};

class JsonlSerializer {
public:
    // Serialize a single JsonValue to a JSONL line (compact, \n terminated)
    static std::string serialize_line(const core::JsonValue& val);

    // Serialize multiple values
    static std::string serialize(const std::vector<core::JsonValue>& values);
};

}  // namespace pie::wire
