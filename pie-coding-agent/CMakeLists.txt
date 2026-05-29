cmake_minimum_required(VERSION 3.27)

project(pie-coding-agent
    VERSION 0.1.0
    LANGUAGES CXX
    DESCRIPTION "Pie — C++20 coding agent"
)

# --------------------------------------------------------------------------
# C++23 standard, enforced globally
# --------------------------------------------------------------------------
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# --------------------------------------------------------------------------
# vcpkg toolchain integration (set via CMAKE_TOOLCHAIN_FILE or preset)
# --------------------------------------------------------------------------
if(DEFINED ENV{VCPKG_ROOT} AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake"
        CACHE STRING "vcpkg toolchain file")
endif()

# --------------------------------------------------------------------------
# Ubuntu gcc toolchain enforcement
# --------------------------------------------------------------------------
include(cmake/UbuntuGcc.cmake)

# --------------------------------------------------------------------------
# Global compile options
# --------------------------------------------------------------------------
add_compile_options(-Wall -Wextra -Wpedantic)

# --------------------------------------------------------------------------
# Find vcpkg-managed dependencies
# --------------------------------------------------------------------------
# Required for Phase 1 (pie binary)
find_package(CLI11 CONFIG REQUIRED)

# Required for Phase 2
find_package(nlohmann_json CONFIG REQUIRED)
find_package(CURL REQUIRED)
find_package(OpenSSL REQUIRED)

# Optional dependencies — required by later phases, found when available.
find_package(ftxui CONFIG QUIET)
find_package(Boost QUIET COMPONENTS headers)
find_package(asio CONFIG QUIET)
find_package(efsw CONFIG QUIET)
find_package(valijson CONFIG QUIET)
find_package(ryml CONFIG QUIET)
find_package(httplib CONFIG QUIET)
find_package(libexif CONFIG QUIET)
find_package(CMakeRC CONFIG QUIET)

# --------------------------------------------------------------------------
# Embed runtime assets via CMakeRC (when available)
# --------------------------------------------------------------------------
if(CMakeRC_FOUND)
    cmrc_add_resource_library(pie_runtime_assets
        ALIAS pie::runtime_assets
        NAMESPACE pie_assets
        runtime/extension-runtime.cjs
        runtime/export-html/template.html
        runtime/export-html/template.css
        runtime/export-html/template.js
        runtime/export-html/vendor/highlight.min.js
        runtime/theme/dark.json
        runtime/theme/light.json
    )
    set(PIE_HAS_RUNTIME_ASSETS TRUE)
else()
    set(PIE_HAS_RUNTIME_ASSETS FALSE)
endif()

# --------------------------------------------------------------------------
# Public SDK include directory
# --------------------------------------------------------------------------
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# --------------------------------------------------------------------------
# Third-party vendored headers
# --------------------------------------------------------------------------
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/third_party)

# --------------------------------------------------------------------------
# Static library target: libpie
# --------------------------------------------------------------------------
add_library(libpie STATIC
    src/core/placeholder.cpp
    src/io/file_lock.cpp
    src/io/http_client.cpp
    src/io/subprocess.cpp
    src/io/fs_watcher.cpp
    src/io/image_pipeline.cpp
    src/wire/jsonl.cpp
    src/wire/yaml_frontmatter.cpp
    src/wire/json_schema.cpp
    src/wire/globber.cpp
    src/wire/diff.cpp
    src/auth/auth_storage.cpp
    src/auth/oauth.cpp
    src/session/session_file.cpp
    src/session/session_tree.cpp
    src/session/session_manager.cpp
    src/session/migration.cpp
    src/settings/settings_manager.cpp
    src/settings/first_run_import.cpp
    src/models/model_registry.cpp
    src/resources/resource_loader.cpp
    src/resources/prompt_loader.cpp
    src/resources/skill_loader.cpp
    src/resources/theme_manager.cpp
    src/tools/tool_host.cpp
    src/tools/builtin_tools.cpp
    src/tools/bash_executor.cpp
    src/compaction/compactor.cpp
    src/queue/message_queue.cpp
    src/providers/provider.cpp
    src/tui/tui_runtime.cpp
    src/extension_host/extension_host.cpp
    src/sdk/package_manager.cpp
    src/sdk/exporter.cpp
    src/sdk/startup_checks.cpp
    src/sdk/agent_session.cpp
    src/sdk/agent_session_runtime.cpp
    src/tools/define_tool.cpp
    src/cli/cli_invocation.cpp
    src/cli/slash_commands.cpp
    src/cli/keybindings.cpp
    src/modes/interactive/interactive_mode.cpp
    src/modes/print/print_mode.cpp
    src/modes/json/json_mode.cpp
    src/modes/rpc/rpc_mode.cpp
    src/modes/export/export_mode.cpp
)
target_include_directories(libpie PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/asio/include
)
target_link_libraries(libpie PUBLIC
    nlohmann_json::nlohmann_json
    CURL::libcurl
    OpenSSL::SSL OpenSSL::Crypto
    ${CMAKE_DL_LIBS}
)
# valijson is header-only; add include path if found
if(valijson_FOUND)
    target_link_libraries(libpie PUBLIC valijson)
else()
    # System-installed valijson (header-only, no cmake target)
    target_include_directories(libpie PUBLIC /usr/include)
endif()
set_target_properties(libpie PROPERTIES
    OUTPUT_NAME pie
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
)
if(PIE_HAS_RUNTIME_ASSETS)
    target_link_libraries(libpie PRIVATE pie::runtime_assets)
endif()

# --------------------------------------------------------------------------
# Executable target: pie
# --------------------------------------------------------------------------
add_executable(pie
    src/cli/main.cpp
)
target_link_libraries(pie PRIVATE libpie CLI11::CLI11)
set_target_properties(pie PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
)

# --------------------------------------------------------------------------
# Tests (Catch2)
# --------------------------------------------------------------------------
find_package(Catch2 3 CONFIG QUIET)
if(NOT Catch2_FOUND)
    find_package(Catch2 3 REQUIRED)
endif()

enable_testing()

add_executable(pie_tests
    tests/unit/phase2_test.cpp
    tests/unit/phase3_test.cpp
    tests/unit/phase4_test.cpp
    tests/unit/phase5_test.cpp
    tests/unit/phase6_test.cpp
    tests/unit/test_verbose_banner.cpp
)
target_link_libraries(pie_tests PRIVATE libpie Catch2::Catch2WithMain)
target_compile_definitions(pie_tests PRIVATE
    PIE_BINARY_PATH="${CMAKE_BINARY_DIR}/pie"
    PIE_FIXTURES_DIR="${CMAKE_SOURCE_DIR}/tests/fixtures"
)
set_target_properties(pie_tests PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
)

include(CTest)
include(Catch)
catch_discover_tests(pie_tests)

# --------------------------------------------------------------------------
# Property-based tests (rapidcheck + Catch2)
# --------------------------------------------------------------------------
include(cmake/FetchRapidcheck.cmake)

file(GLOB PROPERTY_TEST_SRCS "tests/property/p*.cpp")
if(PROPERTY_TEST_SRCS)
    add_executable(pie_property_tests ${PROPERTY_TEST_SRCS})
    target_link_libraries(pie_property_tests PRIVATE
        libpie
        Catch2::Catch2WithMain
        rapidcheck
        rapidcheck_catch
    )
    target_include_directories(pie_property_tests PRIVATE
        ${rapidcheck_SOURCE_DIR}/extras/catch/include
    )
    set_target_properties(pie_property_tests PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
    )
    catch_discover_tests(pie_property_tests)
endif()

# --------------------------------------------------------------------------
# Integration tests (gated by PIE_INTEGRATION_TESTS option)
# --------------------------------------------------------------------------
option(PIE_INTEGRATION_TESTS "Build and register integration tests" OFF)
if(PIE_INTEGRATION_TESTS)
    file(GLOB INTEGRATION_TEST_SRCS "tests/integration/test_*.cpp")
    if(INTEGRATION_TEST_SRCS)
        add_executable(pie_integration_tests ${INTEGRATION_TEST_SRCS})
        target_link_libraries(pie_integration_tests PRIVATE
            libpie
            Catch2::Catch2WithMain
        )
        set_target_properties(pie_integration_tests PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
        )
        catch_discover_tests(pie_integration_tests)
    endif()
endif()

# --------------------------------------------------------------------------
# Conformance tests
# --------------------------------------------------------------------------
file(GLOB CONFORMANCE_TEST_SRCS "tests/conformance/test_*.cpp")
if(CONFORMANCE_TEST_SRCS)
    add_executable(pie_conformance_tests ${CONFORMANCE_TEST_SRCS})
    target_link_libraries(pie_conformance_tests PRIVATE
        libpie
        Catch2::Catch2WithMain
    )
    target_compile_definitions(pie_conformance_tests PRIVATE
        PIE_FIXTURES_DIR="${CMAKE_SOURCE_DIR}/tests/fixtures"
        PIE_BINARY_PATH="${CMAKE_BINARY_DIR}/pie"
    )
    set_target_properties(pie_conformance_tests PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
    )
    catch_discover_tests(pie_conformance_tests)
endif()

# --------------------------------------------------------------------------
# Coverage support (optional, use -DCMAKE_BUILD_TYPE=Coverage)
# --------------------------------------------------------------------------
include(cmake/Coverage.cmake)
