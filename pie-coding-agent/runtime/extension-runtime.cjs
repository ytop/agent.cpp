#!/usr/bin/env node
// extension-runtime.cjs — JSON-RPC bridge for JS extensions
// This is a placeholder. The real implementation comes in Task 55.
"use strict";
const path = process.argv[2];
if (!path) { process.stderr.write("usage: extension-runtime.cjs <extension-path>\n"); process.exit(1); }
process.stderr.write(`[pie] extension-runtime: loading ${path} (stub)\n`);
