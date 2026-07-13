#!/usr/bin/env bash
# Start a virtual X display, launch the Scrivi Linux app on it, and expose it
# over VNC so it can be viewed from the host (the Mac). Used as the image CMD.
set -euo pipefail

export DISPLAY=:99
SCREEN_GEOMETRY="${SCRIVI_VNC_GEOMETRY:-1024x768x24}"

# VNC password. macOS's built-in Screen Sharing client REFUSES passwordless
# (-nopw) servers — it won't enable "Sign In" without a password. So we set one.
# Override with SCRIVI_VNC_PASSWORD; default "scrivi" (localhost-only alpha harness).
VNC_PASSWORD="${SCRIVI_VNC_PASSWORD:-scrivi}"

# Virtual framebuffer — no physical display needed.
Xvfb "$DISPLAY" -screen 0 "$SCREEN_GEOMETRY" &
XVFB_PID=$!

# Give Xvfb a moment to come up before clients connect to it.
for _ in $(seq 1 50); do
    if xdpyinfo -display "$DISPLAY" >/dev/null 2>&1; then
        break
    fi
    sleep 0.1
done

# The app. If it exits, we tear the whole container down (below).
/src/build/platforms/linux/scrivi_linux &
APP_PID=$!

# Serve the framebuffer over VNC on :5900 (container-internal; the host maps it,
# e.g. to 5901). -forever keeps serving across client reconnects. A password is
# stored so Apple's Screen Sharing client will connect (see above).
VNC_PASSFILE="/tmp/scrivi-vnc-passwd"
x11vnc -storepasswd "$VNC_PASSWORD" "$VNC_PASSFILE" >/dev/null 2>&1
x11vnc -display "$DISPLAY" -forever -shared -rfbauth "$VNC_PASSFILE" -rfbport 5900 -quiet &
VNC_PID=$!

cleanup() {
    kill "$APP_PID" "$VNC_PID" "$XVFB_PID" 2>/dev/null || true
}
trap cleanup EXIT INT TERM

# Exit when the app exits (so `docker run` returns when you close the window).
wait "$APP_PID"
