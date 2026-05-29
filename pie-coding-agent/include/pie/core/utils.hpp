#pragma once

#include <chrono>
#include <cstdint>
#include <filesystem>
#include <iomanip>
#include <random>
#include <sstream>
#include <string>

namespace pie::core {

// UUID v4 generator
inline std::string uuid_v4() {
    static thread_local std::mt19937 rng{std::random_device{}()};
    std::uniform_int_distribution<uint32_t> dist(0, 255);

    uint8_t bytes[16];
    for (auto& b : bytes) b = static_cast<uint8_t>(dist(rng));

    bytes[6] = (bytes[6] & 0x0F) | 0x40;  // version 4
    bytes[8] = (bytes[8] & 0x3F) | 0x80;  // variant 1

    std::ostringstream oss;
    oss << std::hex << std::setfill('0');
    for (int i = 0; i < 16; ++i) {
        if (i == 4 || i == 6 || i == 8 || i == 10) oss << '-';
        oss << std::setw(2) << static_cast<int>(bytes[i]);
    }
    return oss.str();
}

// 8-char hex ID generator
inline std::string hex_id() {
    static thread_local std::mt19937 rng{std::random_device{}()};
    std::uniform_int_distribution<uint32_t> dist;
    std::ostringstream oss;
    oss << std::hex << std::setfill('0') << std::setw(8) << dist(rng);
    return oss.str();
}

// ISO-8601 timestamp (UTC)
inline std::string iso8601_now() {
    auto now = std::chrono::system_clock::now();
    auto time_t_now = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()) % 1000;
    std::tm tm{};
    gmtime_r(&time_t_now, &tm);
    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%dT%H:%M:%S")
        << '.' << std::setfill('0') << std::setw(3) << ms.count() << 'Z';
    return oss.str();
}

// Path normalization (resolve . and .., make absolute)
inline std::string normalize_path(const std::string& path) {
    return std::filesystem::weakly_canonical(path).string();
}

}  // namespace pie::core
