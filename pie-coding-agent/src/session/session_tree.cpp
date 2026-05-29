#include "pie/session/session_tree.hpp"
#include <algorithm>

namespace pie::session {

void SessionTree::append(const std::string& id, const std::string& parent_id, const core::JsonValue& entry) {
    TreeNode node{id, parent_id, entry, {}};
    nodes_[id] = node;
    if (!parent_id.empty() && nodes_.contains(parent_id)) {
        nodes_[parent_id].children.push_back(id);
    }
    leaf_id_ = id;
}

std::optional<TreeNode> SessionTree::get(const std::string& id) const {
    auto it = nodes_.find(id);
    if (it == nodes_.end()) return std::nullopt;
    return it->second;
}

std::vector<std::string> SessionTree::branch_to_root(const std::string& id) const {
    std::vector<std::string> path;
    std::string current = id;
    while (!current.empty()) {
        path.push_back(current);
        auto it = nodes_.find(current);
        if (it == nodes_.end()) break;
        current = it->second.parent_id;
    }
    return path;
}

std::vector<std::string> SessionTree::children(const std::string& id) const {
    auto it = nodes_.find(id);
    if (it == nodes_.end()) return {};
    return it->second.children;
}

std::vector<core::JsonValue> SessionTree::build_session_context() const {
    auto path = branch_to_root(leaf_id_);
    std::reverse(path.begin(), path.end());

    std::vector<core::JsonValue> context;
    for (const auto& node_id : path) {
        auto it = nodes_.find(node_id);
        if (it == nodes_.end()) continue;
        const auto& entry = it->second.entry;

        // Skip non-message entries for context
        if (!entry.contains("type")) continue;
        auto type = entry["type"].get<std::string>();
        if (type == "session_header" || type == "compaction_start" || type == "compaction_end")
            continue;

        context.push_back(entry);
    }
    return context;
}

}  // namespace pie::session
