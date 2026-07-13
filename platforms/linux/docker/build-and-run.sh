#!/usr/bin/env bash
# Convenience: build the Scrivi Linux image and run it with VNC exposed on the
# host. Run from anywhere; it resolves the repo root itself. Requires Docker.
#
#   platforms/linux/docker/build-and-run.sh
#
# Then connect a VNC client on the Mac to  vnc://localhost:5901
# (Finder → Go → Connect to Server, or any VNC viewer). Password: scrivi
#
# Host port defaults to 5901, NOT 5900: macOS's built-in Screen Sharing already
# binds 5900, so mapping to 5900 fails with "address already in use". Override
# the host port with SCRIVI_VNC_HOST_PORT, and the password with SCRIVI_VNC_PASSWORD.
set -euo pipefail

REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../../.." && pwd)"
cd "$REPO_ROOT"

HOST_PORT="${SCRIVI_VNC_HOST_PORT:-5901}"
VNC_PASSWORD="${SCRIVI_VNC_PASSWORD:-scrivi}"

echo "==> Building scrivi-linux image (Ubuntu 24.04 + Qt 6.4)…"
docker build -f platforms/linux/docker/Dockerfile -t scrivi-linux .

echo "==> Running. Connect a VNC client to vnc://localhost:${HOST_PORT} (password: ${VNC_PASSWORD})."
echo "    Close the app window (or Ctrl-C here) to stop."
docker run --rm -p "${HOST_PORT}:5900" -e "SCRIVI_VNC_PASSWORD=${VNC_PASSWORD}" scrivi-linux
