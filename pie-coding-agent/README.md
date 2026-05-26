# Pie — C++20 Coding Agent

A native C++20 re-implementation of the Pi coding agent, producing a single `pie` binary and an embeddable `libpie` static library.

## Supported Platforms

| Platform | Architecture | Status |
|----------|-------------|--------|
| macOS 13+ | arm64 (Apple Silicon) | Primary |
| macOS 13+ | x86_64 | Secondary |
| Linux | x86_64 | Supported |
| Linux | aarch64 | Supported |
| Windows | — | Out of scope |

## Requirements

- **CMake** 3.27 or later
- **Apple clang 17.0.0** (macOS) or clang 17 / gcc 13.2 (Linux)
- **vcpkg** (for dependency management)
- C++20 standard support

## Build

```bash
# Set VCPKG_ROOT if not already in your environment
export VCPKG_ROOT=/path/to/vcpkg

# Configure and build (macOS arm64 debug)
cmake --preset macos-arm64-debug
cmake --build --preset macos-arm64-debug

# Verify
./build/pie --version
```

## Project Structure

```
pie-coding-agent/
├── CMakeLists.txt            # Top-level CMake configuration
├── CMakePresets.json         # Build presets for all platforms
├── vcpkg.json                # Pinned dependency manifest
├── vcpkg-configuration.json  # vcpkg registry baseline
├── cmake/                    # CMake modules (clang enforcement, toolchains)
├── include/pie/              # Public SDK headers
├── src/                      # Source code (layered architecture)
│   ├── core/                 # Foundation: Result, Logger, JSON, utilities
│   ├── io/                   # I/O: HTTP, file locking, subprocess, watcher
│   ├── wire/                 # Wire format: JSONL, YAML, JSON Schema, diff
│   ├── auth/                 # Authentication: OAuth, API keys
│   ├── session/              # Session management and tree navigation
│   ├── settings/             # Settings: deep merge, env vars, first-run import
│   ├── models/               # Model registry
│   ├── resources/            # Resource loading: context files, skills, prompts
│   ├── tools/                # Built-in tools and tool host
│   ├── compaction/           # Compaction subsystem
│   ├── queue/                # Message queue (steering/follow-up)
│   ├── providers/            # LLM provider clients
│   ├── tui/                  # Terminal UI (FTXUI)
│   ├── extension_host/       # Extension lifecycle and bridge
│   ├── sdk/                  # Public SDK facade
│   ├── cli/                  # CLI entry point and argv parsing
│   └── modes/                # Mode drivers
│       ├── interactive/
│       ├── print/
│       ├── json/
│       ├── rpc/
│       └── export/
├── runtime/                  # Embedded assets (extension runtime, themes, export HTML)
├── tests/                    # Test suites
│   ├── unit/
│   ├── property/
│   ├── integration/
│   ├── conformance/
│   └── fixtures/
└── third_party/              # Vendored header-only libraries (stb, dtl)
```

## Build Outputs

- `build/pie` — Single CLI binary
- `build/libpie.a` — Static SDK library
- `include/pie/` — Public SDK headers
