#!/usr/bin/env bash
# undo_smoke.sh — EP-026 S2 (SP-178 / T-0605): undo / redo on Linux through a REAL EditorShell and the real key
# dispatch — one step per sentence, Ctrl+Z / Ctrl+Shift+Z (⛔ not Ctrl+Y), a paste one step, a cross-scene delete one
# step, persistence + the session warning, a split's barrier and its reload keeping the same history. Text is read
# back FROM DISK. Offscreen.
#
# Usage: undo_smoke.sh <path-to-harness-binary>
set -euo pipefail

BIN="${1:?usage: undo_smoke.sh <harness-binary>}"

WORKDIR="$(mktemp -d)"
trap 'rm -rf "$WORKDIR"' EXIT
export XDG_DATA_HOME="$WORKDIR/xdg"
export QT_QPA_PLATFORM=offscreen

echo "== Undo / redo (real keys, disk-verified, persistence, barriers, grouped edits) =="
OUT="$("$BIN" "$WORKDIR/projects/undo.scrivi")" || { echo "$OUT"; exit 1; }
echo "$OUT"
if ! echo "$OUT" | grep -qx 'undo-ok'; then
    echo "FAIL: harness did not report undo-ok"
    exit 1
fi
echo "PASS: undo and redo work on Linux through the real dispatch, on the core's capture rules."
