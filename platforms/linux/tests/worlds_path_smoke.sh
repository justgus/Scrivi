#!/usr/bin/env bash
# worlds_path_smoke.sh — EP-035 / SP-127 (I-0184).
#
# ⚠️ The Worlds row's PATH must be readable at the width the row actually gets.
# It previously elided against a hardcoded 360 px, ONCE, at construction — so a
# narrow row showed almost nothing and widening the dialog never helped. The DoD
# asks for "name, path, status"; this proves the path half is really there.
#
# Usage: worlds_path_smoke.sh <path-to-harness-binary>
set -euo pipefail

BIN="${1:?usage: worlds_path_smoke.sh <harness-binary>}"

export QT_QPA_PLATFORM=offscreen

echo "== worlds dialog: the path is readable and re-elides on resize =="
"$BIN"
echo "PASS: path survives a narrow row and grows with the dialog."
