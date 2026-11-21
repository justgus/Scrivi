#!/usr/bin/env bash
# navigator_search_smoke.sh — [SP-230] (EP-059 E): untitled-scene names, Go To, and the Navigator's search, through REAL
# project windows. Offscreen.
#
# Usage: navigator_search_smoke.sh <path-to-harness-binary>
set -euo pipefail

BIN="${1:?usage: navigator_search_smoke.sh <harness-binary>}"

WORKDIR="$(mktemp -d)"
trap 'rm -rf "$WORKDIR"' EXIT
export XDG_DATA_HOME="$WORKDIR/xdg"
export QT_QPA_PLATFORM=offscreen

echo "== Scene names, Go To, Navigator search =="
OUT="$("$BIN" "$WORKDIR/projects")" || { echo "$OUT"; exit 1; }
echo "$OUT"
if ! echo "$OUT" | grep -qx 'navigator-search-ok'; then
    echo "FAIL: harness did not report navigator-search-ok"
    exit 1
fi
echo "PASS: untitled scenes are named, Go To moves, and the Navigator searches as on the Mac."
