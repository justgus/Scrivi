#!/usr/bin/env bash
# structural_baseline_smoke.sh — EP-053 S2a (SP-187 / T-0615): each structural operation through a real EditorShell,
# disk-verified, printing the baseline S2b removes (GUI-thread core calls, project loads, document builds). The scene
# list's text is checked against the document after every change.
#
# Usage: structural_baseline_smoke.sh <path-to-harness-binary>
set -euo pipefail

BIN="${1:?usage: structural_baseline_smoke.sh <harness-binary>}"

WORKDIR="$(mktemp -d)"
trap 'rm -rf "$WORKDIR"' EXIT
export XDG_DATA_HOME="$WORKDIR/xdg"
export QT_QPA_PLATFORM=offscreen
export SCRIVI_VERIFY_SCENE_TEXT=1

echo "== Structural operations: the EP-053 baseline =="
"$BIN" "$WORKDIR/projects/baseline.scrivi"
