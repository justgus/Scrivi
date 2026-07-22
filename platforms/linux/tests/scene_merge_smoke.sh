#!/usr/bin/env bash
# scene_merge_smoke.sh — EP-028 / SP-076 (T-0306, AC5/AC6/AC7 Linux) scene + chapter
# merge check.
#
# Runs the scrivi_linux_scene_merge_smoke harness against a temp $XDG_DATA_HOME + a temp
# project dir. The harness builds a 5-scene / 2-chapter fixture, then:
#   • merges a scene into the previous scene of the same chapter via
#     ScriviBridge::mergeScene and asserts on reopen that the absorbed scene's files are
#     gone and the survivor holds both bodies joined by a blank line;
#   • merges a whole chapter into the previous chapter via ScriviBridge::mergeChapter and
#     asserts on reopen that every scene relocated (nothing lost — the I-0083 fix), the
#     emptied chapter is removed, and bodies are intact.
#
# Uses the Qt "offscreen" platform (a QApplication is created; no window is shown).
#
# Usage: scene_merge_smoke.sh <path-to-harness-binary>
set -euo pipefail

BIN="${1:?usage: scene_merge_smoke.sh <harness-binary>}"

WORKDIR="$(mktemp -d)"
trap 'rm -rf "$WORKDIR"' EXIT
export XDG_DATA_HOME="$WORKDIR/xdg"
export QT_QPA_PLATFORM=offscreen
PROJECT_DIR="$WORKDIR/projects/scene-merge.scrivi"

echo "== scene merge + chapter merge (I-0083 nothing-lost) =="
PROJECT_ID="$("$BIN" "$PROJECT_DIR")"
echo "projectID: $PROJECT_ID"

if [ ! -f "$PROJECT_DIR/project.json" ]; then
    echo "FAIL: $PROJECT_DIR/project.json was not created"
    exit 1
fi

echo "PASS: scene merge joined bodies + chapter merge relocated all scenes (nothing lost)."
