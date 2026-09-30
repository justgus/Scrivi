#!/usr/bin/env bash
# restore_guard_smoke.sh — EP-043 / SP-147 (T-0566), the R6 guard.
#
# Runs the harness against a TEMP app-support root THREE times:
#   1. NO guard signal — the negative control: the funnel must return the open set.
#   2. QT_QPA_PLATFORM=offscreen — a headless run: restore suppressed, file intact.
#   3. SCRIVI_NO_RESTORE=1       — the operator's switch: same.
#
# ⚠️ Pass 1 is what makes 2 and 3 mean anything: without it, a funnel that always
# returned nothing would pass. ⛔ [SP-147] AC7: removing the guard turns 2/3 RED.
#
# Usage: restore_guard_smoke.sh <path-to-harness-binary>
set -euo pipefail

BIN="${1:?usage: restore_guard_smoke.sh <harness-binary>}"

WORKDIR="$(mktemp -d)"
trap 'rm -rf "$WORKDIR"' EXIT
ROOT="$WORKDIR/appsupport"
mkdir -p "$ROOT"

echo "== pass 1: no guard signal (negative control) =="
env -u QT_QPA_PLATFORM -u SCRIVI_NO_RESTORE "$BIN" "$ROOT" restore

echo "== pass 2: QT_QPA_PLATFORM=offscreen =="
env -u SCRIVI_NO_RESTORE QT_QPA_PLATFORM=offscreen "$BIN" "$ROOT" suppressed

echo "== pass 3: SCRIVI_NO_RESTORE=1 =="
env -u QT_QPA_PLATFORM SCRIVI_NO_RESTORE=1 "$BIN" "$ROOT" suppressed

echo "PASS: a guarded run restores nothing and leaves session.ini intact (R6)."
