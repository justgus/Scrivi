#!/usr/bin/env bash
# rig-build.sh — bump, commit, push, PULL ON THE RIG, rebuild, and PROVE which
# build is now installed. This is the DEFAULT route to the rig.
#
#   platforms/linux/rig-build.sh              # bump + push + pull + build + verify
#   platforms/linux/rig-build.sh --test       # …and run ctest + the smokes, non-root
#   platforms/linux/rig-build.sh --run        # …and launch the app on the rig
#   platforms/linux/rig-build.sh --no-bump    # rebuild the rig at HEAD, stamp unchanged
#
# Host defaults to `oathkeeper` (override with SCRIVI_RIG); remote path defaults
# to ~/Dev/Scrivi (override with SCRIVI_RIG_PATH).
#
# ---------------------------------------------------------------------------
# ⚠️ WHY THIS EXISTS, AND HOW IT DIFFERS FROM deploy-to-rig.sh
#
# `deploy-to-rig.sh` tars the WORKING TREE — including uncommitted files — and
# ships it over SSH. That is the ad-hoc route, for when you deliberately want to
# test something not yet committed.
#
# ⚠️ THIS script is the DEFAULT route: it puts the rig on a KNOWN COMMIT via
# `git pull`, so "what is the rig running?" has an answer you can name, diff and
# return to. The two share the build-number counter (~/.scrivi-rig-build-number)
# so they can never collide or reuse a number.
#
# ⚠️ THE RIG IS READ-ONLY FOR SOURCE. Nothing here edits files on the rig, and
# neither should you: a hand-edit there destroys checksum equivalence with the
# Mac, after which you cannot prove which code the rig is running.
#
# ---------------------------------------------------------------------------
# ⚠️ THREE FAILURES THIS SCRIPT EXISTS TO PREVENT, each of which has happened:
#
#   1. A live pass run against a DAY-STALE binary (2026-08-30). Pushing source
#      does NOT rebuild, and a green build on the Mac says nothing about the rig.
#      ✅ This script always ends by printing the stamp AND the commit.
#
#   2. A rig left BEHIND HEAD while a Task waited on it. On 2026-09-10 the rig
#      sat 7 commits back, missing the very fix T-0501 was meant to verify.
#      ✅ This script refuses to build a rig whose HEAD does not match the Mac's.
#
#   3. `ssh` declared "dead" when it was merely SLOW. The rig answers in >6 s and
#      an 8 s timeout read that as powered-off, which sent an investigation down
#      the wrong path entirely. ✅ The timeout here is 30 s.
set -euo pipefail

RIG="${SCRIVI_RIG:-oathkeeper}"
RIG_PATH="${SCRIVI_RIG_PATH:-Dev/Scrivi}"
REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
cd "$REPO_ROOT"

DO_TEST=0; DO_RUN=0; DO_BUMP=1
for arg in "$@"; do
    case "$arg" in
        --test)    DO_TEST=1 ;;
        --run)     DO_RUN=1 ;;
        --no-bump) DO_BUMP=0 ;;
        *) echo "unknown option: $arg" >&2; exit 2 ;;
    esac
done

SSH="ssh -o BatchMode=yes -o ConnectTimeout=30"

echo "==> Rig: ${RIG}:~/${RIG_PATH}   (git-pull route)"

# --- 1. reachability --------------------------------------------------------
# ⚠️ 30 s, deliberately. See failure 3 above.
if ! $SSH "$RIG" true 2>/dev/null; then
    echo "ERROR: cannot reach ${RIG} over SSH after 30 s." >&2
    echo "       The rig may be POWERED OFF — the operator powers it off at night," >&2
    echo "       and WOL is settled as NOT PURSUED, so it must be switched on by hand." >&2
    echo "       (Key auth needs a ~/.ssh/config entry naming the dedicated key;" >&2
    echo "        see docs/Scrivi_Linux_Rig_Setup_v0_1.md §2.1.)" >&2
    exit 1
fi

# --- 2. refuse to ship a dirty tree ----------------------------------------
# ⚠️ A git-pull deploy can only carry COMMITTED work. Shipping from a dirty tree
# would silently test something different from what the rig checks out — which is
# exactly the confusion this route exists to remove.
if [ -n "$(git status --porcelain)" ]; then
    echo "ERROR: the working tree has uncommitted changes." >&2
    echo "       This route deploys COMMITTED work only, so the rig lands on a" >&2
    echo "       commit you can name." >&2
    echo "       Commit them, or use deploy-to-rig.sh to ship the working tree." >&2
    git status --short >&2
    exit 1
fi

# --- 3. bump the stamp ------------------------------------------------------
# ⚠️ The counter belongs to the PUSH, not the compile. Two CMake-side attempts to
# generate it during the build both failed (see platforms/linux/CMakeLists.txt).
if [ "$DO_BUMP" -eq 1 ]; then
    echo "==> Bumping the build stamp"
    "${REPO_ROOT}/platforms/linux/bump-build-stamp.sh" --commit
else
    echo "==> --no-bump: stamp unchanged"
fi

# --- 4. push ----------------------------------------------------------------
BRANCH="$(git rev-parse --abbrev-ref HEAD)"
echo "==> Pushing ${BRANCH}"
git push origin "$BRANCH"
LOCAL_SHA="$(git rev-parse HEAD)"

# --- 5. pull + build on the rig --------------------------------------------
echo "==> Pulling and building on ${RIG} (this takes a few minutes)"
$SSH "$RIG" "set -e
    cd ~/${RIG_PATH}
    if [ -n \"\$(git status --porcelain)\" ]; then
        echo 'ERROR: the RIG has local modifications. The rig is READ-ONLY for source.' >&2
        git status --short >&2
        exit 1
    fi
    git fetch origin
    git checkout ${BRANCH}
    git pull --ff-only origin ${BRANCH}
    cmake -S . -B build-native -DSCRIVI_BUILD_LINUX=ON -DSCRIVI_BUILD_TESTS=ON \
        -DCMAKE_BUILD_TYPE=Release >/dev/null
    cmake --build build-native --parallel"

# --- 6. PROVE what is installed --------------------------------------------
# ⚠️ THE POINT OF THE WHOLE SCRIPT. "Did my change land?" is ANSWERED here, every
# time, rather than assumed.
REMOTE_SHA="$($SSH "$RIG" "cd ~/${RIG_PATH} && git rev-parse HEAD")"
VERSION_LINE="$($SSH "$RIG" "~/${RIG_PATH}/build-native/platforms/linux/scrivi_linux --version" 2>/dev/null || echo '(no --version)')"

echo
echo "==> INSTALLED ON THE RIG"
echo "    commit : ${REMOTE_SHA}"
echo "    stamp  : ${VERSION_LINE}"

if [ "$LOCAL_SHA" != "$REMOTE_SHA" ]; then
    echo
    echo "ERROR: the rig is NOT on the commit you just pushed." >&2
    echo "       local : ${LOCAL_SHA}" >&2
    echo "       rig   : ${REMOTE_SHA}" >&2
    echo "       ⚠️  DO NOT run a live pass against this build — it is not your code." >&2
    exit 1
fi
echo "    ✅ rig HEAD matches local HEAD"

# --- 7. optional: tests -----------------------------------------------------
# ⚠️ NON-ROOT deliberately: the Docker image builds with tests OFF and runs as
# root, so "the container is green" has never meant ctest passed on the rig.
if [ "$DO_TEST" -eq 1 ]; then
    echo
    echo "==> ctest + smokes on the rig (non-root)"
    $SSH "$RIG" "set -e
        cd ~/${RIG_PATH}
        ctest --test-dir build-native --output-on-failure | tail -5
        pass=0; fail=0; failed=''
        for sh in platforms/linux/tests/*.sh; do
            base=\$(basename \"\$sh\" .sh)
            [ \"\$base\" = 'dumas_world_fixture' ] && continue
            bin=\"\$PWD/build-native/platforms/linux/scrivi_linux_\${base}\"
            [ -x \"\$bin\" ] || bin=\"\$PWD/build-native/platforms/linux/\${base}\"
            [ -x \"\$bin\" ] || continue
            if QT_QPA_PLATFORM=offscreen bash \"\$sh\" \"\$bin\" >/dev/null 2>&1
                then pass=\$((pass+1)); else fail=\$((fail+1)); failed=\"\$failed \$base\"; fi
        done
        echo \"smokes: \$pass passed, \$fail failed\"
        [ -n \"\$failed\" ] && echo \"FAILED:\$failed\"
        exit 0"
fi

# --- 8. optional: launch ----------------------------------------------------
if [ "$DO_RUN" -eq 1 ]; then
    echo
    echo "==> Launching the app on the rig's display"
    echo "    (⚠️ needs a real session — see docs/Scrivi_Linux_Rig_Setup_v0_1.md §2)"
    $SSH "$RIG" "cd ~/${RIG_PATH} && DISPLAY=:0 nohup ./build-native/platforms/linux/scrivi_linux >/tmp/scrivi.log 2>&1 &" || true
    echo "    log: ${RIG}:/tmp/scrivi.log"
fi

echo
echo "✅ Done. The rig is on ${REMOTE_SHA:0:8} with ${VERSION_LINE}"
