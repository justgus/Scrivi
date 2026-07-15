#!/usr/bin/env bash
# scene_rename_smoke.sh — EP-023 / SP-066 (T-0254/T-0255/T-0257, AC3) rename-scene +
# rename-chapter check.
#
# Runs the scrivi_linux_scene_rename_smoke harness against a temp $XDG_DATA_HOME +
# a temp project dir. The harness builds a 2-chapter fixture, renames a scene and a
# chapter via ScriviBridge::renameScene / renameChapter (asserting the sidecar `title`
# round-trips on reopen, incl. blank→cleared), and applies the label/heading update to
# a SceneDocument (setSceneTitle leaves the document text unchanged; setChapterTitle
# rewrites the live heading in place with the offset map staying well-formed).
#
# Uses the Qt "offscreen" platform (SceneDocument needs a QApplication for
# QPlainTextDocumentLayout; no window is shown).
#
# Usage: scene_rename_smoke.sh <path-to-harness-binary>
set -euo pipefail

BIN="${1:?usage: scene_rename_smoke.sh <harness-binary>}"

WORKDIR="$(mktemp -d)"
trap 'rm -rf "$WORKDIR"' EXIT
export XDG_DATA_HOME="$WORKDIR/xdg"
export QT_QPA_PLATFORM=offscreen
PROJECT_DIR="$WORKDIR/projects/scene-rename.scrivi"

echo "== navigator rename-scene + rename-chapter (context menu → edit) =="
PROJECT_ID="$("$BIN" "$PROJECT_DIR")"
echo "projectID: $PROJECT_ID"

if [ ! -f "$PROJECT_DIR/project.json" ]; then
    echo "FAIL: $PROJECT_DIR/project.json was not created"
    exit 1
fi

echo "PASS: rename-scene + rename-chapter persisted; live label + heading updated."
