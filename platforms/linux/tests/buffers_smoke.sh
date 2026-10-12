#!/usr/bin/env bash
# buffers_smoke.sh — EP-026 S5 (SP-179 / T-0606): the copy buffers 1–9 on Linux through REAL project windows and the real key
# dispatch — Ctrl+N copy, Ctrl+Shift+N paste, Alt+N cut, the one Buffers palette following the front project. Text is
# read back FROM DISK, slots through scrivi_buffers_*. Offscreen.
#
# Usage: buffers_smoke.sh <path-to-harness-binary>
set -euo pipefail

BIN="${1:?usage: buffers_smoke.sh <harness-binary>}"

WORKDIR="$(mktemp -d)"
trap 'rm -rf "$WORKDIR"' EXIT
export XDG_DATA_HOME="$WORKDIR/xdg"
export QT_QPA_PLATFORM=offscreen

echo "== Copy buffers (real keys, disk-verified, the palette and the front project) =="
OUT="$("$BIN" "$WORKDIR/projects")" || { echo "$OUT"; exit 1; }
echo "$OUT"
if ! echo "$OUT" | grep -qx 'buffers-ok'; then
    echo "FAIL: harness did not report buffers-ok"
    exit 1
fi
echo "PASS: the copy buffers work on Linux through the real dispatch, and the palette follows the front project."
