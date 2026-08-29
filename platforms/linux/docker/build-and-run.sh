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
#   • ⚠️ The REAL RIG DRIVE  →  mounted at the SAME ABSOLUTE PATH it has on macOS
#       (host default: "/Volumes/Scrivi Worlds", override: SCRIVI_WORLDS_DIR;
#       skipped silently when not mounted).
#     Carries the Eskandar world and the real projects (project_test_rig_tintagael_eskandar).
#
#     ⚠️ THE IDENTICAL PATH IS LOad-BEARING, NOT TIDINESS. A project's world
#     binding stores an ABSOLUTE path — the-stairs-of-tintagael.scrivi records
#     `/Volumes/Scrivi Worlds/Eskandar.scrivworld/`. Mount the drive anywhere else
#     (e.g. /worlds) and that path does not exist in the container, so the app
#     reports **"World 'Eskandar' is unavailable. These 5 links are held pending."**
#     — CORRECTLY, because from inside the container it genuinely is not there.
#     ✅ Verified both ways 2026-08-28: at /worlds the world is pending; at the
#     identical path it resolves and every object renders available.
#
# ⚠️⚠️ THE /worlds MOUNT IS READ-WRITE, ON THE ORIGINALS — user ruling 2026-08-28.
#
#     This is REAL WRITING WORK, not a fixture. The app can modify it, and so can
#     anything else running in the container. **BACK IT UP BEFORE A SESSION THAT
#     WILL WRITE.**
#
#     ✅ NOTE: simply OPENING a project is NOT a risk (I-0174, closed not-a-defect).
#     It reconciles the world cache — which is how a SHARED world propagates one
#     project's objects to another — and touches no prose. The real hazard is
#     AUTOMATION typing into an open editor (I-0175), not the app itself.
#
#     ⚠️ Set SCRIVI_WORLDS_READONLY=1 to mount it :ro instead — do that for any
#     session that only needs to READ (world binding, pending states, browsing).
#     Read-only is the right default for most passes; read-write is for
#     exercising object CRUD (EP-035 AC4) against real data.
#
#     ⚠️ NEVER point an automated suite at this path. The Linux smokes and the
#     fixture builder create their own throwaway projects precisely so they never
#     touch it.
#
# ⚠️ BOTH MOUNTS ARE NEEDED TOGETHER for the real click-through projects.
#     `the-stairs-of-tintagael.scrivi` and `the-lone-golem.scrivi` live in
#     $PROJECTS_DIR (→ /projects) and are BOTH bound to the SAME Eskandar world on
#     the drive. The projects mount makes them openable; the drive-at-its-own-path
#     mount makes their shared world RESOLVE. With only the first, both projects
#     open with every world object held pending.
#
# ⚠️ NEVER drive this session with synthetic input (xdotool) while real work is
#     open — I-0175: a `ctrl+q` silently failed and the next typed path landed in
#     the user's manuscript, which the idle-save then persisted. The AC9
#     click-through is a HUMAN pass; that is the point of it.
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

# The real rig drive. ⚠️ Deliberately NOT created if absent — an unplugged drive
# must stay absent, so the app sees a genuinely unavailable world rather than an
# empty directory we invented. (A fabricated empty world would read as "missing",
# and a wrong "missing" is materially worse than an honest "unavailable": only
# "missing" invites destructive remedies. See WorldTypes.hpp.)
WORLDS_DIR="${SCRIVI_WORLDS_DIR:-/Volumes/Scrivi Worlds}"
WORLDS_MOUNTED=0
WORLDS_SPEC=""
WORLDS_NOTE=""
if [ -d "$WORLDS_DIR" ]; then
    WORLDS_MOUNTED=1
    # ⚠️ Mounted at its OWN path inside the container, not at /worlds — the
    # world bindings on this drive store absolute macOS paths, and a different
    # mount point makes every world read as unavailable. See the header note.
    if [ "${SCRIVI_WORLDS_READONLY:-0}" = "1" ]; then
        WORLDS_SPEC="${WORLDS_DIR}:${WORLDS_DIR}:ro"
        WORLDS_NOTE="READ-ONLY"
    else
        WORLDS_SPEC="${WORLDS_DIR}:${WORLDS_DIR}"
        WORLDS_NOTE="⚠️  READ-WRITE — the app CAN modify your real writing work"
    fi
fi

echo "==> Building scrivi-linux image (Ubuntu 24.04 + Qt 6.4)…"
docker build -f platforms/linux/docker/Dockerfile -t scrivi-linux .

echo "==> Persistent app-support (identity + recents): $APPSUPPORT_DIR"
echo "==> Shared projects folder (macOS ↔ container /projects): $PROJECTS_DIR"
if [ "$WORLDS_MOUNTED" = "1" ]; then
    echo "==> Rig drive, mounted at its own path (macOS == container): $WORLDS_DIR"
    echo "    $WORLDS_NOTE"
else
    echo "==> Rig drive NOT mounted (\"$WORLDS_DIR\" is not present)."
    echo "    That is not an error — an unplugged drive SHOULD be absent, so the app"
    echo "    reports an honestly unavailable world. Set SCRIVI_WORLDS_DIR to override."
fi
echo "==> Running. Connect a VNC client to vnc://localhost:${HOST_PORT} (password: ${VNC_PASSWORD})."
echo "    Use the in-app Quit button (or Ctrl-C here) to stop."
# ⚠️ The rig-drive mount is added as a separate argument pair rather than an
# array expansion: macOS ships bash 3.2, where `"${ARR[@]}"` on an EMPTY array is
# an "unbound variable" error under `set -u` — so the array form crashed this
# script in exactly the case it is meant to handle gracefully (drive unplugged).
if [ "$WORLDS_MOUNTED" = "1" ]; then
    docker run --rm -p "${HOST_PORT}:5900" \
        -e "SCRIVI_VNC_PASSWORD=${VNC_PASSWORD}" \
        -v "${APPSUPPORT_DIR}:/root/.local/share/Scrivi" \
        -v "${PROJECTS_DIR}:/projects" \
        -v "${WORLDS_SPEC}" \
        scrivi-linux
else
    docker run --rm -p "${HOST_PORT}:5900" \
        -e "SCRIVI_VNC_PASSWORD=${VNC_PASSWORD}" \
        -v "${APPSUPPORT_DIR}:/root/.local/share/Scrivi" \
        -v "${PROJECTS_DIR}:/projects" \
        scrivi-linux
fi
