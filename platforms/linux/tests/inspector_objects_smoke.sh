#!/usr/bin/env bash
# inspector_objects_smoke.sh — [SP-234] (EP-059 A3–A6): the object verbs on a card, through a REAL
# project window. Offscreen.
#
# Usage: inspector_objects_smoke.sh <path-to-harness-binary>
set -euo pipefail

BIN="${1:?usage: inspector_objects_smoke.sh <harness-binary>}"

WORKDIR="$(mktemp -d)"
trap 'rm -rf "$WORKDIR"' EXIT
export XDG_DATA_HOME="$WORKDIR/xdg"
export QT_QPA_PLATFORM=offscreen

echo "== The object verbs on a card =="
OUT="$("$BIN" "$WORKDIR/projects")" || { echo "$OUT"; exit 1; }
echo "$OUT"
if ! echo "$OUT" | grep -qx 'inspector-objects-ok'; then
    echo "FAIL: harness did not report inspector-objects-ok"
    exit 1
fi
echo "PASS: a card adds, creates, renames and removes its objects, as the Mac's."
