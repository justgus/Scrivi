#!/usr/bin/env bash
# background_wait_smoke.sh — SP-181 ([I-0304] / [I-0305]): a failed save stays dirty, and closing a window or quitting while
# background writes run waits for them, behind "Please wait — saving updates…". REAL project windows; text read back FROM
# DISK. Offscreen. ⚠️ Run as a NON-ROOT user.
#
# Usage: background_wait_smoke.sh <path-to-harness-binary>
set -euo pipefail

BIN="${1:?usage: background_wait_smoke.sh <harness-binary>}"

WORKDIR="$(mktemp -d)"
trap 'rm -rf "$WORKDIR"' EXIT
export XDG_DATA_HOME="$WORKDIR/xdg"
export QT_QPA_PLATFORM=offscreen

echo "== Background writes (a failed save stays dirty; close and quit wait) =="
OUT="$("$BIN" "$WORKDIR/projects")" || { echo "$OUT"; exit 1; }
echo "$OUT"
if ! echo "$OUT" | grep -qx 'background-wait-ok'; then
    echo "FAIL: harness did not report background-wait-ok"
    exit 1
fi
echo "PASS: a failed save stays dirty, and close and quit wait for background writes."
