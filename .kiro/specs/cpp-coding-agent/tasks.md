# Implementation Plan: cpp-coding-agent

## Overview

This plan implements Pie_Cpp — a clean C++20 re-implementation of the Pi_TS coding agent targeting Ubuntu 24.04 with gcc 13+ — in 103 tasks organized into 7 bottom-up phases. Each phase builds on the previous so the binary remains compilable after every task. The 7 phases are: (1) Project Scaffolding & Build System, (2) Foundation Layer, (3) Domain Layer, (4) Subsystems, (5) SDK Facade, (6) CLI & Modes, and (7) Verification & Polish. Property-based tests (36 properties, ≥100 iterations each using rapidcheck) and conformance tests against Pi_TS-produced fixtures are in Phase 7.

## Tasks

### Phase 1: Project Scaffolding & Build System

- [x] 1. Top-level CMake, vcpkg.json with pinned versions, and directory skeleton
  *Creates `pie-coding-agent/` with `CMakeLists.txt`, `CMakePresets.json`, `vcpkg.json` (all deps pinned per design table), `vcpkg-configuration.json`, and the full `src/`, `include/pie/`, `tests/`, `runtime/`, `cmake/`, `third_party/` directory tree.*
  _Requirements: 1.1, 1.2, 1.4, 1.5, 1.6_
  _Design: Build & Distribution > CMake structure / vcpkg.json_

- [x] 2. Ubuntu gcc 13+ enforcement script
  *Adds `cmake/UbuntuGcc.cmake` that verifies the system is Linux, the compiler is GNU gcc/g++, and the version is ≥ 13. Calls `message(FATAL_ERROR)` before any source file is compiled if checks fail.*
  _Requirements: 1.11_
  _Design: Build & Distribution > gcc enforcement_

- [x] 3. Vendor third-party header-only libraries
  *Copies `stb_image.h`, `stb_image_resize2.h` (commit `5c20573`), and `dtl/dtl.hpp` (commit `f3a1b22`) into `third_party/` with a `VERSIONS.md` recording exact commit hashes.*
  _Requirements: 1.6_
  _Design: Technology Stack (stb_image, stb_image_resize2, dtl)_

- [x] 4. Runtime asset layout and cmrc embedding
  *Creates `runtime/extension-runtime.cjs` (stub), `runtime/export-html/` (stub template.html/css/js + vendored highlight.js), `runtime/theme/dark.json` and `light.json` (copied from `coding-agent/src/modes/interactive/theme/`). Wires `cmrc` into CMake so assets are embedded into the binary.*
  _Requirements: 1.7, 15.1, 19.3, 19.6_
  _Design: Build & Distribution > Build outputs_

- [x] 5. `pie --version` stub binary (Phase 1 integration check)
  *Adds a minimal `src/cli/main.cpp` that parses `--version` via CLI11 and exits 0 printing a version string. Wires the `pie` executable target in CMake. Confirms `pie-coding-agent/build/pie --version` exits 0 within 5 s on Ubuntu 24.04.*
  _Requirements: 1.3, 1.7, 1.9_
  _Design: CLI Layer > Dispatch table_

- [ ] 6. CI matrix skeleton
  *Adds GitHub Actions workflow with three Ubuntu 24.04 gcc jobs: x86_64 debug, x86_64 release, and aarch64 release. Each job installs vcpkg, runs CMake configure + build, and runs `pie --version`.*
  _Requirements: 1.3, 1.5, 1.10_
  _Design: Build & Distribution > CI matrix_


### Phase 2: Foundation Layer

- [x] 7. `pie::core` — Result type, Logger, and primitive utilities
  *Implements `pie::Result<T,E>` (tl::expected typedef), `pie::core::ErrorInfo` with Category enum, `pie::core::Logger` (stderr + debug-log-file + `--verbose` flag), UUID v4 generator, 8-char hex ID generator, ISO-8601 timestamp helpers, and path normalization utilities.*
  _Requirements: 24.1, 24.2, 24.4, 24.5, 24.6_
  _Design: Components and Interfaces > Diagnostics / logging_

- [x] 8. `pie::core::JsonValue` — nlohmann/ordered_json wrapper
  *Wraps `nlohmann::ordered_json` in a `pie::core::JsonValue` type that preserves insertion order (for round-trip key order), exposes a `raw` field pattern, and provides `get<T>`, `contains`, `items`, `is_object`, `is_array` helpers used throughout the codebase.*
  _Requirements: 3.11, 3.12, 3.13, 23.7_
  _Design: Technology Stack (nlohmann/json)_

- [x] 9. `pie::io::FileLock` — proper-lockfile-equivalent file locking
  *Implements the `<file>.lock` sibling-file convention: `open(O_CREAT|O_EXCL)`, 50 ms retry up to 10 s, 10 s stale threshold, `unlink` on release. Writes `<pid>\n` to the lockfile for cross-process visibility.*
  _Requirements: 3.10_
  _Design: Components and Interfaces > Session subsystem > File locking_

- [x] 10. `pie::wire::JsonlParser` and `pie::wire::JsonlSerializer`
  *Implements the line-oriented JSONL parser (returns `(line_no, JsonValue)` or `ErrorInfo` per Req 3.14) and the canonical serializer (no insignificant whitespace, documented key order per entry type, single `\n` terminator, byte-identical across runs).*
  _Requirements: 3.4, 3.13, 3.14_
  _Design: Components and Interfaces > Session subsystem > JSONL parser and serializer_

- [x] 11. `pie::io::HttpClient` — libcurl wrapper
  *Wraps libcurl with a `pie::io::HttpClient` supporting GET/POST with headers, SSE chunked callbacks, 2000 ms timeout for telemetry/update-check calls, and OpenSSL TLS verification enabled by default.*
  _Requirements: 21.1, 21.2, 21.6_
  _Design: Technology Stack (libcurl, OpenSSL)_

- [x] 12. `pie::io::Subprocess` — posix_spawn wrapper
  *Wraps `posix_spawnp` with `posix_spawn_file_actions_t` for stdout/stderr pipe redirection, argv-array construction (no shell interpolation), environment inheritance, and a `wait()` method returning exit code.*
  _Requirements: 12.1, 12.8_
  _Design: Technology Stack (posix_spawn)_

- [x] 13. `pie::io::FsWatcher` — efsw-based filesystem watcher
  *Wraps efsw to watch a directory for file changes, debounced at 200 ms, firing a callback with the changed path. Used for theme hot reload.*
  _Requirements: 15.4, 15.6_
  _Design: Technology Stack (efsw)_

- [x] 14. `pie::wire::YamlFrontmatter` — rapidyaml frontmatter parser
  *Parses YAML frontmatter delimited by `---` lines from a Markdown string, extracting a `JsonValue` of key-value pairs. Used for prompt templates and SKILL.md files.*
  _Requirements: 13.10, 14.2_
  _Design: Technology Stack (rapidyaml)_

- [x] 15. `pie::wire::JsonSchemaValidator` — valijson adapter
  *Wraps valijson with a nlohmann/json adapter to validate a `JsonValue` against a JSON Schema object. Returns a list of validation errors or success.*
  _Requirements: 22.7_
  _Design: Technology Stack (valijson)_

- [x] 16. `pie::wire::Globber` — glob/fnmatch with `**` support
  *Wraps POSIX `glob.h` and adds a custom `**` recursive-match layer to match Pi_TS's `glob` + `minimatch` behavior. Used for settings `extensions`/`skills`/`prompts`/`themes` array patterns.*
  _Requirements: 5.5_
  _Design: Technology Stack (Glob / fnmatch)_

- [x] 17. `pie::wire::Diff` — dtl Myers diff wrapper
  *Wraps dtl to produce a unified-diff string from two text inputs, matching Pi_TS's `edit` tool `details.diff` output format.*
  _Requirements: 7.4_
  _Design: Technology Stack (dtl)_

- [x] 18. Phase 2 integration check — foundation layer compiles clean, unit tests green
  *Adds Catch2 unit tests for JsonlParser (round-trip of each entry type), FileLock (acquire/release/stale), YamlFrontmatter (basic + missing description), JsonSchemaValidator (valid + invalid schema), Globber (`**` patterns), and Diff (unified output). All pass on Ubuntu 24.04.*
  _Requirements: 3.14, 9.10_
  _Design: Testing Strategy > Unit tests_


### Phase 3: Domain Layer

- [x] 19. `pie::auth::AuthStorage` — auth.json read/write and API-key resolution chain
  *Implements `AuthStorage::create()`, `resolve_api_key(provider)` with precedence runtime > stored > env > fallback, `set_runtime_api_key`, `store_oauth`, `remove_provider`, and `auth.json` read/write preserving unknown keys. Creates `auth.json` with mode `0600`.*
  _Requirements: 6.1, 6.4, 6.5, 6.7, 6.8, 6.14_
  _Design: Components and Interfaces > Auth subsystem_

- [x] 20. All 25+ API-key provider descriptors
  *Registers a `ProviderDescriptor` for each of the 25+ API-key providers listed in Req 6.3 (Anthropic, OpenAI, Azure OpenAI, DeepSeek, Google Gemini, Google Vertex, Amazon Bedrock, Mistral, Groq, Cerebras, Cloudflare AI Gateway, Cloudflare Workers AI, xAI, OpenRouter, Vercel AI Gateway, ZAI, OpenCode Zen, OpenCode Go, Hugging Face, Fireworks, Together AI, Kimi For Coding, MiniMax, Xiaomi MiMo + 3 Token Plan regional variants). Each descriptor records the env-var name, provider id, and default endpoint.*
  _Requirements: 6.3_
  _Design: Components and Interfaces > Auth subsystem_

- [x] 21. OAuth device-code flows — Anthropic, OpenAI Codex, GitHub Copilot
  *Implements `AnthropicOAuth`, `OpenAICodexOAuth`, `GithubCopilotOAuth` as `asio::awaitable` coroutines polling between 1–5 s with a 300 s hard ceiling. On success persists tokens to `auth.json`. On timeout/cancel/error emits a diagnostic and leaves `auth.json` unchanged.*
  _Requirements: 6.2, 6.7_
  _Design: Components and Interfaces > Auth subsystem_

- [x] 22. OAuth token refresh (60-second window)
  *Implements `AuthStorage::refresh_if_needed(provider)` that initiates a refresh when `expires_at_ms - now_ms ≤ 60_000`, persists refreshed tokens, and returns the unrefreshed credentials with an error on failure.*
  _Requirements: 6.6_
  _Design: Components and Interfaces > Auth subsystem_

- [x] 23. `pie::session::SessionFile` — JSONL read, atomic crash-safe append, and file locking
  *Implements `SessionFile::open`, `SessionFile::create`, `SessionFile::append` (acquire FileLock → lseek-to-end → write full line → fsync → release lock), and `SessionFile::rewrite_v1_or_v2_to_v3` (write to `.tmp` then `rename(2)`).*
  _Requirements: 3.3, 3.4, 3.9, 3.10, 3.15_
  _Design: Components and Interfaces > Session subsystem > File locking / Atomic append_

- [x] 24. `pie::session::SessionTree` — tree navigation and `buildSessionContext`
  *Implements `SessionTree` with `leaf_id`, `get`, `branch_to_root`, `children`, `label`, `set_leaf`, `append`, and `build_session_context` following the algorithm in `coding-agent/docs/session-format.md` (walk leaf→root, extract model/thinking, apply compaction cut, convert branch_summary/custom_message).*
  _Requirements: 4.4, 4.5, 4.7, 4.8, 4.9, 4.10, 4.11, 4.12, 4.13_
  _Design: Components and Interfaces > Session subsystem_

- [x] 25. `pie::session::SessionManager` — full public API surface
  *Implements all static factories (`create`, `open`, `continueRecent`, `inMemory`, `forkFrom`), listing methods (`list`, `listAll`), all `append_*` methods returning 8-char hex IDs, all tree-navigation methods, context/info methods, and session-management methods per Req 4.*
  _Requirements: 4.1, 4.2, 4.3, 4.4, 4.5, 4.6_
  _Design: Components and Interfaces > Session subsystem_

- [x] 26. Session migration v1 → v3 on load
  *Implements `SessionFile` v1 migration: walk entries in file order, assign fresh `EntryId` to each, set `parentId` to the previous entry's id (null for first). Holds migrated tree in memory; writes v3 only on next persist. Leaves original file untouched on failure.*
  _Requirements: 23.1, 23.3, 23.4_
  _Design: Components and Interfaces > Session subsystem > Migration v1/v2 → v3_

- [x] 27. Session migration v2 → v3 on load
  *Implements `SessionFile` v2 migration: rename `hookMessage` role to `custom` on each matching message entry, preserving all other fields byte-equivalent. Handles unsupported/future version (> 3 or missing) by refusing to load with an error.*
  _Requirements: 23.2, 23.3, 23.5_
  _Design: Components and Interfaces > Session subsystem > Migration v1/v2 → v3_

- [x] 28. `pie::settings::SettingsManager` — deep merge, env resolver, and persistence
  *Implements `SettingsManager::create` (loads global + project layers), `in_memory`, `deep_merge` (pure function), `effective()`, `get`/`set`, async `flush` (write to `<file>.tmp` then `rename`), `drain_errors`, and `apply_overrides`. Recognizes all settings from `coding-agent/docs/settings.md`.*
  _Requirements: 5.1, 5.2, 5.3, 5.4, 5.5, 5.6, 5.14, 5.15_
  _Design: Components and Interfaces > Settings subsystem_

- [x] 29. Environment variable resolver with tristate parsing
  *Implements `EnvResolver` reading `PIE_CODING_AGENT_DIR`, `PIE_CODING_AGENT_SESSION_DIR`, `PIE_PACKAGE_DIR`, `PIE_OFFLINE`, `PIE_SKIP_VERSION_CHECK`, `PIE_TELEMETRY`, `PIE_CACHE_RETENTION`, `VISUAL`, `EDITOR`, `PIE_SHARE_VIEWER_URL`. Boolean vars return `Tristate{Unset, Truthy, Falsy}` per the case-insensitive value table in Req 5.9.*
  _Requirements: 5.7, 5.8, 5.9, 5.10, 5.11_
  _Design: Components and Interfaces > Settings subsystem > Environment variables_

- [x] 30. First-run import from `~/.pi/agent` to `~/.pie/agent`
  *Implements `pie::settings::FirstRunImport::run(agent_dir, ts_agent_dir)`: probes `<Agent_Dir>` for all canonical files, probes `~/.pi/agent/` if all absent, copies matching files/dirs verbatim, writes `.import-from-pi.json` marker. Session discovery merges both `<Session_Dir>` and `~/.pi/agent/sessions/` for `--continue`/`--resume`.*
  _Requirements: 23.8, 23.9_
  _Design: Components and Interfaces > Settings subsystem > First-run import_

- [x] 31. `pie::models::ModelRegistry` — built-in model list codegen and models.json merge
  *Adds `scripts/sync-builtin-models.mjs` (build-time codegen from Pi_TS reference commit → `builtin_models.gen.cpp`). Implements `ModelRegistry::create`, `find`, `get_available` (filtered by valid creds), `all`, and `parse_selector` for `[provider/]id[:thinking]` patterns.*
  _Requirements: 6.9, 6.10, 6.11, 6.12_
  _Design: Components and Interfaces > Model registry_

- [x] 32. `pie::resources::ResourceLoader` — AGENTS.md / CLAUDE.md / SYSTEM.md discovery
  *Implements discovery of `AGENTS.md` and `CLAUDE.md` in the documented order (Agent_Dir → ancestors from FS root → cwd), concatenation with `\n` separator, `SYSTEM.md` / `APPEND_SYSTEM.md` override logic (project > global), and `--system-prompt` / `--append-system-prompt` CLI overrides.*
  _Requirements: 13.1, 13.2, 13.3, 13.4, 13.5, 13.6, 13.12_
  _Design: Components and Interfaces > Context files / system prompt / prompt templates_

- [x] 33. Prompt templates — discovery, frontmatter, and `{{variable}}` substitution
  *Implements non-recursive `*.md` discovery from `<Agent_Dir>/prompts/`, `<cwd>/.pie/prompts/`, and Pie_Package prompt dirs with project > user > package precedence. Parses YAML frontmatter (`description`, `argument-hint`). Substitutes `{{variable}}` from CLI arg or interactive prompt. Emits diagnostic and leaves buffer unchanged on missing variable.*
  _Requirements: 13.7, 13.8, 13.9, 13.10, 13.11_
  _Design: Components and Interfaces > Context files / system prompt / prompt templates_

- [x] 34. Skills — discovery, validation, progressive disclosure, and `/skill:<name>` dispatch
  *Implements the seven-source discovery order (Req 14.1), `SKILL.md` validation (missing `description` → skip + diagnostic), name-collision first-wins-with-warning, system-prompt injection of name+description only (not full body), `disable-model-invocation` frontmatter handling, `/skill:<name> [args]` injection, unknown-skill-name error path, and `--no-skills` / `enableSkillCommands: false` behavior.*
  _Requirements: 14.1, 14.2, 14.3, 14.4, 14.5, 14.6, 14.7, 14.8, 14.9_
  _Design: Components and Interfaces > Skills_

- [x] 35. Themes — discovery, conflict resolution, hot reload, and error handling
  *Implements top-level `.json` discovery from `<Agent_Dir>/themes/`, `<cwd>/.pie/themes/`, and Pie_Package theme dirs with project > user > package precedence. Registers built-in `dark` and `light` themes from embedded constexpr JSON. Wires `FsWatcher` for hot reload within 1000 ms. Retains previous theme on reload failure.*
  _Requirements: 15.1, 15.2, 15.3, 15.4, 15.5, 15.6, 15.7_
  _Design: Components and Interfaces > Themes_

- [x] 36. Phase 3 integration check — domain layer compiles clean, unit tests green
  *Adds Catch2 unit tests for AuthStorage (API-key precedence chain, `auth.json` round-trip), SessionManager (append → branch → buildSessionContext), SettingsManager (deep merge, env resolver, flush), ModelRegistry (find, parse_selector), ResourceLoader (AGENTS.md discovery order, SYSTEM.md override), and FirstRunImport (marker file creation). All pass on Ubuntu 24.04.*
  _Requirements: 5.12, 5.13, 6.14_
  _Design: Testing Strategy > Unit tests_


### Phase 4: Subsystems

- [ ] 37. `pie::tools::Tool` interface and `pie::tools::ToolHost` with Tool_Allowlist
  *Defines the `Tool` abstract base class (name, label, description, parameters_schema, execute). Implements `ToolHost` with `register_tool`, `apply_allowlist`, `is_allowed`, `find`, `all_allowed`. Implements `build_allowlist` from CLI flags with precedence `--no-tools > --no-builtin-tools > --tools`.*
  _Requirements: 7.1, 7.7, 7.8, 7.9, 7.10, 7.11, 7.12, 7.13_
  _Design: Components and Interfaces > Tool subsystem_

- [ ] 38. Built-in tools — `read`, `write`, `edit`, `ls`
  *Implements `ReadTool` (line-range support, error on non-existent path), `WriteTool` (overwrite + parent-dir creation), `EditTool` (dtl unified diff in `details.diff`, no-modify on failure), `LsTool` (posix opendir/readdir). JSON Schema parameters match Pi_TS.*
  _Requirements: 7.2, 7.3, 7.4, 7.6_
  _Design: Components and Interfaces > Tool subsystem_

- [ ] 39. Built-in tools — `grep`, `find`
  *Implements `GrepTool` (ripgrep-equivalent local impl using POSIX regex + recursive dir walk) and `FindTool` (Globber-based filename search). Parameters and result shapes match Pi_TS.*
  _Requirements: 7.6_
  _Design: Components and Interfaces > Tool subsystem_

- [ ] 40. `pie::tools::BashExecutor` — process spawn, output capture, truncation, cancellation, timeout
  *Implements `BashExecutor::run(BashRequest)`: spawn via `posix_spawnp` with `shellPath`/`shellCommandPrefix`, capture stdout+stderr together, truncate at Pi_TS threshold (32 KiB) writing remainder to `mkstemp` temp file, SIGTERM+grace+SIGKILL cancellation on Esc or timeout, `excludeFromContext` for `!!` prefix, spawn-failure sentinel exit code.*
  _Requirements: 12.1, 12.2, 12.3, 12.4, 12.5, 12.6, 12.7, 12.8, 12.9, 12.10_
  _Design: Components and Interfaces > Bash executor_

- [ ] 41. `pie::compaction::Compactor` — manual, threshold, overflow, and branch summary
  *Implements `Compactor::run(CompactionRequest)` for all three reasons (manual/threshold/overflow), cut-point rules (user/assistant/bashExecution/custom only), `compaction_start`/`compaction_end` event emission, overflow retry exactly once, failure path (no entry appended, `aborted: true`). Implements `summarize_branch` for `/tree` navigation.*
  _Requirements: 8.1, 8.2, 8.3, 8.4, 8.5, 8.6, 8.7, 8.8, 8.9_
  _Design: Components and Interfaces > Compaction subsystem_

- [ ] 42. `pie::queue::MessageQueue` — steering/follow-up FIFO with delivery modes
  *Implements `push_steering`, `push_follow_up`, `drain_steering(mode)`, `drain_follow_up(mode)`, `drain_all()`, `steering_size`, `follow_up_size`. `OneAtATime` returns at most one message; `All` returns the full queue in FIFO order. Thread-safe via `std::mutex`.*
  _Requirements: 9.1, 9.2, 9.3, 9.4, 9.5, 9.6, 9.7, 9.8, 9.11, 9.12, 9.13_
  _Design: Components and Interfaces > Message queue_

- [ ] 43. Provider transport layer — SSE and WebSocket
  *Implements `SseTransport` (libcurl chunked write callback, SSE event parsing) and `WebSocketTransport` (asio TLS streams + RFC 6455 framing). Implements `RetryPolicy` with exponential backoff, `provider.maxRetryDelayMs` cap, and `auto_retry_start`/`auto_retry_end` events.*
  _Requirements: 5.5 (transport setting), 8.4_
  _Design: Components and Interfaces > Provider/model system_

- [ ] 44. Provider clients — Anthropic and OpenAI
  *Implements `AnthropicClient` and `OpenAIClient` (including Codex) as `Provider` subclasses using the SSE/WebSocket transport. Handles streaming text_delta, thinking_delta, toolcall_*, done, and error events. Maps provider-native overflow errors to `pie::providers::Error{kind=OVERFLOW}`.*
  _Requirements: 6.3_
  _Design: Components and Interfaces > Provider/model system_

- [ ] 45. Provider clients — Google (Gemini + Vertex), Azure OpenAI, Amazon Bedrock
  *Implements `GoogleClient` (Gemini + Vertex), `AzureOpenAIClient`, and `BedrockClient` as `Provider` subclasses.*
  _Requirements: 6.3_
  _Design: Components and Interfaces > Provider/model system_

- [ ] 46. Provider clients — remaining 20+ API-key providers
  *Implements `DeepSeekClient`, `MistralClient`, `GroqClient`, `CerebrasClient`, `CloudflareAiGatewayClient`, `CloudflareWorkersAiClient`, `XaiClient`, `OpenRouterClient`, `VercelAiGatewayClient`, `ZaiClient`, `OpenCodeZenClient`, `OpenCodeGoClient`, `HuggingFaceClient`, `FireworksClient`, `TogetherAiClient`, `KimiForCodingClient`, `MiniMaxClient`, `XiaomiMiMoClient`, and the three Xiaomi Token Plan regional variants.*
  _Requirements: 6.3_
  _Design: Components and Interfaces > Provider/model system_

- [ ] 47. `pie::tui::TuiRuntime` — FTXUI screen, layout, and theme application
  *Implements `TuiRuntime` with FTXUI `ScreenInteractive`, the main layout (header bar, message list, editor, footer bar), theme application (color map → FTXUI Color), `SIGWINCH` reflow, and the `apply_theme` hot-reload path.*
  _Requirements: 11.2, 11.3, 11.7_
  _Design: Components and Interfaces > TUI runtime_

- [ ] 48. TUI editor component — file fuzzy completion, path completion, multi-line, image paste
  *Implements `EditorComponent` with `@`-prefixed file fuzzy completion, Tab path completion, Shift+Enter / Ctrl+Enter multi-line, Ctrl+V / Alt+V clipboard image paste, drag-and-drop image attachment, `!command` / `!!command` prefix detection.*
  _Requirements: 11.1_
  _Design: Components and Interfaces > TUI runtime_

- [ ] 49. TUI message rendering — all message types and diff display
  *Implements `MessageList` rendering for assistant messages, user messages, thinking blocks (collapsible via Ctrl+T), tool calls and results (collapsible via Ctrl+O), bash execution entries, branch summaries, compaction summaries, and custom (extension) entries. Renders unified diff for `edit`/`write` tool calls.*
  _Requirements: 11.4, 11.5, 11.9, 11.10_
  _Design: Components and Interfaces > TUI runtime_

- [ ] 50. TUI modals and selectors — model, scoped-models, session search, settings, theme, thinking level
  *Implements FTXUI `Component` modals for: model selector (sorted list, Esc to cancel), scoped-models selector, session selector with search, settings editor, theme selector, thinking-level selector.*
  _Requirements: 11.6_
  _Design: Components and Interfaces > TUI runtime_

- [ ] 51. TUI modals and selectors — tree view, fork/user-message, login dialog, OAuth selector, show-images, extension selector, config selector, hotkeys, changelog, dynamic-border loader, countdown timer
  *Implements the remaining FTXUI modals: tree view (fold/unfold, filter modes, label/timestamp toggle), fork/user-message selector, login dialog, OAuth provider selector, show-images toggle, extension selector, config selector, keybinding hints (`/hotkeys`), changelog display, dynamic-border loading indicator, countdown timer.*
  _Requirements: 11.6_
  _Design: Components and Interfaces > TUI runtime_

- [ ] 52. `pie::tui::ImageRenderer` — Kitty, Sixel, and placeholder
  *Implements `ImageRenderer::detect()` (Kitty → Sixel → Placeholder via `TERM`, `KITTY_WINDOW_ID`, DA1/DA2 query) and `render(ImageData, width_cells)` emitting the appropriate escape sequence directly to stdout after the FTXUI frame flush.*
  _Requirements: 11.8, 20.7, 20.8_
  _Design: Components and Interfaces > TUI runtime / Image handling_

- [ ] 53. `pie::io::ImagePipeline` — decode, EXIF orientation, auto-resize, base64
  *Implements `ImagePipeline::decode` (stb_image for PNG/JPEG/GIF/WebP, 20 MB limit, unsupported-format rejection), `apply_exif_orientation` (libexif Orientation 1–8, normalize to 1), `auto_resize` (stb_image_resize2, larger dim → 2000 px, ±1 px aspect ratio), `base64_encode`.*
  _Requirements: 20.1, 20.2, 20.3, 20.4, 20.5, 20.6, 20.9_
  _Design: Components and Interfaces > Image handling_

- [ ] 54. `pie::extension_host::ExtensionHost` — out-of-process Node bridge for JS extensions
  *Implements `JsExtension` (spawns `node runtime/extension-runtime.cjs <ext-path>` via `Subprocess`, JSON-RPC bridge over stdio), `ExtensionHost::load_all`, `dispatch` (fan-out), `unload_all`. Emits `extension_error` with `extensionPath`/`event`/`error` on handler throw or load failure. Skips JS extensions with a diagnostic when `node` is absent.*
  _Requirements: 17.1, 17.2, 17.3, 17.4, 17.9, 17.10_
  _Design: Components and Interfaces > Extension subsystem_

- [ ] 55. Extension bridge protocol — `extension-runtime.cjs` and JSON-RPC method handlers
  *Writes the Node-side `runtime/extension-runtime.cjs` that loads the user's extension via `jiti`, exposes the `ExtensionAPI` surface (`registerTool`, `registerCommand`, `registerShortcut`, `registerFlag`, `registerProvider`, `on`, `events`, `sendMessage`, `setSessionName`, `appendEntry` family) over JSON-RPC on stdin/stdout.*
  _Requirements: 17.6, 17.7_
  _Design: Components and Interfaces > Extension subsystem > Bridge protocol_

- [ ] 56. Native plugin ABI for C++ extensions
  *Defines `pie-coding-agent/include/pie/extension/plugin_abi.hpp` with the C ABI `extern "C" void pie_extension_register(PieExtensionVtable*)`. Implements `NativeExtension` (`dlopen`/`dlsym`, vtable dispatch). Documents the ABI in `pie-coding-agent/docs/native-extension-abi.md`.*
  _Requirements: 17.1, 17.8_
  _Design: Components and Interfaces > Extension subsystem_

- [ ] 57. Extension UI sub-protocol for RPC mode
  *Implements `ExtensionUiBridge` handling `extension_ui_request` / `extension_ui_response` for `select`, `confirm`, `input`, `editor`, `notify`, `setStatus`, `setWidget`, `setTitle`, `set_editor_text` per `coding-agent/docs/rpc.md`.*
  _Requirements: 17.5_
  _Design: Components and Interfaces > Extension subsystem > Bridge protocol_

- [ ] 58. `pie::sdk::packages::PackageManager` — install, remove, update, list, config
  *Implements source parser (6 URL shapes, 2048-char limit), `install` (npm/git/https/ssh resolvers, cleanup on failure), `remove`, `update` (skip pinned unless explicit), `list` (global + project-local), `config` (FTXUI selector modal), `parse_manifest` (`pi` key with `pie` fallback, auto-discover from conventional dirs), `npmCommand` override, offline gating.*
  _Requirements: 16.1, 16.2, 16.3, 16.4, 16.5, 16.6, 16.7, 16.8, 16.9, 16.10, 16.11_
  _Design: Components and Interfaces > Pi/Pie packages_

- [ ] 59. `pie::sdk::export_html::Exporter` and `GistUploader`
  *Implements `Exporter::render` (materializes cmrc-embedded template assets into self-contained HTML with no external resource references), `export_to` (write to file, default path = session dir), `GistUploader::upload` (GitHub API POST with 30 s timeout, returns gist URL using `PIE_SHARE_VIEWER_URL`).*
  _Requirements: 19.1, 19.2, 19.3, 19.4, 19.5, 19.6, 19.7, 19.8, 19.9, 19.10_
  _Design: Components and Interfaces > HTTP export / sharing_

- [ ] 60. `pie::sdk::telemetry::StartupChecks` — update check, telemetry, install-method detection
  *Implements `StartupChecks::run` (update check GET with 2000 ms timeout, install/update telemetry POST with 2000 ms timeout, non-fatal on failure), `detect_install_method` (Npm/Pnpm/Yarn/NativeBinary/Unknown heuristic), first-install marker detection, `PIE_OFFLINE`/`PIE_SKIP_VERSION_CHECK`/`PIE_TELEMETRY` gating.*
  _Requirements: 21.1, 21.2, 21.3, 21.4, 21.5, 21.6, 21.7_
  _Design: Components and Interfaces > Telemetry / update checks_

- [ ] 61. Phase 4 integration check — subsystems compile clean, unit tests green
  *Adds Catch2 unit tests for ToolHost (allowlist enforcement), BashExecutor (truncation, spawn failure), Compactor (cut-point selection), MessageQueue (drain modes), ImagePipeline (decode + resize + EXIF), ImageRenderer (protocol detection), PackageManager (source parser), Exporter (no external resource refs in output). All pass on Ubuntu 24.04.*
  _Requirements: 7.13, 9.9, 9.10, 12.3_
  _Design: Testing Strategy > Unit tests_


### Phase 5: Mode-agnostic Facade & SDK

- [ ] 62. `pie::sdk::AgentSession` — core agent loop (prompt, steer, followUp, subscribe)
  *Implements `AgentSession::prompt` (append user message → stream provider → dispatch tool calls → drain steering queue → repeat until idle), `steer` (push to steering queue), `follow_up` (push to follow-up queue), `subscribe` (returns unsubscribe handle that removes subscriber within next dispatch cycle).*
  _Requirements: 22.1, 9.1, 9.2, 9.3, 9.4, 9.5, 9.6_
  _Design: Public API (SDK) > pie::AgentSession_

- [ ] 63. `pie::sdk::AgentSession` — model/thinking control, compaction, abort, dispose, navigateTree
  *Implements `setModel`, `setThinkingLevel`, `cycleModel` (scoped-models FIFO wraparound), `cycleThinkingLevel` (canonical order, skip unsupported), `compact`, `abortCompaction`, `abort` (cancellation signal propagation), `dispose` (resource release, subsequent calls return errors), `navigateTree`.*
  _Requirements: 22.1, 10.1, 10.2, 10.3, 10.4, 10.5, 10.6, 10.7, 10.8, 10.9_
  _Design: Public API (SDK) > pie::AgentSession_

- [ ] 64. `pie::sdk::AgentSession` — state accessors
  *Implements `agent()`, `model()`, `thinkingLevel()`, `messages()`, `isStreaming()`, `sessionFile()`, `sessionId()`.*
  _Requirements: 22.1_
  _Design: Public API (SDK) > pie::AgentSession_

- [ ] 65. `pie::sdk::AgentSessionRuntime` — newSession, switchSession, fork, importFromJsonl
  *Implements `AgentSessionRuntime` with atomic `runtime.session` replacement (prior subscribers stay on prior session, new session gets new subscribers), failure-preserves-prior-session semantics, `newSession`, `switchSession`, `fork`, `importFromJsonl`.*
  _Requirements: 22.2_
  _Design: Public API (SDK) > pie::AgentSessionRuntime_

- [ ] 66. `pie::defineTool` — tool definition with schema validation
  *Implements `pie::define_tool(ToolDefinition)` that validates `parameters_schema` via valijson, returns `Error::InvalidArgument` on missing fields or invalid schema, and registers the tool with `ToolHost`.*
  _Requirements: 22.7_
  _Design: Public API (SDK) > pie::defineTool_

- [ ] 67. `pie::AuthStorage`, `pie::ModelRegistry`, `pie::SettingsManager`, `pie::SessionManager`, `pie::ResourceLoader` — public SDK re-exports
  *Re-exports the domain-layer types into the `pie::` namespace via `pie-coding-agent/include/pie/` headers. Adds `pie::InteractiveMode`, `pie::PrintMode`, `pie::JsonMode`, `pie::RpcMode` forward declarations.*
  _Requirements: 22.3, 22.4, 22.5, 22.6, 22.8, 22.9_
  _Design: Public API (SDK) > Header layout_

- [ ] 68. Public umbrella header `pie/pie.hpp`, version stamp, and ABI exports
  *Creates `pie-coding-agent/include/pie/pie.hpp` including all sub-headers. Adds `pie/version.hpp` with `PIE_VERSION_MAJOR/MINOR/PATCH`. Marks public symbols with `__attribute__((visibility("default")))` for the shared library variant.*
  _Requirements: 1.7_
  _Design: Public API (SDK) > Header layout_

- [ ] 69. Phase 5 integration check — SDK compiles clean, `libpie.a` links, unit tests green
  *Adds Catch2 unit tests for `AgentSession` (subscribe/unsubscribe, dispose-then-call returns error), `AgentSessionRuntime` (atomic switch, prior-subscriber isolation), `define_tool` (invalid schema returns error). Confirms `libpie.a` is produced at `pie-coding-agent/build/libpie.a`.*
  _Requirements: 22.1, 22.2, 22.7_
  _Design: Testing Strategy > Unit tests_


### Phase 6: CLI Layer & Modes

- [ ] 70. `pie::cli::CliInvocation` parser — CLI11 configuration and `@file` expansion
  *Configures CLI11 with all flags from Req 2 (model, session, tool, resource, system-prompt, misc). Implements `@path` positional detection (text vs image MIME), `message_tokens` concatenation, package subcommand detection, conflict-flag detection (mode flags, session flags), unknown-flag error with usage hint.*
  _Requirements: 2.1, 2.9, 2.10, 2.11, 2.12, 2.13, 2.14, 2.15, 2.16, 2.17, 2.18_
  _Design: CLI Layer_

- [ ] 71. Mode detection and dispatch table
  *Implements `detect_mode` (TTY-aware, conflict check), `dispatch(CliInvocation)` routing to InteractiveMode / PrintMode / JsonMode / RpcMode / ExportMode / package subcommands / Help / Version / list-models. Implements exit-code policy (0/1/2/3/4).*
  _Requirements: 2.2, 2.3, 2.8, 2.9, 24.3, 24.4, 24.5_
  _Design: CLI Layer > Mode detection / Dispatch table_

- [ ] 72. Slash command registry and all 20 built-in slash commands
  *Implements `CommandRegistry` with `register_builtin`, `register_extension`, `register_prompt_template`, `dispatch`, and name-collision precedence (built-in > extension > prompt-template). Wires all 20 commands: `/login`, `/logout`, `/model`, `/scoped-models`, `/settings`, `/resume`, `/new`, `/name`, `/session`, `/tree`, `/fork`, `/clone`, `/compact`, `/copy`, `/export`, `/share`, `/reload`, `/hotkeys`, `/changelog`, `/quit`.*
  _Requirements: 18.1, 18.2_
  _Design: Components and Interfaces > Slash commands and keybindings_

- [ ] 73. Keybinding registry — JSON overrides, namespaced/legacy migration, round-trip
  *Implements `KeybindingRegistry::load(keybindings.json)`, `apply_user_overrides`, `match(KeyEvent)`, `serialize_for_legacy`. Migrates pre-namespaced ids (e.g., `cursorUp` → `editor.cursor.up`) on load. Emits diagnostic and falls back to defaults for invalid entries. Preserves original on-disk form for round-trip.*
  _Requirements: 18.3, 18.4, 18.5, 18.6_
  _Design: Components and Interfaces > Slash commands and keybindings_

- [ ] 74. `pie::modes::interactive::InteractiveMode` driver
  *Implements `InteractiveMode::run(AgentSessionRuntime&)`: boots `TuiRuntime`, wires `AgentSession` events to TUI components, handles Esc (abort + queue restore), Alt+Up (queue retrieve), double-Esc (configurable action), Ctrl+C (clear / double-quit), all keybinding actions, and the `--verbose` startup banner.*
  _Requirements: 2.2, 9.7, 9.8, 11.1, 11.2, 11.3, 11.4, 11.5, 11.6, 11.7, 11.8, 11.9, 11.10, 11.11, 11.12, 11.13, 24.2_
  _Design: Components and Interfaces > Modes_

- [ ] 75. `pie::modes::print::PrintMode` driver
  *Implements `PrintMode::run(AgentSession&, initial_prompt)`: reads stdin when not a TTY (prepend with `\n`), sends prompt, streams response text to stdout only (no chrome), exits 0 on success, exits 1 on provider/auth/IO error with stderr diagnostic.*
  _Requirements: 2.3, 2.4, 2.5, 24.3, 24.5_
  _Design: Components and Interfaces > Modes_

- [ ] 76. `pie::modes::json::JsonMode` driver
  *Implements `JsonMode::run(AgentSession&, initial_prompt)`: emits Session_Header as first JSONL line, serializes all `AgentSession` events as JSONL on stdout (one object per `\n`), writes diagnostics to stderr only.*
  _Requirements: 2.6_
  _Design: Components and Interfaces > Modes_

- [ ] 77. `pie::modes::rpc::RpcMode` driver
  *Implements `RpcMode::run(AgentSessionRuntime&)`: strict LF-only record framing on stdin/stdout, `\r\n` tolerance (strip trailing `\r`), rejection of U+2028/U+2029 as record separators, command dispatch per `coding-agent/docs/rpc.md`, extension UI sub-protocol bridging.*
  _Requirements: 2.7_
  _Design: Components and Interfaces > Modes_

- [ ] 78. `pie::modes::export_html::ExportMode` driver and `--export` CLI path
  *Implements `ExportMode::run(in, out)`: validates `<in>` exists and is a parseable Session_File (exits non-zero with diagnostic on failure), renders to HTML, writes to `<out>` or `<in>.html` default, exits 0 on success.*
  _Requirements: 2.8, 19.4, 19.5_
  _Design: Components and Interfaces > Modes_

- [ ] 79. Phase 6 integration check — full binary compiles clean, all modes reachable
  *Confirms `pie --version`, `pie --help`, `pie --mode json`, `pie --mode rpc`, `pie --export <fixture>`, `pie install`, `pie list` all exit with the correct exit codes. Adds Catch2 unit tests for `CliInvocation` parser (all flags, `@file` expansion, conflict detection, unknown-flag error).*
  _Requirements: 2.1, 2.9, 2.15, 2.16, 2.17, 2.18_
  _Design: Testing Strategy > Unit tests_


### Phase 7: Verification & Polish

- [ ] 80. Property-based tests — Properties 1 and 2: session entry round-trip and TS_Wire_Format unknown-key preservation
  *Implements `p01_session_entry_round_trip.cpp` and `p02_ts_wire_format_round_trip.cpp` using rapidcheck with ≥100 iterations each. Generators produce random `SessionEntry` values of all types and random `auth.json`/`settings.json`/`keybindings.json`/theme JSON byte streams.*
  _Requirements: 3.5, 3.11, 3.12, 3.13, 5.15, 6.1, 6.14, 15.8, 18.7, 23.6, 23.7_
  _Validates: Property 1, Property 2_

- [ ] 81. Property-based tests — Properties 3 and 4: `buildSessionContext` and session-tree append invariants
  *Implements `p03_build_session_context.cpp` and `p04_session_tree_append.cpp`. Generators produce random `SessionTree` values with arbitrary append sequences and branch operations.*
  _Requirements: 4.7, 4.8, 3.9, 4.3, 4.10, 4.13_
  _Validates: Property 3, Property 4_

- [ ] 82. Property-based tests — Properties 5 and 6: deep merge and Tool_Allowlist
  *Implements `p05_deep_merge.cpp` and `p06_tool_allowlist.cpp`. Generators produce random JSON object pairs and random CLI-flag combinations.*
  _Requirements: 5.4, 5.12, 5.13, 7.7, 7.8, 7.9, 7.10, 7.13_
  _Validates: Property 5, Property 6_

- [ ] 83. Property-based tests — Properties 7, 8, and 9: message queue, migration, and excludeFromContext
  *Implements `p07_message_queue.cpp`, `p08_migration.cpp`, `p09_exclude_from_context.cpp`. Generators produce random push/drain sequences, random v1/v2 session byte streams, and random session trees with `excludeFromContext` entries.*
  _Requirements: 9.1, 9.9, 9.10, 9.12, 9.13, 23.1, 23.2, 23.3, 12.7_
  _Validates: Property 7, Property 8, Property 9_

- [ ] 84. Property-based tests — Properties 10 and 11: bash truncation and metachar byte-equivalence
  *Implements `p10_bash_truncation.cpp` and `p11_bash_metachar_byte_equivalence.cpp`. Property 11 compares Pie_Cpp's spawn argv against Pi_TS's for the same command string containing all documented metacharacters.*
  _Requirements: 12.3, 12.4, 12.8_
  _Validates: Property 10, Property 11_

- [ ] 85. Property-based tests — Properties 12, 13, and 14: image decode/encode, resize, and EXIF orientation
  *Implements `p12_image_round_trip.cpp`, `p13_image_resize.cpp`, `p14_exif_orientation.cpp`. Generators produce random valid PNG/JPEG/GIF/WebP byte streams and random EXIF Orientation values 1–8.*
  _Requirements: 20.1, 20.3, 20.5_
  _Validates: Property 12, Property 13, Property 14_

- [ ] 86. Property-based tests — Properties 15, 16, 17, and 18: argv parser, mode conflict, model selector, package source
  *Implements `p15_argv_parser.cpp`, `p16_mode_conflict.cpp`, `p17_model_selector.cpp`, `p18_package_source.cpp`. Generators produce random argv vectors, random mode-flag subsets, random selector strings, and random source strings.*
  _Requirements: 2.1, 2.9, 6.12, 16.1_
  _Validates: Property 15, Property 16, Property 17, Property 18_

- [ ] 87. Property-based tests — Properties 19, 20, 21, and 22: resource discovery, tristate env, API-key precedence, OAuth refresh
  *Implements `p19_resource_discovery.cpp`, `p20_tristate_env.cpp`, `p21_api_key_precedence.cpp`, `p22_oauth_refresh_window.cpp`. Generators produce random filesystem layouts, random env-var string values, random credential combinations, and random `(expires_at_ms, now_ms)` pairs.*
  _Requirements: 13.1, 13.7, 14.1, 15.2, 17.2, 5.9, 21.5, 6.4, 6.6_
  _Validates: Property 19, Property 20, Property 21, Property 22_

- [ ] 88. Property-based tests — Properties 23, 24, 25, and 26: model filtering, thinking cycle, atomic append, self-contained HTML
  *Implements `p23_models_filter.cpp`, `p24_thinking_cycle.cpp`, `p25_atomic_append.cpp`, `p26_self_contained_html.cpp`. Property 25 simulates interrupted appends and verifies the file ends at a complete line.*
  _Requirements: 6.11, 10.6, 3.15, 19.3_
  _Validates: Property 23, Property 24, Property 25, Property 26_

- [ ] 89. Property-based tests — Properties 27–36: install method, parser error contract, extension error, diagnostic format, keybindings merge, command collision, frontmatter substitution, image protocol precedence, session-dir precedence, thinkingBudgets fallback
  *Implements `p27_install_method.cpp` through `p36_thinking_budgets_fallback.cpp`. Each file covers one property with ≥100 iterations and a custom generator.*
  _Requirements: 21.7, 24.6, 17.4, 17.10, 24.7, 24.1, 18.4, 18.7, 18.2, 13.10, 13.11, 20.7, 5.7, 10.9_
  _Validates: Property 27, Property 28, Property 29, Property 30, Property 31, Property 32, Property 33, Property 34, Property 35, Property 36_

- [ ] 90. Integration tests — PrintMode with mock provider and stdin pipe
  *Adds integration tests for `PrintMode`: mock provider returning a fixed response, stdin pipe merge (Req 2.5), exit-code-0 on success, exit-code-1 on provider error with stderr diagnostic.*
  _Requirements: 2.3, 2.4, 2.5, 24.3, 24.5_
  _Design: Testing Strategy > Integration tests_

- [ ] 91. Integration tests — JsonMode and RpcMode end-to-end
  *Adds integration tests for `JsonMode` (Session_Header as first line, all event types emitted, diagnostics on stderr only) and `RpcMode` (LF framing, `\r\n` tolerance, U+2028/U+2029 rejection, command round-trip).*
  _Requirements: 2.6, 2.7_
  _Design: Testing Strategy > Integration tests_

- [ ] 92. Integration tests — BashExecutor with real `/bin/sh`, timeout, and cancellation
  *Adds integration tests running real shell commands: output capture, truncation at threshold, timeout (SIGTERM → SIGKILL), Esc-driven cancellation, `!!` excludeFromContext flag.*
  _Requirements: 12.1, 12.2, 12.3, 12.5, 12.7, 12.9_
  _Design: Testing Strategy > Integration tests_

- [ ] 93. Integration tests — Compactor with mock provider returning overflow errors
  *Adds integration tests for threshold compaction (mock provider returning usage near context window), overflow recovery (mock provider returning overflow error → compact → retry), and branch summarization.*
  _Requirements: 8.3, 8.4, 8.6_
  _Design: Testing Strategy > Integration tests_

- [ ] 94. Integration tests — theme hot reload and extension host
  *Adds integration tests for theme hot reload (write JSON change, verify TUI update within 1000 ms) and extension host (load a sample JS extension, verify event flow and `extension_error` on throw).*
  _Requirements: 15.6, 17.4, 17.10_
  _Design: Testing Strategy > Integration tests_

- [ ] 95. Integration tests — PackageManager against mock npm registry and local git repo
  *Adds integration tests for `pie install npm:pkg`, `pie install git:host/user/repo`, `pie remove`, `pie update`, `pie list`, and `pie config` against a local mock npm registry and a local bare git repo.*
  _Requirements: 16.2, 16.3, 16.4, 16.5, 16.7, 16.8_
  _Design: Testing Strategy > Integration tests_

- [ ] 96. Conformance tests — Pi_TS-produced session fixtures (v1, v2, v3)
  *Produces Pi_TS session fixtures at `tests/fixtures/pi-ts-output/sessions/` (v1, v2, v3 samples). Adds conformance tests that load each fixture, re-serialize, and check structural equality. Verifies v1/v2 migration produces a v3 file structurally equal to the expected migrated form.*
  _Requirements: 3.11, 3.12, 23.1, 23.2, 23.3_
  _Design: Testing Strategy > Conformance tests_

- [ ] 97. Conformance tests — settings, auth, models, keybindings, and theme fixtures
  *Produces Pi_TS-written `settings.json`, `auth.json`, `models.json`, `keybindings.json`, and theme JSON fixtures. Adds conformance tests that parse each, re-serialize, and verify structural equality including unknown-key preservation.*
  _Requirements: 5.15, 6.14, 15.8, 18.7, 23.6, 23.7_
  _Design: Testing Strategy > Conformance tests_

- [ ] 98. Conformance tests — export-html, bash outputs, CLI events, and RPC traces
  *Produces Pi_TS-generated export-html, bash-output, JSON-mode event stream, and RPC-mode trace fixtures. Adds conformance tests verifying Pie_Cpp produces structurally equivalent output for the same inputs.*
  _Requirements: 2.6, 2.7, 12.8, 19.3_
  _Design: Testing Strategy > Conformance tests_

- [ ] 99. Coverage collection and reporting
  *Configures CMake to build with `--coverage` (gcov) in the `Coverage` preset. Adds a CI step that runs `gcovr` and produces a Markdown summary + `coverage.json`. Verifies ≥80% line coverage for `pie::core`, `pie::wire`, `pie::session`, `pie::settings`, `pie::queue`, `pie::tools`.*
  _Requirements: 1.3_
  _Design: Testing Strategy > Coverage targets_

- [ ] 100. `--verbose` startup-banner snapshot tests
  *Adds snapshot tests that run `pie --verbose --no-session --mode json` against a known fixture directory and compare the stderr startup banner against a golden file. Covers all enumerated fields: extensions, skills, prompts, themes, context files, model registry, effective settings layers.*
  _Requirements: 24.2_
  _Design: Error Handling > --verbose output_

- [ ] 101. Full CI matrix — all configurations green
  *Confirms all three Ubuntu 24.04 gcc CI jobs (x86_64 debug, x86_64 release, aarch64 release) run the full unit + property + integration + conformance test suites and all pass.*
  _Requirements: 1.3, 1.5, 1.10_
  _Design: Build & Distribution > CI matrix_

- [ ] 102. README and documentation scaffolding
  *Writes `pie-coding-agent/README.md` (build instructions, `pie --version` quick-start, link to docs), `pie-coding-agent/docs/` with pages for: CLI reference, session format, settings, providers, extensions (JS + native ABI), skills, prompt templates, themes, packages, SDK usage, migration from Pi_TS, security notes.*
  _Requirements: 1.3, 1.10_
  _Design: Open Questions / Trade-offs (documentation)_

- [ ] 103. Phase 7 final integration check — all tests green, no regressions
  *Runs the complete test suite (unit + property + integration + conformance) on Ubuntu 24.04 gcc release. Verifies `pie --version` exits 0, `pie --help` exits 0, `pie --mode json` emits a valid Session_Header as the first line, and `pie --export <v3-fixture>` produces a self-contained HTML file with no external resource references.*
  _Requirements: 1.3, 1.9, 2.6, 19.3_
  _Design: Testing Strategy_


## Task Dependency Graph

```json
{
  "waves": [
    {"wave": 1, "tasks": ["1"]},
    {"wave": 2, "tasks": ["2"]},
    {"wave": 3, "tasks": ["3"]},
    {"wave": 4, "tasks": ["4"]},
    {"wave": 5, "tasks": ["5"]},
    {"wave": 6, "tasks": ["6", "7"]},
    {"wave": 7, "tasks": ["8"]},
    {"wave": 8, "tasks": ["9"]},
    {"wave": 9, "tasks": ["10"]},
    {"wave": 10, "tasks": ["11"]},
    {"wave": 11, "tasks": ["12"]},
    {"wave": 12, "tasks": ["13"]},
    {"wave": 13, "tasks": ["14"]},
    {"wave": 14, "tasks": ["15"]},
    {"wave": 15, "tasks": ["16"]},
    {"wave": 16, "tasks": ["17"]},
    {"wave": 17, "tasks": ["18"]},
    {"wave": 18, "tasks": ["19", "23", "28"]},
    {"wave": 19, "tasks": ["20", "24", "29", "31"]},
    {"wave": 20, "tasks": ["21", "25", "26", "30", "32"]},
    {"wave": 21, "tasks": ["22", "27", "33"]},
    {"wave": 22, "tasks": ["34"]},
    {"wave": 23, "tasks": ["35"]},
    {"wave": 24, "tasks": ["36"]},
    {"wave": 25, "tasks": ["37", "42", "43", "47", "53", "54", "58", "59", "60"]},
    {"wave": 26, "tasks": ["38", "44", "48", "55"]},
    {"wave": 27, "tasks": ["39", "41", "45", "49", "56"]},
    {"wave": 28, "tasks": ["40", "46", "50", "57"]},
    {"wave": 29, "tasks": ["51"]},
    {"wave": 30, "tasks": ["52"]},
    {"wave": 31, "tasks": ["61"]},
    {"wave": 32, "tasks": ["62", "65", "66", "67"]},
    {"wave": 33, "tasks": ["63", "68"]},
    {"wave": 34, "tasks": ["64"]},
    {"wave": 35, "tasks": ["69"]},
    {"wave": 36, "tasks": ["70", "72", "74", "75", "76", "77", "78"]},
    {"wave": 37, "tasks": ["71", "73"]},
    {"wave": 38, "tasks": ["79"]},
    {"wave": 39, "tasks": ["80", "81", "82", "83", "84", "85", "86", "87", "88", "89", "90", "91", "92", "93", "94", "95", "96", "97", "98", "100", "102"]},
    {"wave": 40, "tasks": ["99"]},
    {"wave": 41, "tasks": ["101"]},
    {"wave": 42, "tasks": ["103"]}
  ]
}
```

## Notes

- All tasks must leave the build green (compilable + existing tests passing) before the next task begins.
- Tasks 80–89 each require a minimum of 100 rapidcheck iterations per property; the tag format `Feature: cpp-coding-agent, Property {N}: {text}` must appear as a comment in each test file.
- Conformance fixtures (tasks 96–98) are produced by running Pi_TS at the spec's reference commit against scripted inputs and checking the output into `tests/fixtures/pi-ts-output/`. These fixtures are the ground truth for wire-format compatibility.
- The `coding-agent/` directory must never be modified by any task. Tasks 4 and 31 read from it at build time only (theme JSON copy, model list codegen); all other tasks are independent of it.
- Tasks 54–57 (extension host) require `node >= 22.19.0` to be present on the CI runner for JS-extension integration tests. The binary itself does not require Node at runtime for `pie --version` (Req 1.9).
- The Ubuntu gcc 13+ enforcement (task 2) runs at CMake configure time and halts before any compilation if the wrong compiler or platform is detected. Only Ubuntu 24.04 with gcc is supported; macOS and Windows are out of scope.
