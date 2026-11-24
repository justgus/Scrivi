#!/usr/bin/env bash
# glyph_smoke.sh — EP-060 SP-243: the Inkwyn glyphs load, draw, follow the palette (light, dark, disabled) and HiDPI. Offscreen.
#
# Usage: glyph_smoke.sh <path-to-harness-binary>
set -euo pipefail

BIN="${1:?usage: glyph_smoke.sh <harness-binary>}"
export QT_QPA_PLATFORM=offscreen

echo "== The Inkwyn glyphs =="
OUT="$("$BIN")" || { echo "$OUT"; exit 1; }
echo "$OUT" | grep -v '^ok  ' || true
if ! echo "$OUT" | grep -qx 'glyph-ok'; then
    echo "FAIL: harness did not report glyph-ok"
    exit 1
fi
echo "PASS: every glyph loads, draws ink, and takes the palette's colour."
