#!/usr/bin/env bash
# inspector_layout_smoke.sh — EP-035 / SP-126 (T-0486).
#
# ⚠️ THE SPRINT'S DATA-LOSS TEST. Proves `inspector-layout.json` is PATCHED and
# never reconstructed: an Apple-written document must survive a Linux write with
# `stackSort`, `defaultStacks`, per-scene `scenes` — and keys this build has
# never heard of — all intact.
#
# ⚠️ Without this guarantee, opening a project on Linux silently deletes the
# writer's Apple-side card layout, and she does not find out until she reopens it
# on the Mac.
#
# Usage: inspector_layout_smoke.sh <path-to-harness-binary>
set -euo pipefail

BIN="${1:?usage: inspector_layout_smoke.sh <harness-binary>}"

WORKDIR="$(mktemp -d)"
trap 'rm -rf "$WORKDIR"' EXIT
export QT_QPA_PLATFORM=offscreen

echo "== inspector layout: patched, never reconstructed =="
"$BIN" "$WORKDIR"
echo "PASS: Apple-written keys survive a Linux write."
