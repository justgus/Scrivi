#!/usr/bin/env bash
# scene_reorder_smoke.sh — EP-023 / SP-067 (T-0258…T-0263, AC4 + I-0063/I-0064)
# scene/chapter reorder + chapter-split + renumber check.
#
# Runs the scrivi_linux_scene_reorder_smoke harness against a temp $XDG_DATA_HOME +
# a temp project dir. The harness builds a 5-scene / 2-chapter fixture, then:
#   • splices scenes within and across chapters via SceneDocument::moveScene (asserting
#     the offset map stays well-formed, bodies preserved, chapter-first re-promotion);
#   • persists a cross-chapter reorder via ScriviBridge::reorderScene and asserts the new
#     manuscript order on reopen;
#   • replays the I-0064 chapter-split orchestration at the bridge level (create_chapter →
#     reorder_chapter → reorder_scene followers → save head/tail) for the end-of-scene and
#     mid-scene cases, asserting the on-disk order + head/tail;
#   • renumbers created "Chapter N" chapters (I-0063) and asserts the corrected ordinals.
#
# Uses the Qt "offscreen" platform (SceneDocument needs a QApplication for
# QPlainTextDocumentLayout; no window is shown).
#
# Usage: scene_reorder_smoke.sh <path-to-harness-binary>
set -euo pipefail

BIN="${1:?usage: scene_reorder_smoke.sh <harness-binary>}"

WORKDIR="$(mktemp -d)"
trap 'rm -rf "$WORKDIR"' EXIT
export XDG_DATA_HOME="$WORKDIR/xdg"
export QT_QPA_PLATFORM=offscreen
PROJECT_DIR="$WORKDIR/projects/scene-reorder.scrivi"

echo "== scene drag-reorder + chapter split (I-0064) + renumber (I-0063) =="
PROJECT_ID="$("$BIN" "$PROJECT_DIR")"
echo "projectID: $PROJECT_ID"

if [ ! -f "$PROJECT_DIR/project.json" ]; then
    echo "FAIL: $PROJECT_DIR/project.json was not created"
    exit 1
fi

echo "PASS: reorder spliced the map, persisted the new order, split the chapter, and renumbered."
