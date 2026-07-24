#!/usr/bin/env bash
# timeline_events_smoke.sh — EP-025 / SP-082 (T-0344, AC5) historical-events +
# imported-timelines + export persistence check.
#
# Runs the scrivi_linux_timeline_events_smoke harness against a temp $XDG_DATA_HOME + a
# temp project dir. Through the bridge (the layer EditorShell's SP-082 slots call):
# creates/updates/deletes a historical event (list round-trip); imports a fixture
# .scrivi-timeline.json (asserts stored + metadata + the on-disk file exists); toggles
# visibility + updates the epoch offset (both persist); exports the project timeline
# (asserts a valid scrivi.externalTimeline.v1 body carrying the surviving historical
# event); removes the import (list back to empty).
#
# Uses the Qt "offscreen" platform (a QCoreApplication is created; no window/display).
#
# Usage: timeline_events_smoke.sh <path-to-harness-binary>
set -euo pipefail

BIN="${1:?usage: timeline_events_smoke.sh <harness-binary>}"

WORKDIR="$(mktemp -d)"
trap 'rm -rf "$WORKDIR"' EXIT
export XDG_DATA_HOME="$WORKDIR/xdg"
export QT_QPA_PLATFORM=offscreen
PROJECT_DIR="$WORKDIR/projects/timeline-events.scrivi"

echo "== timeline events: historical CRUD + import/hide/show/offset/export/remove =="
"$BIN" "$PROJECT_DIR"

if [ ! -f "$PROJECT_DIR/project.json" ]; then
    echo "FAIL: $PROJECT_DIR/project.json was not created"
    exit 1
fi

echo "PASS: historical events + imported timelines + export round-trip."
