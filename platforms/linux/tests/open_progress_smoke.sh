#!/usr/bin/env bash
# open_progress_smoke.sh — SP-128 / T-0500 ([I-0195]) project-open progress.
#
# Drives a REAL EditorShell::load() against a temp project with several scenes,
# and asserts the progress it reports is DETERMINATE: the total arrives with the
# FIRST report (before the per-scene body reads), never changes mid-load, never
# goes backwards, and ends EXACTLY at the total.
#
# ⚠️ That is the arithmetic under SP-128 §2a's ruling — `files read / files to
# read` is a COUNT, not an estimate. A total that moved would be an estimate, and
# an estimate that grows is how a bar ends up going backwards in front of a
# writer.
#
# ⚠️ THIS DOES NOT COVER THE DEFECT [I-0195] WAS FILED FOR. That is UI
# responsiveness under a SLOW mount, which needs a real `cache=none` share and a
# human watching — T-0501's live pass. Docker has no slow mount, so a green run
# here says nothing about it.
#
# Uses the Qt "offscreen" platform (widgets are built; no window appears).
#
# Usage: open_progress_smoke.sh <path-to-harness-binary>
set -euo pipefail

BIN="${1:?usage: open_progress_smoke.sh <harness-binary>}"

WORKDIR="$(mktemp -d)"
trap 'rm -rf "$WORKDIR"' EXIT
export XDG_DATA_HOME="$WORKDIR/xdg"
export QT_QPA_PLATFORM=offscreen
PROJECT_DIR="$WORKDIR/projects/open-progress.scrivi"

echo "== open progress: determinate total, monotonic, ends at the total =="
"$BIN" "$PROJECT_DIR"
