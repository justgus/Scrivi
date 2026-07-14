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
#
# Two host directories are bind-mounted so state and projects survive the (--rm)
# container and are reachable from macOS:
#   • App-support  (identity + recents)  →  /root/.local/share/Scrivi
#       host default: ~/ScriviLinux/appsupport   (override: SCRIVI_APPSUPPORT_DIR)
#     Makes the local identity + recent-projects list persist across container
#     restarts — quit the app, re-run this script, and the same identity + recents
#     come back (SP-059 AC4/AC5, verifiable visually over VNC).
#   • Shared projects  →  /projects   (host default: ~/ScriviLinux/projects,
#       override: SCRIVI_PROJECTS_DIR). A folder visible on BOTH macOS and the
#     container: drop existing .scrivi packages here from macOS, then browse to
#     /projects in the app's "Choose…" folder picker to create/open there. New
#     projects created under /projects likewise appear on the Mac.
set -euo pipefail

REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../../.." && pwd)"
cd "$REPO_ROOT"

HOST_PORT="${SCRIVI_VNC_HOST_PORT:-5901}"
VNC_PASSWORD="${SCRIVI_VNC_PASSWORD:-scrivi}"

# Host-side directories (created if absent). Both are plain macOS folders you can
# open in Finder.
APPSUPPORT_DIR="${SCRIVI_APPSUPPORT_DIR:-$HOME/ScriviLinux/appsupport}"
PROJECTS_DIR="${SCRIVI_PROJECTS_DIR:-$HOME/ScriviLinux/projects}"
mkdir -p "$APPSUPPORT_DIR" "$PROJECTS_DIR"

echo "==> Building scrivi-linux image (Ubuntu 24.04 + Qt 6.4)…"
docker build -f platforms/linux/docker/Dockerfile -t scrivi-linux .

echo "==> Persistent app-support (identity + recents): $APPSUPPORT_DIR"
echo "==> Shared projects folder (macOS ↔ container /projects): $PROJECTS_DIR"
echo "==> Running. Connect a VNC client to vnc://localhost:${HOST_PORT} (password: ${VNC_PASSWORD})."
echo "    Use the in-app Quit button (or Ctrl-C here) to stop."
docker run --rm -p "${HOST_PORT}:5900" \
    -e "SCRIVI_VNC_PASSWORD=${VNC_PASSWORD}" \
    -v "${APPSUPPORT_DIR}:/root/.local/share/Scrivi" \
    -v "${PROJECTS_DIR}:/projects" \
    scrivi-linux
