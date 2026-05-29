// Integration tests: BashExecutor with real /bin/sh
// Task 92: Requirements 12.1, 12.2, 12.3, 12.5, 12.7, 12.9

#include <catch2/catch_test_macros.hpp>
#include "pie/io/subprocess.hpp"
#include <string>
#include <chrono>

TEST_CASE("Integration: BashExecutor captures stdout", "[integration][bash]") {
    auto res = pie::io::Subprocess::shell("echo hello_world");
    REQUIRE(res.has_value());
    CHECK(res->exit_code == 0);
    CHECK(res->output.find("hello_world") != std::string::npos);
}

TEST_CASE("Integration: BashExecutor captures exit code", "[integration][bash]") {
    auto res = pie::io::Subprocess::shell("exit 42");
    REQUIRE(res.has_value());
    CHECK(res->exit_code == 42);
}

TEST_CASE("Integration: BashExecutor handles empty output", "[integration][bash]") {
    auto res = pie::io::Subprocess::shell("true");
    REQUIRE(res.has_value());
    CHECK(res->exit_code == 0);
}

TEST_CASE("Integration: BashExecutor handles multiline output", "[integration][bash]") {
    auto res = pie::io::Subprocess::shell("printf 'line1\\nline2\\nline3\\n'");
    REQUIRE(res.has_value());
    CHECK(res->exit_code == 0);
    CHECK(res->output.find("line1") != std::string::npos);
    CHECK(res->output.find("line2") != std::string::npos);
    CHECK(res->output.find("line3") != std::string::npos);
}

TEST_CASE("Integration: BashExecutor run with explicit argv", "[integration][bash]") {
    auto res = pie::io::Subprocess::run({"/bin/echo", "explicit_argv"});
    REQUIRE(res.has_value());
    CHECK(res->exit_code == 0);
    CHECK(res->output.find("explicit_argv") != std::string::npos);
}

TEST_CASE("Integration: BashExecutor nonexistent command returns error", "[integration][bash]") {
    auto res = pie::io::Subprocess::shell("/this/cmd/does/not/exist_xyz_abc");
    // May succeed (returning non-zero exit) or return error — must not crash
    if (res.has_value()) {
        CHECK(res->exit_code != 0);
    }
    // No CHECK needed if it returns an error - that's valid too
}
