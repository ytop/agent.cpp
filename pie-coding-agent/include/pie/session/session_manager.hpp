#pragma once

#include "pie/core/result.hpp"
#include "pie/core/utils.hpp"
#include "pie/session/session_file.hpp"
#include "pie/session/session_tree.hpp"
#include <filesystem>
#include <optional>
#include <string>
#include <vector>

namespace pie::session {

struct SessionInfo {
    std::string id;
    std::string name;
    std::filesystem::path path;
    std::string created_at;
};

class SessionManager {
public:
    static Result<SessionManager> create(const std::filesystem::path& session_dir);
    static Result<SessionManager> open(const std::filesystem::path& path);
    static SessionManager in_memory();

    // Append a user message, returns entry ID
    std::string append_user_message(const std::string& content);
    std::string append_assistant_message(const std::string& content);
    std::string append_tool_call(const std::string& name, const core::JsonValue& params);
    std::string append_tool_result(const std::string& call_id, const core::JsonValue& result);

    // Tree navigation
    SessionTree& tree() { return tree_; }
    const SessionTree& tree() const { return tree_; }
    std::vector<core::JsonValue> build_session_context() const { return tree_.build_session_context(); }

    // Session listing
    static std::vector<SessionInfo> list(const std::filesystem::path& session_dir);

    const std::string& session_id() const { return session_id_; }
    const std::filesystem::path& path() const { return file_ ? file_->path() : empty_path_; }

private:
    SessionManager() = default;
    std::string append_entry(core::JsonValue entry);

    std::string session_id_;
    std::optional<SessionFile> file_;
    SessionTree tree_;
    std::string last_id_;
    static inline const std::filesystem::path empty_path_{};
};

}  // namespace pie::session
