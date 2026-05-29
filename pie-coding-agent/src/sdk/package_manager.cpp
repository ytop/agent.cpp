#include "pie/sdk/package_manager.hpp"
#include "pie/io/subprocess.hpp"

namespace pie::sdk::packages {

namespace fs = std::filesystem;

PackageManager::PackageManager(const fs::path& global_dir, const fs::path& project_dir)
    : global_dir_(global_dir), project_dir_(project_dir) {}

Result<PackageSource> PackageManager::parse_source(const std::string& source) {
    if (source.size() > 2048)
        return std::unexpected("source exceeds 2048 char limit");

    PackageSource ps{PackageSource::Unknown, source, "", ""};

    if (source.starts_with("npm:")) {
        ps.kind = PackageSource::Npm;
        ps.name = source.substr(4);
    } else if (source.starts_with("git:") || source.starts_with("git+")) {
        ps.kind = PackageSource::Git;
        ps.url = source;
    } else if (source.starts_with("https://")) {
        ps.kind = PackageSource::Https;
        ps.url = source;
    } else if (source.starts_with("ssh://") || source.starts_with("git@")) {
        ps.kind = PackageSource::Ssh;
        ps.url = source;
    } else if (source.starts_with("/") || source.starts_with("./")) {
        ps.kind = PackageSource::Local;
        ps.url = source;
    } else {
        ps.kind = PackageSource::Npm;
        ps.name = source;
    }
    return ps;
}

Result<void> PackageManager::install(const std::string& source) {
    if (offline_) return std::unexpected("offline mode: cannot install packages");

    auto parsed = parse_source(source);
    if (!parsed) return std::unexpected(parsed.error());

    fs::create_directories(global_dir_);

    if (parsed->kind == PackageSource::Npm) {
        auto r = pie::io::Subprocess::run({npm_cmd_, "install", "--prefix", global_dir_.string(), parsed->name});
        if (!r) return std::unexpected(r.error());
        if (r->exit_code != 0) return std::unexpected("npm install failed: " + r->output);
    } else if (parsed->kind == PackageSource::Git || parsed->kind == PackageSource::Https || parsed->kind == PackageSource::Ssh) {
        auto dest = global_dir_ / fs::path(parsed->url).filename();
        auto r = pie::io::Subprocess::run({"git", "clone", parsed->url, dest.string()});
        if (!r) return std::unexpected(r.error());
        if (r->exit_code != 0) return std::unexpected("git clone failed: " + r->output);
    }
    return {};
}

Result<void> PackageManager::remove(const std::string& name) {
    auto pkg_path = global_dir_ / name;
    if (!fs::exists(pkg_path)) return std::unexpected("package not found: " + name);
    fs::remove_all(pkg_path);
    return {};
}

Result<void> PackageManager::update(const std::string& name) {
    if (offline_) return std::unexpected("offline mode: cannot update packages");
    (void)name;  // Would iterate and update each
    return {};
}

std::vector<PackageInfo> PackageManager::list() const {
    std::vector<PackageInfo> pkgs;
    for (auto* dir : {&global_dir_, &project_dir_}) {
        if (!fs::is_directory(*dir)) continue;
        for (auto& entry : fs::directory_iterator(*dir)) {
            if (entry.is_directory()) {
                pkgs.push_back({entry.path().filename().string(), "", entry.path(), false});
            }
        }
    }
    return pkgs;
}

}  // namespace pie::sdk::packages
