#!/usr/bin/env bash
# scene_create_smoke.sh — SP-062 / T-0240 + T-0241 (EP-022 AC3) in-editor
# create-scene + create-chapter check.
#
# Runs the scrivi_linux_scene_create_smoke harness against a temp $XDG_DATA_HOME +
# a temp project dir. The harness creates a project, creates a scene after the
# active one (ScriviBridge::createScene) and a chapter at the end
# (ScriviBridge::createChapter), splices each into a SceneDocument via
# insertSceneAfter, and asserts both the map splices and the reopen persistence.
#
# Uses the Qt "offscreen" platform (SceneDocument needs a QApplication for
# QPlainTextDocumentLayout; no window is shown).
#
# Usage: scene_create_smoke.sh <path-to-harness-binary>
set -euo pipefail

BIN="${1:?usage: scene_create_smoke.sh <harness-binary>}"

WORKDIR="$(mktemp -d)"
trap 'rm -rf "$WORKDIR"' EXIT
export XDG_DATA_HOME="$WORKDIR/xdg"
export QT_QPA_PLATFORM=offscreen
PROJECT_DIR="$WORKDIR/projects/scene-create.scrivi"

echo "== in-editor create-scene + create-chapter (Ctrl+Return / Ctrl+Shift+Return) =="
PROJECT_ID="$("$BIN" "$PROJECT_DIR")"
echo "projectID: $PROJECT_ID"

if [ ! -f "$PROJECT_DIR/project.json" ]; then
    echo "FAIL: $PROJECT_DIR/project.json was not created"
    exit 1
fi

echo "PASS: new scene + new chapter persisted and spliced into the offset map."
