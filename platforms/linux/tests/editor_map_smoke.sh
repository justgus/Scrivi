#!/usr/bin/env bash
# editor_map_smoke.sh — SP-062 / T-0238 editable-viewport map + boundary check.
#
# Runs the scrivi_linux_editor_map_smoke harness (pure SceneDocument logic — no
# project on disk, no display). Asserts boundary classification (editable body vs.
# protected heading/separator) and offset-map maintenance across insert + delete.
#
# Uses the Qt "offscreen" platform so QTextDocument needs no display.
#
# Usage: editor_map_smoke.sh <path-to-harness-binary>
set -euo pipefail

BIN="${1:?usage: editor_map_smoke.sh <harness-binary>}"

export QT_QPA_PLATFORM=offscreen

echo "== editable-viewport offset map + boundary classification =="
OUT="$("$BIN")"
if [ "$OUT" != "editor-map-ok" ]; then
    echo "FAIL: harness did not report editor-map-ok (got: '$OUT')"
    exit 1
fi

echo "PASS: offset map + boundary integrity hold across insert and delete."
