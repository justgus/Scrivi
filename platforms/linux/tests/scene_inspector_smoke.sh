#!/usr/bin/env bash
# scene_inspector_smoke.sh — EP-035 / SP-125 (T-0480–T-0483) Scene Inspector
# read-path check.
#
# Runs the scrivi_linux_scene_inspector_smoke harness against a temp
# $XDG_DATA_HOME + a temp project dir. Exercises the panel's read path THROUGH
# ScriviBridge — never the C ABI directly (feedback_boundary_tests_not_facade).
#
# Covers: ObjectKindScope deriving the kind partition from the core (and matching
# it kind-for-kind); the unknown-kind fallback being world-scoped on purpose; the
# edge label + far-endpoint kind arriving ALREADY RESOLVED; and — the one this
# sprint most needed — an EMPTY listing being distinguishable from a FAILED call,
# which is what lets the panel tell "no objects" from "unreadable" (T-0483).
#
# ⚠️ This does NOT replace EP-035 AC9's live click-through. It covers the read
# path's data decisions; a human at the app covers everything else.
#
# Uses the Qt "offscreen" platform (a QCoreApplication is created; no window).
#
# Usage: scene_inspector_smoke.sh <path-to-harness-binary>
set -euo pipefail

BIN="${1:?usage: scene_inspector_smoke.sh <harness-binary>}"

WORKDIR="$(mktemp -d)"
trap 'rm -rf "$WORKDIR"' EXIT
export XDG_DATA_HOME="$WORKDIR/xdg"
export QT_QPA_PLATFORM=offscreen
PROJECT_DIR="$WORKDIR/projects/scene-inspector.scrivi"

echo "== scene inspector: kind scope derived, labels read, empty != failed =="
PROJECT_ID="$("$BIN" "$PROJECT_DIR")"
echo "projectID: $PROJECT_ID"

if [ ! -f "$PROJECT_DIR/project.json" ]; then
    echo "FAIL: $PROJECT_DIR/project.json was not created"
    exit 1
fi

echo "PASS: kind scope derived from the core; empty and failed listings separable."
