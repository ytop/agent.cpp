# Pie — C++20 Coding Agent

A native C++20 re-implementation of the Pi coding agent, producing a single `pie` binary and an embeddable `libpie` static library.

## Supported Platforms

| Platform | Architecture | Status |
|----------|-------------|--------|
| Ubuntu 24.04 | x86_64 | Supported |
| Ubuntu 24.04 | aarch64 | Supported |

macOS and Windows are out of scope.

## Requirements

- **CMake** 3.27 or later
- **gcc/g++ 13** or later (Ubuntu 24.04 default)
- **vcpkg** (for dependency management)
- C++20 standard support

## Build

```bash
# Set VCPKG_ROOT if not already in your environment
export VCPKG_ROOT=/path/to/vcpkg

# Configure and build (Ubuntu gcc debug)
cmake --preset default
cmake --build --preset default

# Verify
./build/pie --version
```

## Project Structure

```
pie-coding-agent/
├── CMakeLists.txt            # Top-level CMake configuration
├── CMakePresets.json         # Ubuntu gcc build presets
├── vcpkg.json                # Pinned dependency manifest
├── vcpkg-configuration.json  # vcpkg registry baseline
├── cmake/                    # CMake modules (gcc enforcement, toolchains)
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
