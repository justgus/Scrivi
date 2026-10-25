#!/usr/bin/env bash
# deploy-to-rig.sh — push the working tree to the Linux rig, build it there, and
# CONFIRM the binary reports the Mac's version.
#
#   platforms/linux/deploy-to-rig.sh            # deploy + build + verify
#   platforms/linux/deploy-to-rig.sh --test     # …and run ctest + the smokes
#   platforms/linux/deploy-to-rig.sh --run      # …and launch the app on the rig
#
# Host defaults to `oathkeeper`; override with SCRIVI_RIG. The remote path
# defaults to ~/Dev/Scrivi; override with SCRIVI_RIG_PATH.
#
# ⚠️ WHY THIS EXISTS: on 2026-08-30 a live pass was nearly run against a DAY-OLD
# binary that predated an entire sprint. Pushing source does NOT rebuild, and a
# green build on the workstation says NOTHING about what is installed on the rig.
# ⚠️ This script always ends by printing the stamp, so "did my change land?" is
# answered every time rather than assumed.
#
# ⚠️ It ships the WORKING TREE, including UNCOMMITTED and UNTRACKED files —
# deliberately, because that is what you are testing. `git ls-files` alone
# silently omits work in progress, and the configure then fails on missing
# sources (which is exactly what happened the first time).
set -euo pipefail

RIG="${SCRIVI_RIG:-oathkeeper}"
RIG_PATH="${SCRIVI_RIG_PATH:-Dev/Scrivi}"
REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
cd "$REPO_ROOT"

DO_TEST=0
DO_RUN=0
for arg in "$@"; do
    case "$arg" in
        --test) DO_TEST=1 ;;
        --run)  DO_RUN=1 ;;
        *) echo "unknown option: $arg" >&2; exit 2 ;;
    esac
done

echo "==> Rig: ${RIG}:~/${RIG_PATH}"

# --- reachability -----------------------------------------------------------
# ⚠️ The rig SLEEPS. A failed connection here usually means asleep, not broken.
if ! ssh -o BatchMode=yes -o ConnectTimeout=8 "$RIG" true 2>/dev/null; then
    echo "ERROR: cannot reach ${RIG} over SSH." >&2
    echo "       The rig may simply be ASLEEP — wake it and try again." >&2
    echo "       (Key auth needs a ~/.ssh/config entry naming the dedicated key;" >&2
    echo "        see docs/Scrivi_Linux_Rig_Setup_v0_1.md §2.1.)" >&2
    exit 1
fi

# --- ✅ [T-0622] the version this tree builds as --------------------------------
# The shipped tree has no git history, so the Mac's numbers travel with it in
# `.scrivi-version-override` ("<count> <hash>-dirty"; git-ignored), which
# `platforms/linux/cmake/ScriviVersion.cmake` reads instead of git. Marked -dirty
# whenever the tree differs from HEAD, which is what this route is for.
COUNT="$(git rev-list --count HEAD)"
HASH="$(git rev-parse --short=7 HEAD)"
[ -n "$(git status --porcelain)" ] && HASH="${HASH}-dirty"
EXPECTED="$(tr -d '[:space:]' < VERSION).${COUNT} (${HASH})"
echo "==> Version: ${EXPECTED}"

# --- package the working tree ----------------------------------------------
STAGE="$(mktemp -d)"
trap 'rm -rf "$STAGE"' EXIT
{ git ls-files; git ls-files --others --exclude-standard; } | sort -u > "$STAGE/filelist"
COUNT=$(wc -l < "$STAGE/filelist" | tr -d ' ')
echo "==> Packaging ${COUNT} files (tracked + untracked, excluding .gitignore'd)…"
printf '%s %s\n' "$COUNT" "$HASH" > "$STAGE/.scrivi-version-override"
tar -czf "$STAGE/src.tgz" -T "$STAGE/filelist" -C "$STAGE" .scrivi-version-override

# --- ship + unpack ----------------------------------------------------------
echo "==> Copying to ${RIG}…"
scp -q "$STAGE/src.tgz" "${RIG}:/tmp/scrivi-src.tgz"

# ⚠️ macOS `tar` regenerates AppleDouble `._*` files from extended attributes
# even when they were deleted before archiving, so they are stripped ON THE RIG.
ssh "$RIG" "set -e
    mkdir -p ~/${RIG_PATH}
    cd ~/${RIG_PATH}
    tar -xzf /tmp/scrivi-src.tgz 2>/dev/null || true
    rm -f /tmp/scrivi-src.tgz
    find . -name '._*' -delete 2>/dev/null || true"

# --- configure (only if needed) + build ------------------------------------
# ⚠️ A CMake build tree does NOT survive being moved — CMakeCache.txt bakes
# absolute paths. Configuring only when the cache is absent keeps rebuilds fast
# while still handling a fresh clone.
echo "==> Building natively on ${RIG}…"
ssh "$RIG" "set -e
    cd ~/${RIG_PATH}
    if [ ! -f build-native/CMakeCache.txt ]; then
        cmake -S . -B build-native -G Ninja \
              -DSCRIVI_BUILD_LINUX=ON -DSCRIVI_BUILD_TESTS=ON \
              -DCMAKE_BUILD_TYPE=Debug
    else
        cmake -B build-native -DCMAKE_BUILD_TYPE=Debug >/dev/null   # an older tree's cache may say Release
    fi
    cmake --build build-native --parallel"

# --- ⚠️ THE POINT OF THE SCRIPT --------------------------------------------
# ✅ [T-0622]: the binary's version must be the one this tree was shipped as.
echo
VERSION_LINE="$(ssh "$RIG" "~/${RIG_PATH}/build-native/platforms/linux/scrivi_linux --version")"
echo "==> Deployed:  ${VERSION_LINE}"
if ! printf '%s' "$VERSION_LINE" | grep -qF "$EXPECTED"; then
    echo
    echo "ERROR: expected ${EXPECTED} but the rig reports: ${VERSION_LINE}" >&2
    echo "       The binary does NOT match the source just pushed." >&2
    exit 1
fi
echo
echo "    Check the app reports:  ${EXPECTED}   (Help ▸ About Scrivi)"
echo "    ⚠️  Anything else means you are running an older binary."

# --- optional: tests --------------------------------------------------------
if [ "$DO_TEST" = "1" ]; then
    echo
    echo "==> ctest (non-root) + Linux smokes…"
    ssh "$RIG" "set -e
        cd ~/${RIG_PATH}
        ctest --test-dir build-native 2>&1 | tail -3
        f=0; p=0
        for sh in platforms/linux/tests/*.sh; do
            n=\$(basename \"\$sh\" .sh)
            b=\"./build-native/platforms/linux/scrivi_linux_\${n}\"
            [ -x \"\$b\" ] || continue
            if bash \"\$sh\" \"\$b\" >/dev/null 2>&1; then p=\$((p+1)); else echo \"FAIL \$n\"; f=\$((f+1)); fi
        done
        echo \"smokes: \$p passed, \$f failed\""
fi

# --- optional: launch -------------------------------------------------------
if [ "$DO_RUN" = "1" ]; then
    echo
    # ⚠️ The app needs a REAL display, which SSH does not have. Launching it from
    # here would either fail or push a window onto someone else's session.
    echo "==> To run it, open a terminal INSIDE the RDP session and use:"
    echo "        ~/${RIG_PATH}/build-native/platforms/linux/scrivi_linux"
    echo "    …or the Scrivi shortcut on the rig's Desktop."
    echo
    echo "    ⚠️  It cannot be launched over SSH: there is no DISPLAY, and forcing"
    echo "        one would push a window onto the RDP session rather than test it."
fi

echo
echo "==> Done."
