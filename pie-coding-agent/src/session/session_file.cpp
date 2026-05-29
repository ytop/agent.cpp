#include "pie/session/session_file.hpp"
#include "pie/io/file_lock.hpp"
#include <fcntl.h>
#include <fstream>
#include <unistd.h>

namespace pie::session {

Result<SessionFile> SessionFile::open(const std::filesystem::path& path) {
    SessionFile sf;
    sf.path_ = path;

    std::ifstream f(path);
    if (!f) return std::unexpected("cannot open " + path.string());

    std::string content((std::istreambuf_iterator<char>(f)), {});
    auto [entries, errors] = wire::JsonlParser::parse(content);
    sf.entries_ = std::move(entries);

    // Detect version from header entry
    if (!sf.entries_.empty() && sf.entries_[0].value.contains("version")) {
        sf.version_ = sf.entries_[0].value["version"].get<int>();
    }

    return sf;
}

Result<SessionFile> SessionFile::create(const std::filesystem::path& path, int version) {
    SessionFile sf;
    sf.path_ = path;
    sf.version_ = version;

    std::filesystem::create_directories(path.parent_path());

    core::JsonValue header = {{"type", "session_header"}, {"version", version}};
    std::ofstream f(path);
    if (!f) return std::unexpected("cannot create " + path.string());
    f << header.dump(-1) << "\n";
    sf.entries_.push_back({1, header});
    return sf;
}

Result<void> SessionFile::append(const core::JsonValue& entry) {
    auto lock = io::FileLock::acquire(path_);
    if (!lock) return std::unexpected(lock.error());

    int fd = ::open(path_.c_str(), O_WRONLY | O_APPEND);
    if (fd < 0) return std::unexpected("cannot open for append");

    auto line = entry.dump(-1) + "\n";
    auto written = ::write(fd, line.c_str(), line.size());
    fsync(fd);
    ::close(fd);

    if (written != static_cast<ssize_t>(line.size()))
        return std::unexpected("incomplete write");

    entries_.push_back({static_cast<int>(entries_.size() + 1), entry});
    return {};
}

Result<void> SessionFile::rewrite(const std::vector<core::JsonValue>& entries) {
    auto tmp = path_.string() + ".tmp";
    {
        std::ofstream f(tmp);
        if (!f) return std::unexpected("cannot write " + tmp);
        for (const auto& e : entries) f << e.dump(-1) << "\n";
    }
    std::filesystem::rename(tmp, path_);

    entries_.clear();
    int line = 0;
    for (const auto& e : entries) entries_.push_back({++line, e});
    version_ = 3;
    return {};
}

}  // namespace pie::session
