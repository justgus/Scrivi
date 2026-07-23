#!/usr/bin/env bash
# story_structure_smoke.sh — EP-025 / SP-081 (T-0332, AC4) story-structure persistence
# check.
#
# Runs the scrivi_linux_story_structure_smoke harness against a temp $XDG_DATA_HOME + a
# temp project dir. Through the bridge (the layer EditorShell's structure/assignment
# slots call): sets a built-in structure (three-act) + asserts on reopen it persisted;
# assigns a scene to a band + asserts the bandID persisted; re-proportions the bands
# (update_band_layout) + asserts the new proportions persisted; removes the structure +
# asserts the scene's band assignment is PRESERVED (the AC4 contract).
#
# Uses the Qt "offscreen" platform (a QCoreApplication is created; no window/display).
#
# Usage: story_structure_smoke.sh <path-to-harness-binary>
set -euo pipefail

BIN="${1:?usage: story_structure_smoke.sh <harness-binary>}"

WORKDIR="$(mktemp -d)"
trap 'rm -rf "$WORKDIR"' EXIT
export XDG_DATA_HOME="$WORKDIR/xdg"
export QT_QPA_PLATFORM=offscreen
PROJECT_DIR="$WORKDIR/projects/story-structure.scrivi"

echo "== story structure: set + assign + re-proportion + remove(keeps assignment) =="
PROJECT_ID="$("$BIN" "$PROJECT_DIR")"
echo "projectID: $PROJECT_ID"

if [ ! -f "$PROJECT_DIR/project.json" ]; then
    echo "FAIL: $PROJECT_DIR/project.json was not created"
    exit 1
fi

echo "PASS: story-structure layout + assignment persisted; remove kept the assignment."
