#!/usr/bin/env bash
# branch_smoke.sh — EP-026 S3 (SP-204 / T-0662): branching undo on Linux through a REAL EditorShell and the real key
# dispatch — the chooser on Ctrl+Shift+Z (never on an undo), the branch mark, a branch written before the window opened,
# the History section, Project Settings ▸ Undo History, Project ▸ Purge Stale History Branches…. Text is read back FROM
# DISK. Offscreen.
#
# Usage: branch_smoke.sh <path-to-harness-binary>
set -euo pipefail

BIN="${1:?usage: branch_smoke.sh <harness-binary>}"

WORKDIR="$(mktemp -d)"
trap 'rm -rf "$WORKDIR"' EXIT
export XDG_DATA_HOME="$WORKDIR/xdg"
export XDG_CONFIG_HOME="$WORKDIR/config"
export QT_QPA_PLATFORM=offscreen

echo "== Branching undo (chooser, mark, History, settings, purge — real keys, disk-verified) =="
OUT="$("$BIN" "$WORKDIR/projects/branch.scrivi")" || { echo "$OUT"; exit 1; }
echo "$OUT"
if ! echo "$OUT" | grep -qx 'branch-ok'; then
    echo "FAIL: harness did not report branch-ok"
    exit 1
fi
echo "PASS: branching undo works on Linux through the real dispatch."
