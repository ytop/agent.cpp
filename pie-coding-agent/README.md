# Pie — C++20 Coding Agent

**Pie** is a native C++20 re-implementation of the `@earendil-works/pi-coding-agent` TypeScript application, providing the same interactive coding assistant experience with a self-contained binary.

## Quick Start

```bash
# Build
cmake -S pie-coding-agent -B pie-coding-agent/build -DCMAKE_BUILD_TYPE=Release
cmake --build pie-coding-agent/build --parallel

# Verify
./pie-coding-agent/build/pie --version

# Interactive mode
./pie-coding-agent/build/pie

# Print mode (pipe-friendly)
echo "Explain this code" | ./pie-coding-agent/build/pie --print

# JSON event stream mode
./pie-coding-agent/build/pie --mode json "hello"

# Export a session to HTML
./pie-coding-agent/build/pie --export ~/.pie/agent/sessions/session.jsonl
```

## Prerequisites

| Dependency | Version | Notes |
|---|---|---|
| CMake | ≥ 3.27 | Required |
| GCC | ≥ 13 (Ubuntu) | C++23 |
| libcurl | system | HTTP client |
| OpenSSL | system | TLS |
| nlohmann/json | 3.11.3 | JSON (auto-fetched) |
| Catch2 | 3.x | Tests (auto-fetched) |
| rapidcheck | pinned commit | Property tests (auto-fetched) |

## Build Options

```bash
# Integration tests (requires /bin/sh and network for some)
cmake -DPIE_INTEGRATION_TESTS=ON ...

# Coverage build
cmake -DCMAKE_BUILD_TYPE=Coverage ...
make coverage  # Requires gcovr
```

## Project Structure

```
pie-coding-agent/
├── include/pie/          # Public headers
├── src/                  # Implementation
├── tests/
│   ├── unit/             # Catch2 unit tests
│   ├── property/         # rapidcheck property tests
│   ├── integration/      # Integration tests (gated)
│   ├── conformance/      # Wire-format conformance tests
│   └── fixtures/         # Pi_TS-produced golden fixtures
├── scripts/
│   └── generate_fixtures.sh  # Generate/refresh golden fixtures
├── docs/                 # Documentation
└── CMakeLists.txt
```

## Modes

| Mode | Flag | Description |
|---|---|---|
| Interactive | *(default)* | Full TUI with rich rendering |
| Print | `--print` | Stream response text to stdout |
| JSON | `--mode json` | Stream events as JSONL to stdout |
| RPC | `--mode rpc` | JSON-RPC protocol on stdin/stdout |
| Export | `--export <file>` | Render session to self-contained HTML |

## CLI Reference

See [docs/cli.md](docs/cli.md) for the full CLI reference.

## Session Format

Pie reads and writes the same JSONL session format as Pi_TS (`~/.pie/agent/sessions/`). Existing Pi_TS sessions at `~/.pi/agent/` are automatically discovered on first run.

See [docs/session-format.md](docs/session-format.md).

## Providers & Authentication

```bash
# Set API key
export ANTHROPIC_API_KEY="sk-ant-..."
export OPENAI_API_KEY="sk-..."

# Or use OAuth device-code flow
pie --provider anthropic /login
```

See [docs/providers.md](docs/providers.md).

## Extensions

Pie supports both:
- **JS/TS extensions** — executed out-of-process via Node (same API as Pi_TS)
- **Native C++ plugins** — loaded via `dlopen`

```bash
pie install npm:@my-scope/my-extension
pie install ./local/extension
```

See [docs/extensions.md](docs/extensions.md).

## Migration from Pi_TS

Pie is designed for zero-friction migration. On first run it automatically imports configuration from `~/.pi/agent/` and discovers Pi_TS sessions in the legacy location.

See [docs/migration.md](docs/migration.md).

## Testing

```bash
# Unit tests
./build/pie_tests

# Property-based tests
./build/pie_property_tests

# Integration tests
cmake -DPIE_INTEGRATION_TESTS=ON .. && ./build/pie_integration_tests

# Conformance tests (generate fixtures first)
bash scripts/generate_fixtures.sh
./build/pie_conformance_tests
```

## License

MIT
