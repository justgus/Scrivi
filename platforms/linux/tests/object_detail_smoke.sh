#!/usr/bin/env bash
# object_detail_smoke.sh — [SP-235] (EP-059 B1–B4): the object detail view, through a REAL
# project window. Offscreen.
#
# Usage: object_detail_smoke.sh <path-to-harness-binary>
set -euo pipefail

BIN="${1:?usage: object_detail_smoke.sh <harness-binary>}"

WORKDIR="$(mktemp -d)"
trap 'rm -rf "$WORKDIR"' EXIT
export XDG_DATA_HOME="$WORKDIR/xdg"
export QT_QPA_PLATFORM=offscreen

echo "== The object detail view =="
OUT="$("$BIN" "$WORKDIR/projects")" || { echo "$OUT"; exit 1; }
echo "$OUT"
if ! echo "$OUT" | grep -qx 'object-detail-ok'; then
    echo "FAIL: harness did not report object-detail-ok"
    exit 1
fi
echo "PASS: the detail view edits, saves, images and protects an object, as the Mac's."
