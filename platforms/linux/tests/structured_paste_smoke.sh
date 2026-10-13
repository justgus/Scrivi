#!/usr/bin/env bash
# structured_paste_smoke.sh — EP-026 S4 (SP-180 / T-0607): structured cut / copy / paste across scenes on Linux through a
# REAL EditorShell and the real key dispatch — Ctrl+C / Ctrl+V rebuild scene and chapter breaks as ONE undo step, the
# copy buffers hold the structure, and a history made on the Mac replays here. Read back FROM DISK. Offscreen.
#
# Usage: structured_paste_smoke.sh <path-to-harness-binary>
set -euo pipefail

BIN="${1:?usage: structured_paste_smoke.sh <harness-binary>}"

WORKDIR="$(mktemp -d)"
trap 'rm -rf "$WORKDIR"' EXIT
export XDG_DATA_HOME="$WORKDIR/xdg"
export QT_QPA_PLATFORM=offscreen

echo "== Structured cut / copy / paste (real keys, disk-verified, undo, buffers, Mac history) =="
OUT="$("$BIN" "$WORKDIR/projects/structured.scrivi")" || { echo "$OUT"; exit 1; }
echo "$OUT"
if ! echo "$OUT" | grep -qx 'structured-paste-ok'; then
    echo "FAIL: harness did not report structured-paste-ok"
    exit 1
fi
echo "PASS: structured cut / copy / paste works on Linux through the real dispatch."
