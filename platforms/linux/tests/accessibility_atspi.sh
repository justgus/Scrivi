#!/usr/bin/env bash
# accessibility_atspi.sh — EP-050 S4 (SP-203, T-0659): the manuscript read over a REAL AT-SPI bus, as Orca reads it.
#
# Its own D-Bus session, runtime folder, accessibility bus and Xvfb display: nothing reaches the desktop, its Orca or its log.
# Needs Xvfb, at-spi2-core and python3-gi + gir1.2-atspi-2.0 (the rig and scrivi-vm; not the CI image).
#
# Usage: accessibility_atspi.sh <path-to-scrivi_linux_accessibility_smoke>
set -euo pipefail

BIN="${1:?usage: accessibility_atspi.sh <scrivi_linux_accessibility_smoke>}"
HERE="$(cd "$(dirname "$0")" && pwd)"
export BIN HERE

echo "== Accessibility over AT-SPI (GetText, lines, caret, text-caret-moved) =="
dbus-run-session -- bash -c '
    # ⛔ Its OWN runtime folder: at-spi-bus-launcher puts its socket at $XDG_RUNTIME_DIR/at-spi/bus and removes it on exit.
    # Under /run/user/UID that deleted the bus socket OF THE DESKTOP (2026-11-04: Orca then aborted at start).
    # ⚠️ No apostrophes in here: this is a single-quoted bash -c string.
    export XDG_RUNTIME_DIR="$(mktemp -d)"
    Xvfb :97 -screen 0 1280x800x24 -nolisten tcp >/dev/null 2>&1 & X=$!
    export DISPLAY=:97   # the bus, the registry (key synthesis) and the checker all on this display
    sleep 1
    /usr/libexec/at-spi-bus-launcher --launch-immediately >/dev/null 2>&1 & B=$!
    sleep 1
    QT_QPA_PLATFORM=xcb QT_LINUX_ACCESSIBILITY_ALWAYS_ON=1 SCRIVI_A11Y_HOLD_MS=15000 \
        XDG_STATE_HOME="$(mktemp -d)" "$BIN" >/dev/null 2>&1 & APP=$!
    sleep 4
    python3 -I "$HERE/accessibility_atspi.py"; R=$?
    kill $APP $B $X 2>/dev/null
    wait 2>/dev/null
    rm -rf "$XDG_RUNTIME_DIR"
    exit $R
' | tee /dev/stderr | grep -qx 'atspi-ok' || { echo "FAIL: AT-SPI did not report atspi-ok"; exit 1; }
echo "PASS: over AT-SPI, Orca is given the page the writer reads."
