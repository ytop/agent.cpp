#pragma once

#include "pie/core/result.hpp"
#include <filesystem>
#include <string>
#include <vector>

namespace pie::sdk::packages {

struct PackageSource {
    enum Kind { Npm, Git, Https, Ssh, Local, Unknown } kind = Unknown;
    std::string raw;
    std::string name;
    std::string url;
};

struct PackageInfo {
    std::string name;
    std::string version;
    std::filesystem::path path;
    bool pinned = false;
};

class PackageManager {
public:
    PackageManager(const std::filesystem::path& global_dir,
                   const std::filesystem::path& project_dir);

    static Result<PackageSource> parse_source(const std::string& source);

    Result<void> install(const std::string& source);
    Result<void> remove(const std::string& name);
    Result<void> update(const std::string& name = "");
    std::vector<PackageInfo> list() const;

    void set_offline(bool offline) { offline_ = offline; }
    void set_npm_command(const std::string& cmd) { npm_cmd_ = cmd; }

private:
    std::filesystem::path global_dir_;
    std::filesystem::path project_dir_;
    bool offline_ = false;
    std::string npm_cmd_ = "npm";
};

}  // namespace pie::sdk::packages
