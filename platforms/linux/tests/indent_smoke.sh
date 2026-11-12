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
# [I-0359]: name the exit (134 = abort, 139 = segfault, 1 = a failed check) — CI saw only "exit 1".
OUT="$("$BIN" "$WORKDIR/projects")" || { rc=$?; echo "$OUT"; echo "FAIL: harness exited $rc$( ((rc > 128)) && echo " (SIG$(kill -l "$rc"))")"; exit 1; }
echo "$OUT"
if ! echo "$OUT" | grep -qx 'indent-ok'; then
    echo "FAIL: harness did not report indent-ok"
    exit 1
fi
echo "PASS: the paragraph indent and line heights are drawn on Linux, with zero characters in the file."
