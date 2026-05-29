# CLI Reference

## Synopsis

```
pie [OPTIONS] [MESSAGE...]
pie [SUBCOMMAND]
```

## Positional Arguments

| Argument | Description |
|---|---|
| `MESSAGE...` | The prompt message tokens. Concatenated with spaces. |
| `@path` | Include file content inline (text) or as base64 (image). |

## Mode Flags

At most one mode flag may be specified. Conflicting flags produce an error (exit 2).

| Flag | Mode | Description |
|---|---|---|
| *(none, TTY)* | Interactive | Full TUI |
| *(none, pipe)* | Print | Stream text to stdout |
| `-p`, `--print` | Print | Force print mode |
| `--mode json` | JSON | Emit JSONL event stream to stdout |
| `--mode rpc` | RPC | JSON-RPC on stdin/stdout |
| `--export <file>` | Export | Render session to HTML |

## Model & Provider Options

| Flag | Description |
|---|---|
| `--model <id>` | Model ID (e.g. `claude-opus-4-5`) |
| `--provider <name>` | Provider name (e.g. `anthropic`) |
| `--thinking <level>` | Thinking level: `off`, `low`, `medium`, `high` |
| `--api-key <key>` | Override API key for session |

## Session Options

| Flag | Description |
|---|---|
| `--session <file>` | Open an existing session file |
| `--session-dir <dir>` | Override session directory |
| `--no-session` | Do not persist session to disk |
| `--continue` | Continue most recent session for current directory |

## Resource Options

| Flag | Description |
|---|---|
| `--extension <path>` | Load extension (repeatable) |
| `--skill <path>` | Load skill (repeatable) |
| `--theme <name>` | Apply theme (repeatable) |
| `--no-extensions` | Disable all extensions |
| `--no-skills` | Disable all skills |
| `--no-tools` | Disable all tools |
| `--no-builtin-tools` | Disable built-in tools only |

## System Prompt Options

| Flag | Description |
|---|---|
| `--system-prompt <text>` | Override system prompt |
| `--system-prompt-file <path>` | Load system prompt from file |
| `--append-system-prompt <text>` | Append to system prompt |

## Misc Options

| Flag | Description |
|---|---|
| `--version` | Print version and exit 0 |
| `--help` | Print help and exit 0 |
| `--verbose` | Print startup banner to stderr |
| `--offline` | Disable network requests |
| `--list-models [search]` | List available models |

## Package Subcommands

```bash
pie install npm:<pkg>          # Install from npm
pie install git:<host/user/repo>  # Install from git
pie install ./local/path       # Install local package
pie remove <name>              # Remove installed package
pie update [<name>]            # Update package(s)
pie list                       # List installed packages
pie config                     # Show package configuration
```

## Exit Codes

| Code | Meaning |
|---|---|
| 0 | Success |
| 1 | Provider / auth / I/O error |
| 2 | CLI argument / validation error |
| 3 | Unhandled exception |
| 4 | Package manager error |
