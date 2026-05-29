#pragma once

#include "pie/core/result.hpp"
#include <string>

namespace pie::sdk::telemetry {

enum class InstallMethod { Npm, Pnpm, Yarn, NativeBinary, Unknown };

struct StartupInfo {
    bool update_available = false;
    std::string latest_version;
    InstallMethod install_method = InstallMethod::Unknown;
};

class StartupChecks {
public:
    void set_offline(bool v) { offline_ = v; }
    void set_skip_version_check(bool v) { skip_version_check_ = v; }
    void set_telemetry_enabled(bool v) { telemetry_ = v; }

    // Run startup checks (non-fatal on failure, 2s timeout)
    StartupInfo run(const std::string& current_version);

    static InstallMethod detect_install_method();

private:
    bool offline_ = false;
    bool skip_version_check_ = false;
    bool telemetry_ = true;
};

}  // namespace pie::sdk::telemetry
