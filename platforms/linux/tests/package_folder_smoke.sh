#!/usr/bin/env bash
# package_folder_smoke.sh — EP-035 / SP-127 (I-0185).
#
# ⚠️ `.scrivi` and `.scrivworld` are DIRECTORIES, so an ordinary directory picker
# walks INTO them: the writer double-clicks the project she wants and ends up
# looking at `manuscript/` and `objects/`. It still worked — which is why nothing
# caught it — but it lands on the app's two most common actions.
#
# Usage: package_folder_smoke.sh <path-to-harness-binary>
set -euo pipefail

BIN="${1:?usage: package_folder_smoke.sh <harness-binary>}"

WORKDIR="$(mktemp -d)"
trap 'rm -rf "$WORKDIR"' EXIT
export QT_QPA_PLATFORM=offscreen

echo "== package folder: a package is a leaf, not a folder to enter =="
"$BIN" "$WORKDIR"
echo "PASS: packages select; inside-paths resolve up; ordinary folders unchanged."
