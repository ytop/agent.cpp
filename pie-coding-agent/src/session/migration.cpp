#include "pie/session/migration.hpp"
#include "pie/core/utils.hpp"

namespace pie::session {

Result<void> migrate_v1_to_v3(SessionFile& file) {
    std::vector<core::JsonValue> migrated;
    migrated.push_back({{"type", "session_header"}, {"version", 3}});

    std::string prev_id;
    for (const auto& entry : file.entries()) {
        if (entry.value.contains("type") && entry.value["type"] == "session_header")
            continue;

        auto e = entry.value;
        auto id = core::hex_id();
        e["id"] = id;
        e["parentId"] = prev_id.empty() ? core::JsonValue(nullptr) : core::JsonValue(prev_id);
        migrated.push_back(e);
        prev_id = id;
    }

    return file.rewrite(migrated);
}

Result<void> migrate_v2_to_v3(SessionFile& file) {
    std::vector<core::JsonValue> migrated;

    for (const auto& entry : file.entries()) {
        auto e = entry.value;
        if (e.contains("type") && e["type"] == "session_header") {
            e["version"] = 3;
        }
        if (e.contains("role") && e["role"] == "hookMessage") {
            e["role"] = "custom";
        }
        migrated.push_back(e);
    }

    return file.rewrite(migrated);
}

Result<void> migrate_if_needed(SessionFile& file) {
    if (file.version() == 3) return {};
    if (file.version() > 3)
        return std::unexpected("unsupported session version: " + std::to_string(file.version()));
    if (file.version() == 1) return migrate_v1_to_v3(file);
    if (file.version() == 2) return migrate_v2_to_v3(file);
    return migrate_v1_to_v3(file);  // version 0 or missing treated as v1
}

}  // namespace pie::session
