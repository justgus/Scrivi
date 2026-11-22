#!/usr/bin/env bash
# inspector_stack_smoke.sh — [SP-233] (EP-059 B5, B6, A7): the Scene Inspector card stacks, through a REAL
# project window. Offscreen.
#
# Usage: inspector_stack_smoke.sh <path-to-harness-binary>
set -euo pipefail

BIN="${1:?usage: inspector_stack_smoke.sh <harness-binary>}"

WORKDIR="$(mktemp -d)"
trap 'rm -rf "$WORKDIR"' EXIT
export XDG_DATA_HOME="$WORKDIR/xdg"
export QT_QPA_PLATFORM=offscreen

echo "== The Scene Inspector card stacks =="
OUT="$("$BIN" "$WORKDIR/projects")" || { echo "$OUT"; exit 1; }
echo "$OUT"
if ! echo "$OUT" | grep -qx 'inspector-stack-ok'; then
    echo "FAIL: harness did not report inspector-stack-ok"
    exit 1
fi
echo "PASS: the Writing and Worldbuilding tabs are the Mac's card stacks."
