#!/usr/bin/env bash
# scene_delete_smoke.sh — EP-023 / SP-065 (T-0250/T-0252/T-0253, AC1/AC2)
# delete-scene + delete-chapter check.
#
# Runs the scrivi_linux_scene_delete_smoke harness against a temp $XDG_DATA_HOME +
# a temp project dir. The harness builds a 4-scene / 2-chapter fixture, splices
# scenes/chapters out of a SceneDocument via removeScene / removeChapter (asserting
# the offset map stays well-formed and survivors keep their bodies), then deletes
# via ScriviBridge::deleteScene / deleteChapter and asserts the removals persist on
# reopen (real .md + index removal).
#
# Uses the Qt "offscreen" platform (SceneDocument needs a QApplication for
# QPlainTextDocumentLayout; no window is shown).
#
# Usage: scene_delete_smoke.sh <path-to-harness-binary>
set -euo pipefail

BIN="${1:?usage: scene_delete_smoke.sh <harness-binary>}"

WORKDIR="$(mktemp -d)"
trap 'rm -rf "$WORKDIR"' EXIT
export XDG_DATA_HOME="$WORKDIR/xdg"
export QT_QPA_PLATFORM=offscreen
PROJECT_DIR="$WORKDIR/projects/scene-delete.scrivi"

echo "== navigator delete-scene + delete-chapter (context menu → confirm) =="
PROJECT_ID="$("$BIN" "$PROJECT_DIR")"
echo "projectID: $PROJECT_ID"

if [ ! -f "$PROJECT_DIR/project.json" ]; then
    echo "FAIL: $PROJECT_DIR/project.json was not created"
    exit 1
fi

echo "PASS: delete-scene + delete-chapter spliced the offset map and persisted."
