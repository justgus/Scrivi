#!/usr/bin/env bash
# timeline_story_time_smoke.sh — EP-025 / SP-080 (T-0328, AC3) story-time placement
# check.
#
# Runs the scrivi_linux_timeline_story_time_smoke harness against a temp $XDG_DATA_HOME
# + a temp project dir. The harness builds a 3-scene fixture, then through the bridge
# (the layer EditorShell::showTimeDeltaPicker calls):
#   • places a scene MANUALLY 2 hours after the previous scene's end
#     (setSceneStoryTime source="manual") + re-persists the chain, and asserts on reopen
#     that the manual gap persisted and later default scenes stayed on the chain;
#   • RESETS the scene back to default (source="default", gap 0) and asserts it returns
#     to the gap chain on reopen.
#
# Uses the Qt "offscreen" platform (a QCoreApplication is created; no window/display).
#
# Usage: timeline_story_time_smoke.sh <path-to-harness-binary>
set -euo pipefail

BIN="${1:?usage: timeline_story_time_smoke.sh <harness-binary>}"

WORKDIR="$(mktemp -d)"
trap 'rm -rf "$WORKDIR"' EXIT
export XDG_DATA_HOME="$WORKDIR/xdg"
export QT_QPA_PLATFORM=offscreen
PROJECT_DIR="$WORKDIR/projects/timeline-story-time.scrivi"

echo "== timeline story-time: manual placement + chain propagation + reset =="
PROJECT_ID="$("$BIN" "$PROJECT_DIR")"
echo "projectID: $PROJECT_ID"

if [ ! -f "$PROJECT_DIR/project.json" ]; then
    echo "FAIL: $PROJECT_DIR/project.json was not created"
    exit 1
fi

echo "PASS: setSceneStoryTime persisted manual/default placement + chain across reopen."
