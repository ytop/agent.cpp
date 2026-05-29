#pragma once

#include "pie/core/json.hpp"
#include "pie/core/result.hpp"
#include <map>
#include <optional>
#include <string>
#include <vector>

namespace pie::session {

struct TreeNode {
    std::string id;
    std::string parent_id;
    core::JsonValue entry;
    std::vector<std::string> children;
};

class SessionTree {
public:
    SessionTree() = default;

    void append(const std::string& id, const std::string& parent_id, const core::JsonValue& entry);
    void set_leaf(const std::string& id) { leaf_id_ = id; }

    const std::string& leaf_id() const { return leaf_id_; }
    std::optional<TreeNode> get(const std::string& id) const;
    std::vector<std::string> branch_to_root(const std::string& id) const;
    std::vector<std::string> children(const std::string& id) const;

    // Build session context: walk leaf→root, collect messages
    std::vector<core::JsonValue> build_session_context() const;

    size_t size() const { return nodes_.size(); }

private:
    std::map<std::string, TreeNode> nodes_;
    std::string leaf_id_;
};

}  // namespace pie::session
