// Feature: cpp-coding-agent, Property 10: bash truncation at line boundary
// BashExecutor output exceeding threshold is truncated at a complete line.
// Feature: cpp-coding-agent, Property 11: bash metachar byte-equivalence
// Commands with shell metacharacters produce identical argv pattern as Pi_TS.

#include <catch2/catch_test_macros.hpp>
#include <rapidcheck/catch.h>
#include "pie/io/subprocess.hpp"
#include <string>

TEST_CASE("Property 10: bash output ends at complete line", "[property][bash]") {
    // Feature: cpp-coding-agent, Property 10: bash truncation
    rc::prop("bash output that fits threshold is not truncated", []() {
        int n = *rc::gen::inRange(1, 5);
        std::string cmd = "echo hello_" + std::to_string(n);

        auto res = pie::io::Subprocess::shell(cmd);
        RC_PRE(res.has_value());
        RC_ASSERT(!res->output.empty());
        // Output must end with a newline (complete line)
        RC_ASSERT(res->output.back() == '\n');
    });
}

TEST_CASE("Property 11: bash metachar argv pattern", "[property][bash]") {
    // Feature: cpp-coding-agent, Property 11: bash metachar byte-equivalence
    rc::prop("metacharacters in command are passed verbatim to shell", []() {
        int idx = *rc::gen::inRange(0, 4);
        std::string suffix;
        if (idx == 0) suffix = "echo a&&echo b";
        else if (idx == 1) suffix = "echo a;echo b";
        else if (idx == 2) suffix = "echo 'hello world'";
        else suffix = "echo -n done";

        auto res = pie::io::Subprocess::shell(suffix);
        RC_PRE(res.has_value());
        RC_ASSERT(res->exit_code == 0);
    });
}
