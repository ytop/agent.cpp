# Pie C++20 Coding Agent (`agent.cpp`)

**Pie** is a high-performance, native C++23 re-implementation of the `@earendil-works/pi-coding-agent` TypeScript agent. It provides a robust, self-contained interactive coding assistant with a beautiful terminal interface (TUI), powerful multi-mode operations (JSON, RPC, Print, Export), and zero dependencies at runtime.

---

## 🚀 Key Features

*   **⚡ Native Performance**: Built on a modern C++23 foundation for near-instant startup, minimal memory consumption, and extreme responsiveness.
*   **💻 Rich TUI Experience**: Beautiful interactive terminal dashboard offering seamless navigation, visual workspace representation, syntax highlighting, and hot-reloadable themes.
*   **🛠️ Flexible Execution Modes**:
    *   **Interactive TUI**: Standard developer workspace mode.
    *   **Print Mode (`--print`)**: Ideal for CLI piping, scripts, and redirecting outputs.
    *   **JSON Event Stream Mode (`--mode json`)**: Headless observation mode emitting structured JSONL events.
    *   **JSON-RPC Mode (`--mode rpc`)**: Strict LF-framed communication bridge for IDE integrations.
    *   **Archival Export Mode (`--export`)**: Archiving session history as responsive, self-contained HTML reports with zero external dependencies.
*   **📦 Advanced Extensibility**: Full support for legacy JavaScript extensions (via headless Node), standard skills, prompt templates, custom themes, and high-speed native C++ plugins loaded via `dlopen`.
*   **🛡️ Robust Sandboxing & Safety**: Secure command execution layers featuring automated truncation and graceful timeout handling (SIGTERM with SIGKILL fallback).
*   **🔄 Zero-Friction Migration**: Built-in lazy discovery and lossless in-memory migration from legacy TypeScript session configurations (`v1`/`v2` format to `v3` format).

---

## 🛠️ How to Compile

### Prerequisites

Please ensure the following system tools and libraries are installed (typical for modern Ubuntu environments):

*   **CMake** (≥ 3.27)
*   **GCC** (≥ 13) or **Clang** (for C++23 support)
*   **libcurl** and **OpenSSL** development headers

```bash
# On Ubuntu / Debian:
sudo apt-get update
sudo apt-get install -y build-essential cmake libcurl4-openssl-dev libssl-dev
```

### Compiling the Project

Run the following commands to configure and build the standalone `pie` binary:

```bash
# Configure the build system (Release mode)
cmake -S pie-coding-agent -B pie-coding-agent/build -DCMAKE_BUILD_TYPE=Release

# Compile all targets using all available cores
cmake --build pie-coding-agent/build --parallel
```

The compiled binary will be placed at `pie-coding-agent/build/pie`.

---

## 🏃 How to Run

### 1. Verify Installation
Ensure that the binary is built correctly:
```bash
./pie-coding-agent/build/pie --version
```

### 2. Run Interactive Mode (TUI)
Simply invoke the binary without mode flags to enter the interactive console:
```bash
./pie-coding-agent/build/pie
```

### 3. Run Print Mode (Non-Interactive Piping)
Pipe standard input directly to the agent or force `--print`:
```bash
echo "Generate a quicksort function in C++" | ./pie-coding-agent/build/pie --print
```

### 4. Run JSON Event Stream Mode
Stream real-time structured events to standard output:
```bash
./pie-coding-agent/build/pie --mode json "Refactor this file"
```

### 5. Export Sessions to HTML
Archive session history as a beautifully formatted, standalone HTML report:
```bash
./pie-coding-agent/build/pie --export ~/.pie/agent/sessions/simple_session.jsonl
```

---

## 🧪 Testing and Verification

Pie features a layered test harness spanning Catch2 unit tests, rapidcheck property-based tests, integration tests, and conformance tests.

```bash
# Run unit & snapshot tests
./pie-coding-agent/build/pie_tests

# Run property-based tests (universals correctness via rapidcheck)
./pie-coding-agent/build/pie_property_tests

# Generate/update Pi_TS golden conformance fixtures
bash pie-coding-agent/scripts/generate_fixtures.sh

# Run wire-format conformance tests
./pie-coding-agent/build/pie_conformance_tests
```

---

## 📄 License

This project is licensed under the MIT License.
