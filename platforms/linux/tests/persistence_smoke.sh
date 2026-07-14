#!/usr/bin/env bash
# persistence_smoke.sh — SP-059 / T-0228 headless persistence check.
#
# Runs the scrivi_linux_persistence_smoke harness twice against one temp
# $XDG_DATA_HOME (a simulated app restart) and asserts that both the local
# identity (AC4) and a recents entry (AC5) survive the restart.
#
#   pass 1: bootstrap identity + write a recents entry; prints identityID.
#   pass 2: bootstrap again + load recents; asserts the identity matches pass 1
#           and the recents entry is present. Non-zero exit = failure.
#
# Usage: persistence_smoke.sh <path-to-harness-binary>
set -euo pipefail

BIN="${1:?usage: persistence_smoke.sh <harness-binary>}"

WORKDIR="$(mktemp -d)"
trap 'rm -rf "$WORKDIR"' EXIT
export XDG_DATA_HOME="$WORKDIR/xdg"

echo "== pass 1 (bootstrap + write recents) =="
IDENTITY="$("$BIN" 1)"
echo "identityID (pass 1): $IDENTITY"

# Sanity: the app-support root and recents file must now exist on disk.
ROOT="$XDG_DATA_HOME/Scrivi"
if [ ! -f "$ROOT/recents.json" ]; then
    echo "FAIL: recents.json was not written under $ROOT"
    exit 1
fi

echo "== pass 2 (restart: re-bootstrap + reload recents) =="
"$BIN" 2 "$IDENTITY"

echo "PASS: identity + recents persisted across restart."
