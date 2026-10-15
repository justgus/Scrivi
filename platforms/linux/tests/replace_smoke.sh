#!/usr/bin/env bash
# replace_smoke.sh — SP-181 (EP-048 S4b, T-0608): Replace and Replace All over the presented text on Linux, through a REAL
# EditorShell — Replace types over the current match, Replace All runs off the GUI thread as ONE undo step and writes only
# the scenes holding a match. Read back FROM DISK. Offscreen.
#
# Usage: replace_smoke.sh <path-to-harness-binary>
set -euo pipefail

BIN="${1:?usage: replace_smoke.sh <harness-binary>}"

WORKDIR="$(mktemp -d)"
trap 'rm -rf "$WORKDIR"' EXIT
export XDG_DATA_HOME="$WORKDIR/xdg"
export QT_QPA_PLATFORM=offscreen

echo "== Replace / Replace All (the find bar, disk-verified, one undo step, only matched scenes written) =="
OUT="$("$BIN" "$WORKDIR/projects/replace.scrivi")" || { echo "$OUT"; exit 1; }
echo "$OUT"
if ! echo "$OUT" | grep -qx 'replace-ok'; then
    echo "FAIL: harness did not report replace-ok"
    exit 1
fi
echo "PASS: Replace and Replace All work on Linux."
