cmake_minimum_required(VERSION 3.27)

project(pie-coding-agent
    VERSION 0.1.0
    LANGUAGES CXX
    DESCRIPTION "Pie — C++20 coding agent"
)

# --------------------------------------------------------------------------
# C++20 standard, enforced globally
# --------------------------------------------------------------------------
set(CMAKE_CXX_STANDARD 20)
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
# Apple clang version enforcement (Req 1.11)
# --------------------------------------------------------------------------
include(cmake/ApplePinClang.cmake)

# --------------------------------------------------------------------------
# Global compile options
# --------------------------------------------------------------------------
add_compile_options(-Wall -Wextra -Wpedantic)

# --------------------------------------------------------------------------
# Find vcpkg-managed dependencies
# --------------------------------------------------------------------------
# Required for Phase 1 (pie binary)
find_package(CLI11 CONFIG REQUIRED)

# Optional dependencies — required by later phases, found when available.
find_package(nlohmann_json CONFIG QUIET)
find_package(CURL CONFIG QUIET)
find_package(OpenSSL QUIET)
find_package(ftxui CONFIG QUIET)
find_package(Boost QUIET COMPONENTS headers)
find_package(asio CONFIG QUIET)
find_package(tl-expected CONFIG QUIET)
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
# Placeholder static library target: libpie
# --------------------------------------------------------------------------
add_library(libpie STATIC
    src/core/placeholder.cpp
)
target_include_directories(libpie PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
set_target_properties(libpie PROPERTIES
    OUTPUT_NAME pie
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
)
if(PIE_HAS_RUNTIME_ASSETS)
    target_link_libraries(libpie PRIVATE pie::runtime_assets)
endif()

# --------------------------------------------------------------------------
# Placeholder executable target: pie
# --------------------------------------------------------------------------
add_executable(pie
    src/cli/main.cpp
)
target_link_libraries(pie PRIVATE libpie CLI11::CLI11)
set_target_properties(pie PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
)
