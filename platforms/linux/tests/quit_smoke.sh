#!/usr/bin/env bash
# quit_smoke.sh — SP-062 regression guard: the landing Quit path must quit the app.
#
# Runs the scrivi_linux_quit_smoke harness, which wires QQmlEngine::quit() (what
# QML's Qt.quit() emits) to QApplication::quit the way main.cpp does, emits it, and
# asserts the app exits(0). Fails loudly (non-zero, no hang) if the signal is
# unconnected — the shell-flip bug.
#
# Uses the Qt "offscreen" platform (no display).
#
# Usage: quit_smoke.sh <path-to-harness-binary>
set -euo pipefail

BIN="${1:?usage: quit_smoke.sh <harness-binary>}"

export QT_QPA_PLATFORM=offscreen

echo "== landing Quit path (QQmlEngine::quit → app exit) =="
OUT="$("$BIN")"
if [ "$OUT" != "quit-ok" ]; then
    echo "FAIL: harness did not report quit-ok (got: '$OUT')"
    exit 1
fi

echo "PASS: Qt.quit() from the QQuickWidget-hosted landing quits the app."
