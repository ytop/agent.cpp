// Feature: cpp-coding-agent, Property 5: deep merge idempotence and dominance
// deep_merge(a, {}) == a; for same scalar key b[k] always wins over a[k].
// Feature: cpp-coding-agent, Property 6: Tool_Allowlist enforcement
// ToolHost with an allowlist never permits tools not on the list.

#include <catch2/catch_test_macros.hpp>
#include <rapidcheck/catch.h>
#include "pie/settings/settings_manager.hpp"
#include "pie/tools/tool_host.hpp"
#include "pie/core/json.hpp"
#include <string>

TEST_CASE("Property 5: deep merge idempotence and dominance", "[property][settings]") {
    // Feature: cpp-coding-agent, Property 5: deep merge
    rc::prop("merge with empty is identity", []() {
        auto key = *rc::gen::arbitrary<std::string>();
        auto val = *rc::gen::arbitrary<std::string>();

        pie::core::JsonValue base = pie::core::JsonValue::object();
        if (!key.empty()) {
            std::string safe_key = key;
            for (auto& c : safe_key) {
                if (c == '"' || c == '\\' || c < 0x20) c = '_';
            }
            base[safe_key] = val;
        }

        auto merged = pie::settings::SettingsManager::deep_merge(base, pie::core::JsonValue::object());
        RC_ASSERT(merged == base);
    });

    rc::prop("overlay key wins over base key", []() {
        auto key = *rc::gen::nonEmpty(rc::gen::arbitrary<std::string>());
        auto base_val = *rc::gen::arbitrary<std::string>();
        auto overlay_val = *rc::gen::arbitrary<std::string>();

        // Sanitize key
        std::string safe_key = key;
        for (auto& c : safe_key) {
            if (c == '"' || c == '\\' || c < 0x20) c = '_';
        }
        if (safe_key.empty()) safe_key = "k";

        pie::core::JsonValue base = {{safe_key, base_val}};
        pie::core::JsonValue overlay = {{safe_key, overlay_val}};

        auto merged = pie::settings::SettingsManager::deep_merge(base, overlay);
        RC_ASSERT(merged.contains(safe_key));
        RC_ASSERT(merged[safe_key].get<std::string>() == overlay_val);
    });
}

TEST_CASE("Property 6: ToolHost allowlist enforcement", "[property][tools]") {
    // Feature: cpp-coding-agent, Property 6: Tool_Allowlist
    rc::prop("blocked tools are never accessible when allowlist is set", []() {
        pie::tools::ToolHost host;
        auto allowed_tool = *rc::gen::element(std::vector<std::string>{"read_file", "bash"});
        auto blocked_tool = *rc::gen::element(std::vector<std::string>{"write_file", "edit_file"});

        pie::tools::ToolAllowlist al;
        al.allowed.insert(allowed_tool);
        host.apply_allowlist(al);

        RC_ASSERT(host.is_allowed(allowed_tool) == true);
        RC_ASSERT(host.is_allowed(blocked_tool) == false);
    });

    rc::prop("no_tools flag blocks all", []() {
        pie::tools::ToolHost host;
        pie::tools::ToolAllowlist al;
        al.no_tools = true;
        host.apply_allowlist(al);

        RC_ASSERT(host.is_allowed("bash") == false);
        RC_ASSERT(host.is_allowed("read_file") == false);
    });
}
