#!/usr/bin/env bash
# scene_save_smoke.sh — SP-062 / T-0239 (EP-022 AC3) per-scene save check.
#
# Runs the scrivi_linux_scene_save_smoke harness against a temp $XDG_DATA_HOME + a
# temp project dir. The harness creates a project, saves a scene's body with new
# Markdown via ScriviBridge::saveScene (the editor's auto-save path), then reopens
# and asserts the new bytes are on disk.
#
# Usage: scene_save_smoke.sh <path-to-harness-binary>
set -euo pipefail

BIN="${1:?usage: scene_save_smoke.sh <harness-binary>}"

WORKDIR="$(mktemp -d)"
trap 'rm -rf "$WORKDIR"' EXIT
export XDG_DATA_HOME="$WORKDIR/xdg"
PROJECT_DIR="$WORKDIR/projects/scene-save.scrivi"

echo "== per-scene save (edit → save_scene → reopen) =="
PROJECT_ID="$("$BIN" "$PROJECT_DIR")"
echo "projectID: $PROJECT_ID"

if [ ! -f "$PROJECT_DIR/project.json" ]; then
    echo "FAIL: $PROJECT_DIR/project.json was not created"
    exit 1
fi

echo "PASS: scene body persisted via save_scene and reopened from disk."
