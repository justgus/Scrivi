#!/usr/bin/env bash
# find_smoke.sh — EP-048 S4 (SP-176, T-0603): Find over the presented text on Linux, offscreen.
#
# Usage: find_smoke.sh <path-to-harness-binary>
set -euo pipefail

BIN="${1:?usage: find_smoke.sh <harness-binary>}"
export QT_QPA_PLATFORM=offscreen

echo "== Find over the presented text (escapes, markers, prefixes; options; real shortcuts; ~1.85 MB measured) =="
OUT="$("$BIN")"
echo "$OUT" | grep '^measure' || true
if ! echo "$OUT" | grep -qx 'find-ok'; then
    echo "FAIL: harness did not report find-ok"
    exit 1
fi
echo "PASS: Find matches what the writer sees, through the real dispatch."
