#pragma once

#include "pie/core/json.hpp"
#include "pie/core/result.hpp"
#include <filesystem>
#include <memory>
#include <string>
#include <vector>

namespace pie::extension_host {

struct ExtensionError {
    std::string extension_path;
    std::string event;
    std::string error;
};

class Extension {
public:
    virtual ~Extension() = default;
    virtual std::string path() const = 0;
    virtual Result<core::JsonValue> dispatch(const std::string& method, const core::JsonValue& params) = 0;
    virtual void unload() = 0;
};

class ExtensionHost {
public:
    Result<void> load_all(const std::vector<std::filesystem::path>& extension_paths);
    Result<core::JsonValue> dispatch(const std::string& event, const core::JsonValue& data);
    void unload_all();

    const std::vector<ExtensionError>& errors() const { return errors_; }
    size_t count() const { return extensions_.size(); }

private:
    std::vector<std::unique_ptr<Extension>> extensions_;
    std::vector<ExtensionError> errors_;
};

// Native C++ plugin ABI
struct PieExtensionVtable {
    void (*on_event)(const char* event_json);
    const char* (*get_info)();
    void (*destroy)();
};

using PieExtensionRegisterFn = void (*)(PieExtensionVtable*);

}  // namespace pie::extension_host
