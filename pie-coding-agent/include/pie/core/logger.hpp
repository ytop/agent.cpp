#pragma once

#include <fstream>
#include <iostream>
#include <mutex>
#include <string>

namespace pie::core {

class Logger {
public:
    static Logger& instance() {
        static Logger logger;
        return logger;
    }

    void set_verbose(bool v) { verbose_ = v; }
    void set_log_file(const std::string& path) {
        log_file_.open(path, std::ios::app);
    }

    void error(const std::string& msg) { log("ERROR", msg); }
    void warn(const std::string& msg) { log("WARN", msg); }
    void info(const std::string& msg) { if (verbose_) log("INFO", msg); }
    void debug(const std::string& msg) { if (verbose_) log("DEBUG", msg); }

private:
    Logger() = default;

    void log(const char* level, const std::string& msg) {
        std::lock_guard<std::mutex> lock(mu_);
        std::cerr << "[" << level << "] " << msg << "\n";
        if (log_file_.is_open()) {
            log_file_ << "[" << level << "] " << msg << "\n";
            log_file_.flush();
        }
    }

    bool verbose_ = false;
    std::ofstream log_file_;
    std::mutex mu_;
};

}  // namespace pie::core
