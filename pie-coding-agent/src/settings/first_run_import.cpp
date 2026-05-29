#include "pie/settings/first_run_import.hpp"
#include "pie/core/json.hpp"
#include <cstdlib>
#include <fstream>

namespace pie::settings {

namespace fs = std::filesystem;

Result<bool> FirstRunImport::run(
    const fs::path& agent_dir,
    const fs::path& ts_agent_dir_override) {

    auto marker = agent_dir / ".import-from-pi.json";
    if (fs::exists(marker)) return false;  // Already imported

    fs::path ts_dir = ts_agent_dir_override;
    if (ts_dir.empty()) {
        auto home = std::getenv("HOME");
        if (home) ts_dir = fs::path(home) / ".pi" / "agent";
    }

    if (!fs::is_directory(ts_dir)) return false;  // Nothing to import

    // Check if agent_dir already has content
    if (fs::is_directory(agent_dir) && !fs::is_empty(agent_dir)) return false;

    fs::create_directories(agent_dir);

    // Copy canonical files
    for (auto& entry : fs::directory_iterator(ts_dir)) {
        auto dest = agent_dir / entry.path().filename();
        if (entry.is_directory()) {
            fs::copy(entry.path(), dest, fs::copy_options::recursive);
        } else {
            fs::copy_file(entry.path(), dest, fs::copy_options::skip_existing);
        }
    }

    // Write marker
    core::JsonValue marker_data = {
        {"importedFrom", ts_dir.string()},
        {"importedAt", core::JsonValue(nullptr)}  // timestamp would go here
    };
    std::ofstream(marker) << marker_data.dump(2) << "\n";

    return true;
}

}  // namespace pie::settings
