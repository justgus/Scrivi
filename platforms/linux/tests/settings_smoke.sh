#!/usr/bin/env bash
# settings_smoke.sh — SP-182 (EP-048 S5a, T-0609): Linux's Project Settings dialog, the bundled typefaces and Markup Hints,
# through REAL project windows and the real dispatch; files read back FROM DISK. Offscreen.
#
# Usage: settings_smoke.sh <path-to-harness-binary>
set -euo pipefail

BIN="${1:?usage: settings_smoke.sh <harness-binary>}"

WORKDIR="$(mktemp -d)"
trap 'rm -rf "$WORKDIR"' EXIT
export XDG_DATA_HOME="$WORKDIR/xdg"
export QT_QPA_PLATFORM=offscreen

echo "== Project settings (the dialog, the bundled faces, Markup Hints) =="
OUT="$("$BIN" "$WORKDIR/projects")" || { echo "$OUT"; exit 1; }
echo "$OUT"
if ! echo "$OUT" | grep -qx 'settings-ok'; then
    echo "FAIL: harness did not report settings-ok"
    exit 1
fi
echo "PASS: Project Settings, the bundled faces and Markup Hints work on Linux."
