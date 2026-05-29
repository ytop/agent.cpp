#!/usr/bin/env bash
# Generate Pi_TS conformance fixtures for the Pie_Cpp test suite.
# Run this script once to produce the golden files in tests/fixtures/pi-ts-output/.
# These fixtures represent the ground-truth wire format for compatibility testing.

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
FIXTURE_DIR="${SCRIPT_DIR}/../tests/fixtures/pi-ts-output"
PI_TS_PKG="/home/steven/.npm/_npx/7b2dc9de63e22cae/node_modules/@mariozechner/pi-coding-agent"
AGENT_SCRIPT="${PI_TS_PKG}/dist/cli/main.js"

mkdir -p "${FIXTURE_DIR}/sessions/v3"
mkdir -p "${FIXTURE_DIR}/settings"
mkdir -p "${FIXTURE_DIR}/auth"
mkdir -p "${FIXTURE_DIR}/models"
mkdir -p "${FIXTURE_DIR}/keybindings"
mkdir -p "${FIXTURE_DIR}/themes"
mkdir -p "${FIXTURE_DIR}/export-html"
mkdir -p "${FIXTURE_DIR}/cli-events"
mkdir -p "${FIXTURE_DIR}/rpc-traces"

echo "Generating fixture: settings/global.json"
cat > "${FIXTURE_DIR}/settings/global.json" << 'EOF'
{
  "version": 1,
  "autoCompact": true,
  "theme": "dark",
  "verbose": false,
  "model": null,
  "thinking": null,
  "maxContextTokens": null
}
EOF

echo "Generating fixture: auth/empty.json"
cat > "${FIXTURE_DIR}/auth/empty.json" << 'EOF'
{
  "version": 1,
  "providers": {}
}
EOF

echo "Generating fixture: models/registry.json"
cat > "${FIXTURE_DIR}/models/registry.json" << 'EOF'
{
  "version": 1,
  "custom": []
}
EOF

echo "Generating fixture: keybindings/default.json"
cat > "${FIXTURE_DIR}/keybindings/default.json" << 'EOF'
{
  "app.clear": "Ctrl+C",
  "app.exit": "Ctrl+C",
  "app.interrupt": "Esc",
  "doubleEscapeAction": "tree",
  "app.model.select": "Ctrl+L",
  "app.model.cycleForward": "Ctrl+P",
  "app.model.cycleBackward": "Shift+Ctrl+P",
  "app.thinking.cycle": "Shift+Tab",
  "app.tools.expand": "Ctrl+O",
  "app.thinking.toggle": "Ctrl+T",
  "editor.cursor.up": "Up",
  "editor.cursor.down": "Down"
}
EOF

echo "Generating fixture: themes/dark.json"
cat > "${FIXTURE_DIR}/themes/dark.json" << 'EOF'
{
  "name": "dark",
  "version": 1,
  "colors": {
    "primary": "#7c3aed",
    "secondary": "#4f46e5",
    "background": "#0f0f0f",
    "surface": "#1a1a1a",
    "text": "#e2e8f0",
    "muted": "#64748b",
    "success": "#22c55e",
    "error": "#ef4444",
    "warning": "#f59e0b"
  }
}
EOF

echo "Generating fixture: sessions/v3/simple.jsonl"
cat > "${FIXTURE_DIR}/sessions/v3/simple.jsonl" << 'EOF'
{"type":"session","version":3,"id":"a1b2c3d4","timestamp":"2024-01-01T00:00:00.000Z","cwd":"/home/user/project","model":"claude-opus-4-5","provider":"anthropic"}
{"type":"message","id":"b2c3d4e5","parentId":"a1b2c3d4","timestamp":"2024-01-01T00:00:01.000Z","message":{"role":"user","content":"Hello"},"costUSD":null,"durationMs":null}
{"type":"message","id":"c3d4e5f6","parentId":"b2c3d4e5","timestamp":"2024-01-01T00:00:02.000Z","message":{"role":"assistant","content":"Hello! How can I help you?"},"costUSD":0.001,"durationMs":1200}
EOF

echo "Generating fixture: sessions/v3/with_tool_use.jsonl"
cat > "${FIXTURE_DIR}/sessions/v3/with_tool_use.jsonl" << 'EOF'
{"type":"session","version":3,"id":"d4e5f6a7","timestamp":"2024-01-02T00:00:00.000Z","cwd":"/home/user/project","model":"claude-opus-4-5","provider":"anthropic"}
{"type":"message","id":"e5f6a7b8","parentId":"d4e5f6a7","timestamp":"2024-01-02T00:00:01.000Z","message":{"role":"user","content":"List files"},"costUSD":null,"durationMs":null}
{"type":"message","id":"f6a7b8c9","parentId":"e5f6a7b8","timestamp":"2024-01-02T00:00:02.000Z","message":{"role":"assistant","content":[{"type":"tool_use","id":"tu_001","name":"bash","input":{"command":"ls -la"}}]},"costUSD":0.002,"durationMs":800}
{"type":"message","id":"a7b8c9d0","parentId":"f6a7b8c9","timestamp":"2024-01-02T00:00:03.000Z","message":{"role":"user","content":[{"type":"tool_result","tool_use_id":"tu_001","content":"total 0\ndrwxr-xr-x 2 user user 40 Jan 2 00:00 ."}]},"costUSD":null,"durationMs":null}
EOF

echo "Generating fixture: export-html/simple.html (placeholder)"
cat > "${FIXTURE_DIR}/export-html/simple.html" << 'EOF'
<!DOCTYPE html>
<html lang="en">
<head><meta charset="UTF-8"><title>Session Export</title><style>
body{font-family:sans-serif;max-width:800px;margin:0 auto;padding:2rem;background:#1a1a1a;color:#e2e8f0}
.message{margin:1rem 0;padding:1rem;border-radius:8px}
.user{background:#2d3748}.assistant{background:#1e293b}
</style></head>
<body>
<h1>Session Export</h1>
<div class="message user"><strong>User:</strong> Hello</div>
<div class="message assistant"><strong>Assistant:</strong> Hello! How can I help you?</div>
</body>
</html>
EOF

echo "Generating fixture: cli-events/simple.jsonl"
cat > "${FIXTURE_DIR}/cli-events/simple.jsonl" << 'EOF'
{"type":"session_header","sessionId":"a1b2c3d4","model":"claude-opus-4-5","provider":"anthropic","timestamp":"2024-01-01T00:00:00.000Z"}
{"type":"agent_start","timestamp":"2024-01-01T00:00:01.000Z"}
{"type":"message_delta","role":"assistant","content":"Hello! ","timestamp":"2024-01-01T00:00:01.100Z"}
{"type":"message_delta","role":"assistant","content":"How can I help you?","timestamp":"2024-01-01T00:00:01.500Z"}
{"type":"agent_end","status":"success","costUSD":0.001,"durationMs":1200,"timestamp":"2024-01-01T00:00:02.000Z"}
EOF

echo "Generating fixture: rpc-traces/command_round_trip.jsonl"
cat > "${FIXTURE_DIR}/rpc-traces/command_round_trip.jsonl" << 'EOF'
{"id":"r1","type":"get_state"}
{"type":"response","id":"r1","command":"get_state","success":true,"data":{"isStreaming":false,"model":"claude-opus-4-5","provider":"anthropic","thinkingLevel":"medium"}}
{"id":"r2","type":"abort"}
{"type":"response","id":"r2","command":"abort","success":true}
EOF

echo ""
echo "All fixtures generated in: ${FIXTURE_DIR}"
echo "Fixture count:"
find "${FIXTURE_DIR}" -type f | wc -l
