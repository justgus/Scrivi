#!/usr/bin/env bash
# chapter_reorder_smoke.sh — EP-023 / SP-073 (T-0294…T-0296, AC5) chapter
# drag-reorder check.
#
# Runs the scrivi_linux_chapter_reorder_smoke harness against a temp $XDG_DATA_HOME +
# a temp project dir. The harness builds a 4-scene / 3-chapter fixture, then:
#   • splices whole chapters (heading + member scenes as a block) between chapters, to
#     the manuscript front, and to the end via SceneDocument::moveChapter (asserting the
#     offset map stays well-formed, bodies preserved, member order kept, no-op landings
#     refused);
#   • persists chapter moves via ScriviBridge::reorderChapter and asserts the new
#     manuscript order + every scene body on reopen;
#   • proves the SP-073 stale-path refresh (I-0074/I-0079 class): the reorder reslugs the
#     chapter's folder, the envelope reports the new metadataPath, refreshChapterPaths
#     re-bases the captured paths, and a save through the refreshed paths lands.
#
# Uses the Qt "offscreen" platform (SceneDocument needs a QApplication for
# QPlainTextDocumentLayout; no window is shown).
#
# Usage: chapter_reorder_smoke.sh <path-to-harness-binary>
set -euo pipefail

BIN="${1:?usage: chapter_reorder_smoke.sh <harness-binary>}"

WORKDIR="$(mktemp -d)"
trap 'rm -rf "$WORKDIR"' EXIT
export XDG_DATA_HOME="$WORKDIR/xdg"
export QT_QPA_PLATFORM=offscreen
PROJECT_DIR="$WORKDIR/projects/chapter-reorder.scrivi"

echo "== chapter drag-reorder (AC5): block splice + persistence + path refresh =="
PROJECT_ID="$("$BIN" "$PROJECT_DIR")"
echo "projectID: $PROJECT_ID"

if [ ! -f "$PROJECT_DIR/project.json" ]; then
    echo "FAIL: $PROJECT_DIR/project.json was not created"
    exit 1
fi

echo "PASS: chapter moved as a container, persisted, and saves land at the reslugged paths."
