#!/usr/bin/env bash
# timeline_cluster_smoke.sh — EP-025 / SP-084 (T-0349, AC6b) co-located-dot clustering
# check.
#
# Runs the scrivi_linux_timeline_cluster_smoke harness, which drives TimelinePanel's
# clustering pass directly (no mouse/paint): co-located dots collapse into one aggregate
# of N; zooming in resolves them into singletons (zoom-resolve); well-separated dots
# never cluster; a historical event co-located with scenes joins the aggregate.
#
# Uses the Qt "offscreen" platform (a QApplication is created; no window/display).
#
# Usage: timeline_cluster_smoke.sh <path-to-harness-binary>
set -euo pipefail

BIN="${1:?usage: timeline_cluster_smoke.sh <harness-binary>}"

export QT_QPA_PLATFORM=offscreen

echo "== clustering: coincident → aggregate, zoom-resolve, separated → none, hist joins =="
"$BIN"

echo "PASS: co-located dot clustering + zoom-resolve."
