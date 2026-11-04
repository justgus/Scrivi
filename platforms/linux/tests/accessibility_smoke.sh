#!/usr/bin/env bash
# accessibility_smoke.sh — EP-050 S4 (SP-203, T-0659): what Orca is given for the Linux manuscript, offscreen.
#
# Usage: accessibility_smoke.sh <path-to-harness-binary>
set -euo pipefail

BIN="${1:?usage: accessibility_smoke.sh <harness-binary>}"
export QT_QPA_PLATFORM=offscreen

echo "== Accessibility: the page at rest, one map, the caret both ways, translated events (~1.8 MB measured) =="
OUT="$("$BIN")"
echo "$OUT" | grep '^measure' || true
if ! echo "$OUT" | grep -qx 'accessibility-ok'; then
    echo "FAIL: harness did not report accessibility-ok"
    exit 1
fi
echo "PASS: Orca is given the page the writer reads."
