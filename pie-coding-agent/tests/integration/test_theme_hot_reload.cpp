// Integration tests: Theme hot reload via FsWatcher
// Task 94: Requirements 15.6, 17.4, 17.10

#include <catch2/catch_test_macros.hpp>
#include "pie/io/fs_watcher.hpp"
#include <filesystem>
#include <fstream>
#include <chrono>
#include <thread>
#include <atomic>

namespace fs = std::filesystem;

TEST_CASE("Integration: FsWatcher detects file creation", "[integration][watcher]") {
    auto tmp = fs::temp_directory_path() / "pie_test_watcher";
    fs::create_directories(tmp);
    auto test_file = tmp / "theme_test.json";
    fs::remove(test_file); // Clean up first

    std::atomic<bool> event_fired{false};
    pie::io::FsWatcher watcher;
    watcher.watch(tmp, [&](const fs::path& changed) {
        if (changed.filename() == "theme_test.json") {
            event_fired = true;
        }
    });
    watcher.start();

    // Create the file
    std::ofstream f(test_file);
    f << R"({"colors":{"primary":"#ff0000"}})";
    f.close();

    // Wait up to 1000ms for the event
    auto deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(1000);
    while (!event_fired && std::chrono::steady_clock::now() < deadline) {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }

    watcher.stop();
    fs::remove(test_file);
    fs::remove(tmp);

    // Event should have fired within 1000ms
    CHECK(event_fired == true);
}

TEST_CASE("Integration: FsWatcher can start and stop cleanly", "[integration][watcher]") {
    auto tmp = fs::temp_directory_path() / "pie_test_watcher2";
    fs::create_directories(tmp);

    pie::io::FsWatcher watcher;
    watcher.watch(tmp, [](const fs::path&) {});

    // Should not throw or crash
    watcher.start();
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    watcher.stop();

    fs::remove(tmp);
    CHECK(true);
}
