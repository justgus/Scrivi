#!/usr/bin/env bash
# chapter_titles_smoke.sh — [SP-184] (EP-048 S5c, T-0611): Show chapter titles (on by default) and the chapter-end divider,
# through REAL project windows and the real dispatch; files read back FROM DISK. Offscreen.
#
# Usage: chapter_titles_smoke.sh <path-to-harness-binary>
set -euo pipefail

BIN="${1:?usage: chapter_titles_smoke.sh <harness-binary>}"

WORKDIR="$(mktemp -d)"
trap 'rm -rf "$WORKDIR"' EXIT
export XDG_DATA_HOME="$WORKDIR/xdg"
export QT_QPA_PLATFORM=offscreen

echo "== Chapter titles and the chapter-end divider =="
OUT="$("$BIN" "$WORKDIR/projects")" || { echo "$OUT"; exit 1; }
echo "$OUT"
if ! echo "$OUT" | grep -qx 'titles-ok'; then
    echo "FAIL: harness did not report titles-ok"
    exit 1
fi
echo "PASS: chapter titles show and hide, and every chapter closes with its divider, on Linux."
