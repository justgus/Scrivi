#!/usr/bin/env bash
# indent_smoke.sh — [SP-183] (EP-048 S5b, T-0610): the first-line indent and fixed line heights,
# through REAL project windows and the real dispatch; files read back FROM DISK. Offscreen.
#
# Usage: indent_smoke.sh <path-to-harness-binary>
set -euo pipefail

BIN="${1:?usage: indent_smoke.sh <harness-binary>}"

WORKDIR="$(mktemp -d)"
trap 'rm -rf "$WORKDIR"' EXIT
export XDG_DATA_HOME="$WORKDIR/xdg"
export QT_QPA_PLATFORM=offscreen

echo "== Paragraph indent and line heights =="
OUT="$("$BIN" "$WORKDIR/projects")" || { echo "$OUT"; exit 1; }
echo "$OUT"
if ! echo "$OUT" | grep -qx 'indent-ok'; then
    echo "FAIL: harness did not report indent-ok"
    exit 1
fi
echo "PASS: the paragraph indent and line heights are drawn on Linux, with zero characters in the file."
