#!/usr/bin/env bash
# toolbar_smoke.sh — [SP-232] (EP-059 F1): the project window's toolbar, through a REAL
# project window. Offscreen.
#
# Usage: toolbar_smoke.sh <path-to-harness-binary>
set -euo pipefail

BIN="${1:?usage: toolbar_smoke.sh <harness-binary>}"

WORKDIR="$(mktemp -d)"
trap 'rm -rf "$WORKDIR"' EXIT
export XDG_DATA_HOME="$WORKDIR/xdg"
export QT_QPA_PLATFORM=offscreen

echo "== The project toolbar =="
OUT="$("$BIN" "$WORKDIR/projects")" || { echo "$OUT"; exit 1; }
echo "$OUT"
if ! echo "$OUT" | grep -qx 'toolbar-ok'; then
    echo "FAIL: harness did not report toolbar-ok"
    exit 1
fi
echo "PASS: every project window has the Mac's toolbar, made of the menus' own actions."
