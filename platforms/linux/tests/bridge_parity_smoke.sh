#!/usr/bin/env bash
# bridge_parity_smoke.sh — EP-034 / SP-121 (T-0464) ABI-parity check.
#
# Runs the scrivi_linux_bridge_parity_smoke harness against a temp $XDG_DATA_HOME
# + a temp project dir. Exercises every endpoint SP-121 bridged THROUGH
# ScriviBridge — never the C ABI directly (feedback_boundary_tests_not_facade: a
# facade test cannot see a boundary gap, which is how I-0113 shipped green).
#
# Covers: listObjectKinds (and asserts `source` is project-scoped FROM THE CORE,
# never from a restated list); createObject/listObjects/openObject/saveObject;
# listRelationTypes (+ `cites` seeded); createEdge/listEdgesFor/deleteEdge (+ the
# label arriving already-resolved); worlds, assets, scene metadata, comments,
# inbox, external-change scan, searchable text.
#
# Uses the Qt "offscreen" platform (a QCoreApplication is created; no window).
#
# Usage: bridge_parity_smoke.sh <path-to-harness-binary>
set -euo pipefail

BIN="${1:?usage: bridge_parity_smoke.sh <harness-binary>}"

WORKDIR="$(mktemp -d)"
trap 'rm -rf "$WORKDIR"' EXIT
export XDG_DATA_HOME="$WORKDIR/xdg"
export QT_QPA_PLATFORM=offscreen
PROJECT_DIR="$WORKDIR/projects/bridge-parity.scrivi"

echo "== bridge parity: every SP-121 endpoint reached through ScriviBridge =="
PROJECT_ID="$("$BIN" "$PROJECT_DIR")"
echo "projectID: $PROJECT_ID"

if [ ! -f "$PROJECT_DIR/project.json" ]; then
    echo "FAIL: $PROJECT_DIR/project.json was not created"
    exit 1
fi

echo "PASS: all bridged endpoints reachable; kind scope + edge labels read from the core."
