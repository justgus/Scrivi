#!/usr/bin/env bash
# format_smoke.sh — EP-048 S3b (SP-175, T-0602): the Format commands and balanced edits on Linux, offscreen.
#
# Usage: format_smoke.sh <path-to-harness-binary>
set -euo pipefail

BIN="${1:?usage: format_smoke.sh <harness-binary>}"
export QT_QPA_PLATFORM=offscreen

echo "== Format menu + balanced edits (real shortcuts, the pending pair, cut/copy/paste, across scenes) =="
OUT="$("$BIN")"
if ! echo "$OUT" | grep -qx 'format-ok'; then
    echo "FAIL: harness did not report format-ok"
    exit 1
fi
echo "PASS: the Format commands and balanced edits work through the real dispatch, on the core's logic."
