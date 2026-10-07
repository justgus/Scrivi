#!/usr/bin/env bash
# presenter_smoke.sh — EP-048 S2 (SP-166, T-0595): the Linux manuscript presenter, offscreen.
#
# Usage: presenter_smoke.sh <path-to-harness-binary>
set -euo pipefail

BIN="${1:?usage: presenter_smoke.sh <harness-binary>}"
export QT_QPA_PLATFORM=offscreen

echo "== manuscript presenter (hidden escapes/markers, headings, reveal, caret snap, atomic markers) =="
OUT="$("$BIN")"
echo "$OUT" | grep '^cost:' || true
if ! echo "$OUT" | grep -qx 'presenter-ok'; then
    echo "FAIL: harness did not report presenter-ok"
    exit 1
fi
echo "PASS: the presenter draws what Apple draws, and the stored text is untouched."
