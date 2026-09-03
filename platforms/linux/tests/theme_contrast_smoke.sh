#!/usr/bin/env bash
# theme_contrast_smoke.sh — EP-035 / SP-127 (I-0186).
#
# ⚠️ Secondary text must be legible in the themes people actually run. The world
# PATH shipped at 1.07:1 in Yaru-dark (invisible) and 1.03:1 in Yaru light,
# because `palette(mid)` is a STRUCTURAL role with no contrast guarantee.
#
# ⚠️ THE POINT OF THIS WRAPPER: it runs the check under a REAL GTK THEME, in BOTH
# polarities. Qt's no-theme fallback renders those roles as readable greys, which
# is why every previous offscreen test and screenshot missed the defect entirely.
#
# Usage: theme_contrast_smoke.sh <path-to-harness-binary>
set -euo pipefail

BIN="${1:?usage: theme_contrast_smoke.sh <harness-binary>}"

export QT_QPA_PLATFORM=offscreen

# ⚠️ Without the gtk3 platform theme Qt uses its built-in palette and the test is
# meaningless. Skip loudly rather than pass vacuously if the plugin is absent.
if [ ! -e /usr/lib/x86_64-linux-gnu/qt6/plugins/platformthemes/libqgtk3.so ]; then
    echo "SKIP: no gtk3 Qt platform theme installed — cannot test real palettes." >&2
    exit 0
fi

# ⚠️ AND THE GTK PLATFORM THEME NEEDS A DISPLAY, even under QT_QPA_PLATFORM=offscreen:
# GTK itself opens one to read the theme, and dies with "cannot open display" if it
# cannot. ⚠️ This is why the smoke passes when run from inside an RDP session and
# FAILS over plain SSH — which is exactly how the deploy script runs it.
#
# ⚠️ DO NOT "fix" this by dropping the gtk3 theme: the test would then measure Qt's
# no-theme fallback, ✅ which is the very blind spot I-0186 existed in. A vacuous
# pass here is worse than no test at all.
if [ -z "${DISPLAY:-}" ] && [ -z "${WAYLAND_DISPLAY:-}" ]; then
    if command -v xvfb-run >/dev/null 2>&1; then
        exec xvfb-run -a "$0" "$BIN"
    fi
    echo "SKIP: no DISPLAY and no xvfb-run — the gtk3 theme cannot be loaded, and" >&2
    echo "      testing without it would measure Qt's fallback palette, not a real theme." >&2
    echo "      Run this from inside the RDP session, or install xvfb." >&2
    exit 0
fi
export QT_QPA_PLATFORMTHEME=gtk3

echo "== theme contrast: dark =="
GTK_THEME=Yaru-dark "$BIN" "Yaru-dark"

echo "== theme contrast: light =="
GTK_THEME=Yaru "$BIN" "Yaru"

echo "PASS: secondary text meets WCAG AA in both theme polarities."
