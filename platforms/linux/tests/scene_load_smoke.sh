#!/usr/bin/env bash
# scene_load_smoke.sh — SP-061 / T-0237 (EP-022 AC1/AC2) scene-load check.
#
# Runs the scrivi_linux_scene_load_smoke harness against a temp $XDG_DATA_HOME +
# a temp project dir. The harness builds a 3-scene fixture with distinct bodies,
# then replicates the editor's continuous-viewport assembly (open_project +
# open_scene loop + SceneDocument::build) and asserts every body loaded at its
# recorded offset.
#
# Uses the Qt "offscreen" platform so QTextDocument works with no display.
#
# Usage: scene_load_smoke.sh <path-to-harness-binary>
set -euo pipefail

BIN="${1:?usage: scene_load_smoke.sh <harness-binary>}"

WORKDIR="$(mktemp -d)"
trap 'rm -rf "$WORKDIR"' EXIT
export XDG_DATA_HOME="$WORKDIR/xdg"
export QT_QPA_PLATFORM=offscreen
PROJECT_DIR="$WORKDIR/projects/scene-load.scrivi"

echo "== scene-load assembly (3 scenes → one document) =="
PROJECT_ID="$("$BIN" "$PROJECT_DIR")"
echo "projectID: $PROJECT_ID"

# The .scrivi package must exist on disk.
if [ ! -f "$PROJECT_DIR/project.json" ]; then
    echo "FAIL: $PROJECT_DIR/project.json was not created"
    exit 1
fi

echo "PASS: all scene bodies assembled into the continuous viewport at their offsets."
