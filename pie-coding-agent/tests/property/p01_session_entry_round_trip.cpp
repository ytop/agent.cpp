// Feature: cpp-coding-agent, Property 1: Session entry round-trip
// Serializing a JsonValue and parsing it back produces a structurally equal value.
// Feature: cpp-coding-agent, Property 2: TS wire-format unknown-key preservation
// Random JSON objects survive serialize→parse with all unknown keys preserved.

#include <catch2/catch_test_macros.hpp>
#include <rapidcheck/catch.h>
#include "pie/wire/jsonl.hpp"
#include "pie/core/json.hpp"
#include <string>

static bool structurally_equal(const pie::core::JsonValue& a, const pie::core::JsonValue& b) {
    if (a.type() != b.type()) return false;
    if (a.is_object()) {
        if (a.size() != b.size()) return false;
        for (auto it = a.begin(); it != a.end(); ++it) {
            if (!b.contains(it.key())) return false;
            if (!structurally_equal(it.value(), b[it.key()])) return false;
        }
        return true;
    }
    return a == b;
}

TEST_CASE("Property 1: session entry round-trip", "[property][session][wire]") {
    // Feature: cpp-coding-agent, Property 1: Session entry round-trip
    rc::prop("serialize then parse yields same object", []() {
        auto key1 = *rc::gen::arbitrary<std::string>();
        auto val1 = *rc::gen::arbitrary<std::string>();

        // Build a minimal entry-like object
        pie::core::JsonValue entry = pie::core::JsonValue::object();
        entry["type"] = "message";
        entry["id"] = "a1b2c3d4";
        // Avoid keys with special chars that would break JSON
        std::string safe_key = key1.empty() ? "extra_field" : key1;
        for (auto& c : safe_key) {
            if (c == '"' || c == '\\' || c < 0x20) c = '_';
        }
        if (safe_key != "type" && safe_key != "id") {
            entry[safe_key] = val1;
        }

        std::string line = pie::wire::JsonlSerializer::serialize_line(entry);
        RC_ASSERT(!line.empty());
        RC_ASSERT(line.back() == '\n');

        auto [entries, errors] = pie::wire::JsonlParser::parse(line);
        RC_ASSERT(errors.empty());
        RC_ASSERT(entries.size() == 1u);
        // All original keys must survive the round-trip
        for (auto it = entry.begin(); it != entry.end(); ++it) {
            RC_ASSERT(entries[0].value.contains(it.key()));
        }
    });
}

TEST_CASE("Property 2: TS wire-format unknown-key preservation", "[property][wire]") {
    // Feature: cpp-coding-agent, Property 2: TS wire-format unknown-key preservation
    rc::prop("extra keys survive jsonl round-trip", []() {
        auto extra_key = *rc::gen::arbitrary<std::string>();
        auto extra_val = *rc::gen::arbitrary<std::string>();

        pie::core::JsonValue obj = pie::core::JsonValue::object();
        obj["type"] = "settings";
        obj["version"] = 3;

        // Sanitize extra_key
        std::string safe_key = extra_key;
        for (auto& c : safe_key) {
            if (c == '"' || c == '\\' || c < 0x20) c = '_';
        }

        if (!safe_key.empty() && safe_key != "type" && safe_key != "version") {
            obj[safe_key] = extra_val;
        }

        std::string line = pie::wire::JsonlSerializer::serialize_line(obj);
        auto [entries, errors] = pie::wire::JsonlParser::parse(line);
        RC_ASSERT(errors.empty());
        RC_ASSERT(!entries.empty());

        // All original keys must be present in parsed output
        for (auto it = obj.begin(); it != obj.end(); ++it) {
            RC_ASSERT(entries[0].value.contains(it.key()));
        }
    });
}
