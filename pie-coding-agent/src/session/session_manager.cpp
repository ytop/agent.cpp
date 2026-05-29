#include "pie/session/session_manager.hpp"

namespace pie::session {

Result<SessionManager> SessionManager::create(const std::filesystem::path& session_dir) {
    SessionManager sm;
    sm.session_id_ = core::uuid_v4();
    auto path = session_dir / (sm.session_id_ + ".jsonl");
    auto file = SessionFile::create(path);
    if (!file) return std::unexpected(file.error());
    sm.file_ = std::move(*file);
    return sm;
}

Result<SessionManager> SessionManager::open(const std::filesystem::path& path) {
    SessionManager sm;
    auto file = SessionFile::open(path);
    if (!file) return std::unexpected(file.error());
    sm.file_ = std::move(*file);

    // Rebuild tree from entries
    for (const auto& entry : sm.file_->entries()) {
        if (entry.value.contains("id")) {
            auto id = entry.value["id"].get<std::string>();
            auto parent = entry.value.value("parentId", "");
            sm.tree_.append(id, parent, entry.value);
            sm.last_id_ = id;
        }
    }

    // Extract session ID from filename
    sm.session_id_ = path.stem().string();
    return sm;
}

SessionManager SessionManager::in_memory() {
    SessionManager sm;
    sm.session_id_ = core::uuid_v4();
    return sm;
}

std::string SessionManager::append_entry(core::JsonValue entry) {
    auto id = core::hex_id();
    entry["id"] = id;
    entry["parentId"] = last_id_;
    entry["timestamp"] = core::iso8601_now();

    tree_.append(id, last_id_, entry);
    if (file_) file_->append(entry);
    last_id_ = id;
    return id;
}

std::string SessionManager::append_user_message(const std::string& content) {
    return append_entry({{"type", "message"}, {"role", "user"}, {"content", content}});
}

std::string SessionManager::append_assistant_message(const std::string& content) {
    return append_entry({{"type", "message"}, {"role", "assistant"}, {"content", content}});
}

std::string SessionManager::append_tool_call(const std::string& name, const core::JsonValue& params) {
    return append_entry({{"type", "tool_call"}, {"name", name}, {"parameters", params}});
}

std::string SessionManager::append_tool_result(const std::string& call_id, const core::JsonValue& result) {
    return append_entry({{"type", "tool_result"}, {"callId", call_id}, {"result", result}});
}

std::vector<SessionInfo> SessionManager::list(const std::filesystem::path& session_dir) {
    std::vector<SessionInfo> sessions;
    if (!std::filesystem::is_directory(session_dir)) return sessions;

    for (auto& entry : std::filesystem::directory_iterator(session_dir)) {
        if (entry.path().extension() == ".jsonl") {
            sessions.push_back({
                entry.path().stem().string(),
                "",
                entry.path(),
                ""
            });
        }
    }
    return sessions;
}

}  // namespace pie::session
