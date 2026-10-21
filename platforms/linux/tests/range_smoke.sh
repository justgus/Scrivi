#!/usr/bin/env bash
# range_smoke.sh — EP-053 S2a (SP-187 / T-0615): the range endpoints through ScriviBridge, the delta decoded into
# RangeDelta, every write checked by reopening the project. Qt offscreen (a QCoreApplication; no window).
#
# Usage: range_smoke.sh <path-to-harness-binary>
set -euo pipefail

BIN="${1:?usage: range_smoke.sh <harness-binary>}"

WORKDIR="$(mktemp -d)"
trap 'rm -rf "$WORKDIR"' EXIT
export XDG_DATA_HOME="$WORKDIR/xdg"
export QT_QPA_PLATFORM=offscreen

echo "== range endpoints through ScriviBridge =="
"$BIN" "$WORKDIR/projects/range.scrivi"
