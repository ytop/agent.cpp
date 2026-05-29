#include <catch2/catch_test_macros.hpp>
#include <filesystem>
#include <fstream>

#include "pie/compaction/compactor.hpp"
#include "pie/io/image_pipeline.hpp"
#include "pie/queue/message_queue.hpp"
#include "pie/sdk/exporter.hpp"
#include "pie/sdk/package_manager.hpp"
#include "pie/tools/bash_executor.hpp"
#include "pie/tools/builtin_tools.hpp"
#include "pie/tools/tool_host.hpp"

namespace fs = std::filesystem;

// --- ToolHost allowlist ---

TEST_CASE("ToolHost allowlist enforcement", "[tools]") {
    pie::tools::ToolHost host;
    pie::tools::register_builtin_tools(host);

    REQUIRE(host.all_allowed().size() == 6);
    REQUIRE(host.find("read") != nullptr);

    // Apply no-tools
    host.apply_allowlist({true, false, {}});
    REQUIRE(host.all_allowed().empty());
    REQUIRE(host.find("read") == nullptr);

    // Apply specific allowlist
    host.apply_allowlist({false, false, {"read", "write"}});
    REQUIRE(host.all_allowed().size() == 2);
    REQUIRE(host.find("read") != nullptr);
    REQUIRE(host.find("grep") == nullptr);
}

// --- BashExecutor ---

TEST_CASE("BashExecutor captures output", "[bash]") {
    pie::tools::BashExecutor exec;
    auto result = exec.run({.command = "echo hello"});
    REQUIRE(result.has_value());
    REQUIRE(result->exit_code == 0);
    REQUIRE(result->output == "hello\n");
    REQUIRE_FALSE(result->truncated);
}

TEST_CASE("BashExecutor truncation", "[bash]") {
    pie::tools::BashExecutor exec;
    // Generate > 32KB output
    auto result = exec.run({.command = "dd if=/dev/zero bs=1024 count=64 2>/dev/null | tr '\\0' 'A'"});
    REQUIRE(result.has_value());
    REQUIRE(result->truncated);
    REQUIRE(result->output.size() == pie::tools::BashExecutor::kTruncationThreshold);
    if (!result->overflow_file.empty()) fs::remove(result->overflow_file);
}

TEST_CASE("BashExecutor spawn failure", "[bash]") {
    pie::tools::BashExecutor exec;
    auto result = exec.run({.command = "exit 42"});
    REQUIRE(result.has_value());
    REQUIRE(result->exit_code == 42);
}

// --- Compactor ---

TEST_CASE("Compactor finds valid cut point", "[compaction]") {
    using pie::core::JsonValue;
    std::vector<JsonValue> msgs = {
        {{"type", "session_header"}},
        {{"type", "message"}, {"role", "user"}},
        {{"type", "message"}, {"role", "assistant"}},
        {{"type", "tool_call"}},
        {{"type", "message"}, {"role", "user"}},
    };
    int cut = pie::compaction::Compactor::find_cut_point(msgs);
    REQUIRE(cut == 4);  // last valid cut before end
}

// --- MessageQueue ---

TEST_CASE("MessageQueue drain modes", "[queue]") {
    pie::queue::MessageQueue q;
    q.push_steering("s1");
    q.push_steering("s2");
    q.push_follow_up("f1");

    REQUIRE(q.steering_size() == 2);
    REQUIRE(q.follow_up_size() == 1);

    auto one = q.drain_steering(pie::queue::DrainMode::OneAtATime);
    REQUIRE(one.size() == 1);
    REQUIRE(one[0] == "s1");
    REQUIRE(q.steering_size() == 1);

    auto all = q.drain_all();
    REQUIRE(all.size() == 2);  // s2 + f1
    REQUIRE(q.steering_size() == 0);
    REQUIRE(q.follow_up_size() == 0);
}

// --- ImagePipeline ---

TEST_CASE("ImagePipeline base64 encode", "[image]") {
    std::vector<uint8_t> data = {0, 1, 2, 3, 4, 5};
    auto b64 = pie::io::ImagePipeline::base64_encode(data);
    REQUIRE_FALSE(b64.empty());
    REQUIRE(b64.size() == 8);  // 6 bytes -> 8 base64 chars
}

TEST_CASE("ImagePipeline protocol detection", "[image]") {
    // Without KITTY_WINDOW_ID set, should fall back to Placeholder
    unsetenv("KITTY_WINDOW_ID");
    auto proto = pie::io::ImagePipeline::detect_protocol();
    REQUIRE(proto == pie::io::ImageProtocol::Placeholder);
}

TEST_CASE("ImagePipeline rejects oversized input", "[image]") {
    std::vector<uint8_t> big(21 * 1024 * 1024, 0);
    auto result = pie::io::ImagePipeline::decode(big);
    REQUIRE_FALSE(result.has_value());
    REQUIRE(result.error().find("20 MB") != std::string::npos);
}

// --- PackageManager source parser ---

TEST_CASE("PackageManager parses source strings", "[packages]") {
    using pie::sdk::packages::PackageManager;
    using pie::sdk::packages::PackageSource;

    auto npm = PackageManager::parse_source("npm:my-package");
    REQUIRE(npm.has_value());
    REQUIRE(npm->kind == PackageSource::Npm);
    REQUIRE(npm->name == "my-package");

    auto git = PackageManager::parse_source("git:github.com/user/repo");
    REQUIRE(git.has_value());
    REQUIRE(git->kind == PackageSource::Git);

    auto https = PackageManager::parse_source("https://example.com/pkg.tar.gz");
    REQUIRE(https.has_value());
    REQUIRE(https->kind == PackageSource::Https);

    auto ssh = PackageManager::parse_source("git@github.com:user/repo.git");
    REQUIRE(ssh.has_value());
    REQUIRE(ssh->kind == PackageSource::Ssh);

    // Reject > 2048 chars
    std::string long_src(2049, 'a');
    auto err = PackageManager::parse_source(long_src);
    REQUIRE_FALSE(err.has_value());
}

// --- Exporter ---

TEST_CASE("Exporter produces self-contained HTML", "[export]") {
    using pie::core::JsonValue;
    std::vector<JsonValue> msgs = {
        {{"role", "user"}, {"content", "hello"}},
        {{"role", "assistant"}, {"content", "hi there"}},
    };
    auto html = pie::sdk::export_html::Exporter::render(msgs);
    REQUIRE(html.find("<!DOCTYPE html>") != std::string::npos);
    REQUIRE(html.find("hello") != std::string::npos);
    REQUIRE(html.find("hi there") != std::string::npos);
    // No external resource references
    REQUIRE(html.find("http://") == std::string::npos);
    REQUIRE(html.find("https://") == std::string::npos);
}
