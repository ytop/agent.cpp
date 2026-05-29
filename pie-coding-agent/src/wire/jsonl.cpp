#include "pie/wire/jsonl.hpp"
#include <sstream>

namespace pie::wire {

std::pair<std::vector<JsonlEntry>, std::vector<JsonlError>>
JsonlParser::parse(const std::string& content) {
    std::vector<JsonlEntry> entries;
    std::vector<JsonlError> errors;
    std::istringstream stream(content);
    std::string line;
    int line_no = 0;

    while (std::getline(stream, line)) {
        ++line_no;
        if (line.empty()) continue;

        try {
            entries.push_back({line_no, core::JsonValue::parse(line)});
        } catch (const std::exception& e) {
            errors.push_back({line_no, e.what()});
        }
    }
    return {std::move(entries), std::move(errors)};
}

std::string JsonlSerializer::serialize_line(const core::JsonValue& val) {
    return val.dump(-1) + "\n";
}

std::string JsonlSerializer::serialize(const std::vector<core::JsonValue>& values) {
    std::string out;
    for (const auto& v : values) {
        out += serialize_line(v);
    }
    return out;
}

}  // namespace pie::wire
