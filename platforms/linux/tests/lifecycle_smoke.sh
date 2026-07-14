#!/usr/bin/env bash
# lifecycle_smoke.sh — SP-060 / T-0233 full-loop check.
#
# Runs the scrivi_linux_lifecycle_smoke harness twice against one temp
# $XDG_DATA_HOME (a simulated app restart), proving the
# create → open → close → reopen-recent loop and that it survives a restart.
#
#   pass 1: create a project under a temp dir, open it (ready), record it in
#           recents, "close", reopen from recents. Prints the projectID.
#   pass 2: fresh process — the recents entry persists and reopens ready with the
#           same projectID.
#
# Usage: lifecycle_smoke.sh <path-to-harness-binary>
set -euo pipefail

BIN="${1:?usage: lifecycle_smoke.sh <harness-binary>}"

WORKDIR="$(mktemp -d)"
trap 'rm -rf "$WORKDIR"' EXIT
export XDG_DATA_HOME="$WORKDIR/xdg"
PROJECT_DIR="$WORKDIR/projects/lifecycle-project.scrivi"

echo "== pass 1 (create → open → close → reopen-recent) =="
PROJECT_ID="$("$BIN" 1 "$PROJECT_DIR")"
echo "projectID (pass 1): $PROJECT_ID"

# The .scrivi package must exist on disk.
if [ ! -f "$PROJECT_DIR/project.json" ]; then
    echo "FAIL: $PROJECT_DIR/project.json was not created"
    exit 1
fi

echo "== pass 2 (restart: recents persists → reopen ready) =="
"$BIN" 2 "$PROJECT_ID"

echo "PASS: full create→open→close→reopen loop works and persists across restart."
