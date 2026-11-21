#!/usr/bin/env bash
# world_warning_smoke.sh — [SP-231] (EP-059 D): create a world, Reconnect Worlds, the world-warning strip, through a REAL
# project window. Offscreen.
#
# Usage: world_warning_smoke.sh <path-to-harness-binary>
set -euo pipefail

BIN="${1:?usage: world_warning_smoke.sh <harness-binary>}"

WORKDIR="$(mktemp -d)"
trap 'rm -rf "$WORKDIR"' EXIT
export XDG_DATA_HOME="$WORKDIR/xdg"
export QT_QPA_PLATFORM=offscreen

echo "== Create a world, Reconnect Worlds, the warning strip =="
OUT="$("$BIN" "$WORKDIR/projects")" || { echo "$OUT"; exit 1; }
echo "$OUT"
if ! echo "$OUT" | grep -qx 'world-warning-ok'; then
    echo "FAIL: harness did not report world-warning-ok"
    exit 1
fi
echo "PASS: worlds are created, reconnected and warned about as on the Mac."
