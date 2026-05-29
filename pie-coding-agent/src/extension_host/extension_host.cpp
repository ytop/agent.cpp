#include "pie/extension_host/extension_host.hpp"
#include "pie/io/subprocess.hpp"
#include <dlfcn.h>

namespace pie::extension_host {

namespace fs = std::filesystem;

// --- JS Extension (out-of-process Node bridge) ---
class JsExtension : public Extension {
public:
    JsExtension(fs::path path) : path_(std::move(path)) {}

    std::string path() const override { return path_.string(); }

    Result<core::JsonValue> dispatch(const std::string& method, const core::JsonValue& params) override {
        // In real impl: send JSON-RPC over stdio to node process
        (void)method; (void)params;
        return core::JsonValue{{"status", "stub"}};
    }

    void unload() override {}

private:
    fs::path path_;
};

// --- Native Extension (dlopen) ---
class NativeExtension : public Extension {
public:
    NativeExtension(fs::path path, void* handle, PieExtensionVtable vtable)
        : path_(std::move(path)), handle_(handle), vtable_(vtable) {}

    ~NativeExtension() override { unload(); }

    std::string path() const override { return path_.string(); }

    Result<core::JsonValue> dispatch(const std::string& method, const core::JsonValue& params) override {
        if (vtable_.on_event) {
            auto json = core::JsonValue{{"method", method}, {"params", params}}.dump();
            vtable_.on_event(json.c_str());
        }
        return core::JsonValue{{"status", "ok"}};
    }

    void unload() override {
        if (handle_) {
            if (vtable_.destroy) vtable_.destroy();
            dlclose(handle_);
            handle_ = nullptr;
        }
    }

private:
    fs::path path_;
    void* handle_;
    PieExtensionVtable vtable_;
};

// --- ExtensionHost ---

Result<void> ExtensionHost::load_all(const std::vector<fs::path>& extension_paths) {
    for (const auto& p : extension_paths) {
        if (p.extension() == ".so") {
            void* handle = dlopen(p.c_str(), RTLD_LAZY);
            if (!handle) {
                errors_.push_back({p.string(), "load", dlerror()});
                continue;
            }
            auto reg_fn = reinterpret_cast<PieExtensionRegisterFn>(dlsym(handle, "pie_extension_register"));
            if (!reg_fn) {
                errors_.push_back({p.string(), "load", "missing pie_extension_register symbol"});
                dlclose(handle);
                continue;
            }
            PieExtensionVtable vtable{};
            reg_fn(&vtable);
            extensions_.push_back(std::make_unique<NativeExtension>(p, handle, vtable));
        } else {
            // JS extension — check if node is available
            auto node_check = io::Subprocess::run({"node", "--version"});
            if (!node_check || node_check->exit_code != 0) {
                errors_.push_back({p.string(), "load", "node not available, skipping JS extension"});
                continue;
            }
            extensions_.push_back(std::make_unique<JsExtension>(p));
        }
    }
    return {};
}

Result<core::JsonValue> ExtensionHost::dispatch(const std::string& event, const core::JsonValue& data) {
    core::JsonValue results = core::JsonValue::array();
    for (auto& ext : extensions_) {
        auto r = ext->dispatch(event, data);
        if (!r) {
            errors_.push_back({ext->path(), event, r.error()});
        } else {
            results.push_back(*r);
        }
    }
    return results;
}

void ExtensionHost::unload_all() {
    for (auto& ext : extensions_) ext->unload();
    extensions_.clear();
}

}  // namespace pie::extension_host
