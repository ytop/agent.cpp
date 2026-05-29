#include "pie/tools/builtin_tools.hpp"
#include "pie/wire/diff.hpp"
#include "pie/wire/globber.hpp"

#include <dirent.h>
#include <filesystem>
#include <fstream>
#include <regex>
#include <sstream>

namespace pie::tools {
namespace fs = std::filesystem;

// --- ReadTool ---
class ReadTool : public Tool {
public:
    std::string name() const override { return "read"; }
    std::string label() const override { return "Read File"; }
    std::string description() const override { return "Read file contents, optionally a line range"; }
    core::JsonValue parameters_schema() const override {
        return {{"type", "object"}, {"properties", {{"path", {{"type", "string"}}}, {"start_line", {{"type", "integer"}}}, {"end_line", {{"type", "integer"}}}}}, {"required", {"path"}}};
    }
    Result<core::JsonValue> execute(const core::JsonValue& params) override {
        auto path = params["path"].get<std::string>();
        if (!fs::exists(path)) return std::unexpected("file not found: " + path);
        std::ifstream f(path);
        if (!f) return std::unexpected("cannot open: " + path);

        std::string content((std::istreambuf_iterator<char>(f)), {});
        if (params.contains("start_line")) {
            std::istringstream ss(content);
            std::string line, result;
            int ln = 0, start = params["start_line"].get<int>(), end = params.value("end_line", INT_MAX);
            while (std::getline(ss, line)) {
                if (++ln >= start && ln <= end) result += line + "\n";
            }
            return core::JsonValue{{"content", result}};
        }
        return core::JsonValue{{"content", content}};
    }
};

// --- WriteTool ---
class WriteTool : public Tool {
public:
    std::string name() const override { return "write"; }
    std::string label() const override { return "Write File"; }
    std::string description() const override { return "Write content to a file (creates parent dirs)"; }
    core::JsonValue parameters_schema() const override {
        return {{"type", "object"}, {"properties", {{"path", {{"type", "string"}}}, {"content", {{"type", "string"}}}}}, {"required", {"path", "content"}}};
    }
    Result<core::JsonValue> execute(const core::JsonValue& params) override {
        auto path = fs::path(params["path"].get<std::string>());
        fs::create_directories(path.parent_path());
        std::ofstream f(path);
        if (!f) return std::unexpected("cannot write: " + path.string());
        f << params["content"].get<std::string>();
        return core::JsonValue{{"success", true}};
    }
};

// --- EditTool ---
class EditTool : public Tool {
public:
    std::string name() const override { return "edit"; }
    std::string label() const override { return "Edit File"; }
    std::string description() const override { return "Replace old_text with new_text in a file"; }
    core::JsonValue parameters_schema() const override {
        return {{"type", "object"}, {"properties", {{"path", {{"type", "string"}}}, {"old_text", {{"type", "string"}}}, {"new_text", {{"type", "string"}}}}}, {"required", {"path", "old_text", "new_text"}}};
    }
    Result<core::JsonValue> execute(const core::JsonValue& params) override {
        auto path = params["path"].get<std::string>();
        std::ifstream f(path);
        if (!f) return std::unexpected("cannot open: " + path);
        std::string content((std::istreambuf_iterator<char>(f)), {});
        f.close();

        auto old_text = params["old_text"].get<std::string>();
        auto new_text = params["new_text"].get<std::string>();
        auto pos = content.find(old_text);
        if (pos == std::string::npos) return std::unexpected("old_text not found in file");

        auto new_content = content.substr(0, pos) + new_text + content.substr(pos + old_text.size());
        std::ofstream out(path);
        out << new_content;

        auto diff = wire::Diff::unified_diff(content, new_content);
        return core::JsonValue{{"success", true}, {"diff", diff}};
    }
};

// --- LsTool ---
class LsTool : public Tool {
public:
    std::string name() const override { return "ls"; }
    std::string label() const override { return "List Directory"; }
    std::string description() const override { return "List directory contents"; }
    core::JsonValue parameters_schema() const override {
        return {{"type", "object"}, {"properties", {{"path", {{"type", "string"}}}}}, {"required", {"path"}}};
    }
    Result<core::JsonValue> execute(const core::JsonValue& params) override {
        auto path = params["path"].get<std::string>();
        DIR* dir = opendir(path.c_str());
        if (!dir) return std::unexpected("cannot open directory: " + path);
        core::JsonValue entries = core::JsonValue::array();
        struct dirent* ent;
        while ((ent = readdir(dir)) != nullptr) {
            if (ent->d_name[0] == '.' && (ent->d_name[1] == '\0' || (ent->d_name[1] == '.' && ent->d_name[2] == '\0')))
                continue;
            entries.push_back(ent->d_name);
        }
        closedir(dir);
        return core::JsonValue{{"entries", entries}};
    }
};

// --- GrepTool ---
class GrepTool : public Tool {
public:
    std::string name() const override { return "grep"; }
    std::string label() const override { return "Search Files"; }
    std::string description() const override { return "Search for a regex pattern in files recursively"; }
    core::JsonValue parameters_schema() const override {
        return {{"type", "object"}, {"properties", {{"pattern", {{"type", "string"}}}, {"path", {{"type", "string"}}}}}, {"required", {"pattern", "path"}}};
    }
    Result<core::JsonValue> execute(const core::JsonValue& params) override {
        auto pattern = params["pattern"].get<std::string>();
        auto root = params["path"].get<std::string>();
        std::regex re(pattern, std::regex::ECMAScript);

        core::JsonValue matches = core::JsonValue::array();
        if (!fs::is_directory(root)) return std::unexpected("not a directory: " + root);

        for (auto& entry : fs::recursive_directory_iterator(root, fs::directory_options::skip_permission_denied)) {
            if (!entry.is_regular_file()) continue;
            std::ifstream f(entry.path());
            std::string line;
            int ln = 0;
            while (std::getline(f, line)) {
                ++ln;
                if (std::regex_search(line, re)) {
                    matches.push_back({{"file", entry.path().string()}, {"line", ln}, {"content", line}});
                }
            }
        }
        return core::JsonValue{{"matches", matches}};
    }
};

// --- FindTool ---
class FindTool : public Tool {
public:
    std::string name() const override { return "find"; }
    std::string label() const override { return "Find Files"; }
    std::string description() const override { return "Find files matching a glob pattern"; }
    core::JsonValue parameters_schema() const override {
        return {{"type", "object"}, {"properties", {{"pattern", {{"type", "string"}}}, {"path", {{"type", "string"}}}}}, {"required", {"pattern", "path"}}};
    }
    Result<core::JsonValue> execute(const core::JsonValue& params) override {
        auto pattern = params["pattern"].get<std::string>();
        auto root = params["path"].get<std::string>();
        auto results = wire::Globber::glob(root, pattern);
        core::JsonValue files = core::JsonValue::array();
        for (auto& p : results) files.push_back(p.string());
        return core::JsonValue{{"files", files}};
    }
};

// --- Registration ---
std::shared_ptr<Tool> make_read_tool() { return std::make_shared<ReadTool>(); }
std::shared_ptr<Tool> make_write_tool() { return std::make_shared<WriteTool>(); }
std::shared_ptr<Tool> make_edit_tool() { return std::make_shared<EditTool>(); }
std::shared_ptr<Tool> make_ls_tool() { return std::make_shared<LsTool>(); }
std::shared_ptr<Tool> make_grep_tool() { return std::make_shared<GrepTool>(); }
std::shared_ptr<Tool> make_find_tool() { return std::make_shared<FindTool>(); }

void register_builtin_tools(ToolHost& host) {
    host.register_tool(make_read_tool());
    host.register_tool(make_write_tool());
    host.register_tool(make_edit_tool());
    host.register_tool(make_ls_tool());
    host.register_tool(make_grep_tool());
    host.register_tool(make_find_tool());
}

}  // namespace pie::tools
