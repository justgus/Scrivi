#!/usr/bin/env bash
# authorship_smoke.sh — EP-058 S3 ([SP-211], T-0672): authorship on Linux — the bridge, the questions on open, the menus,
# through REAL project windows; the edit's author read back FROM DISK. Offscreen.
#
# Usage: authorship_smoke.sh <path-to-harness-binary>
set -euo pipefail

BIN="${1:?usage: authorship_smoke.sh <harness-binary>}"

WORKDIR="$(mktemp -d)"
trap 'rm -rf "$WORKDIR"' EXIT
export XDG_DATA_HOME="$WORKDIR/xdg"
export QT_QPA_PLATFORM=offscreen

echo "== Authorship (pen names, Authors, the questions on open) =="
OUT="$("$BIN" "$WORKDIR/projects")" || { echo "$OUT"; exit 1; }
echo "$OUT"
if ! echo "$OUT" | grep -qx 'authorship-ok'; then
    echo "FAIL: harness did not report authorship-ok"
    exit 1
fi
echo "PASS: authorship works on Linux."
