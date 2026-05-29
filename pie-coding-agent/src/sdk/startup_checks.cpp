#include "pie/sdk/startup_checks.hpp"
#include "pie/core/json.hpp"
#include "pie/io/http_client.hpp"
#include <cstdlib>
#include <filesystem>

namespace pie::sdk::telemetry {

InstallMethod StartupChecks::detect_install_method() {
    namespace fs = std::filesystem;
    // Heuristic: check how the binary was installed
    auto exe = fs::read_symlink("/proc/self/exe");
    auto path_str = exe.string();
    if (path_str.find("node_modules") != std::string::npos) {
        if (path_str.find("pnpm") != std::string::npos) return InstallMethod::Pnpm;
        if (path_str.find("yarn") != std::string::npos) return InstallMethod::Yarn;
        return InstallMethod::Npm;
    }
    return InstallMethod::NativeBinary;
}

StartupInfo StartupChecks::run(const std::string& current_version) {
    StartupInfo info;
    info.install_method = detect_install_method();

    if (offline_ || skip_version_check_) return info;

    // Update check with 2s timeout (non-fatal)
    io::HttpClient http;
    http.set_timeout_ms(2000);

    auto resp = http.get("https://registry.npmjs.org/pie-coding-agent/latest",
        {{"Accept", "application/json"}});
    if (resp && resp->status_code == 200) {
        try {
            auto body = core::JsonValue::parse(resp->body);
            info.latest_version = body.value("version", "");
            info.update_available = !info.latest_version.empty() &&
                                    info.latest_version != current_version;
        } catch (...) {}
    }

    // Telemetry POST (non-fatal)
    if (telemetry_) {
        core::JsonValue payload = {
            {"version", current_version},
            {"installMethod", static_cast<int>(info.install_method)}
        };
        http.post("https://telemetry.pie-agent.dev/v1/startup", payload.dump(),
            {{"Content-Type", "application/json"}});
    }

    return info;
}

}  // namespace pie::sdk::telemetry
