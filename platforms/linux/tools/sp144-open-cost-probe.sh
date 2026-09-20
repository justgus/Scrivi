#!/usr/bin/env bash
# sp144-open-cost-probe.sh — SP-144 AC6: the ONE measurement the rig must produce.
#
# ⚠️ AC6 IS SPECIFIC AND THIS SCRIPT EXISTS SO IT IS NOT APPROXIMATED:
#     "PROVEN ON THE REAL RIG under `cache=none`, not in Docker and not on local
#      disk … Report before/after wall-clock AND read counts for the same project."
#
# ⛔ DOCKER CANNOT PRODUCE THIS. It has no slow mount, and the page cache hides
# the defect — which is exactly the gap that let [I-0195] be marked resolved
# while it was still broken. ✅ That is why this runs on the rig, against a REAL
# project, on the `cache=none` share.
#
# ⚠️ WHAT IT DOES NOT DO: it does not test the UI. Progress bars, responsiveness
# and the drive-pull assertions are a HUMAN pass — this only produces the
# syscall + wall-clock numbers AC6 asks for, which a human cannot count by eye.
#
# Usage:
#   sp144-open-cost-probe.sh <projectRoot> [appSupportRoot]
#
# Example (the rig's real project on the CIFS share):
#   platforms/linux/tools/sp144-open-cost-probe.sh \
#       /mnt/scrivi-worlds/the-stairs-of-tintagael.scrivi
set -euo pipefail

PROJECT="${1:?usage: sp144-open-cost-probe.sh <projectRoot> [appSupportRoot]}"
APPSUP="${2:-$HOME/.local/share/Scrivi}"

REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../../.." && pwd)"
PROBE_SRC="$REPO_ROOT/platforms/linux/tools/sp144_open_cost_probe.cpp"
PROBE_BIN="${TMPDIR:-/tmp}/sp144_open_cost_probe"
LIB="$REPO_ROOT/build/ScriviCore/libScriviCore.a"

[ -d "$PROJECT" ] || { echo "⛔ not a directory: $PROJECT" >&2; exit 1; }
[ -f "$LIB" ]     || { echo "⛔ missing $LIB — build ScriviCore on the rig first" >&2; exit 1; }
command -v strace >/dev/null || { echo "⛔ strace not installed" >&2; exit 1; }

echo "=== SP-144 AC6 — project open cost on the REAL rig ==="
echo "project     : $PROJECT"
echo "appSupport  : $APPSUP"

# ⚠️ REPORT THE MOUNT. A run whose mount options are not recorded cannot be
# compared against another run — and `cache=strict` MASKS this defect, so a
# measurement taken under it would be quietly meaningless.
MOUNT_LINE="$(findmnt -T "$PROJECT" -o TARGET,FSTYPE,OPTIONS -n 2>/dev/null || echo 'unknown')"
echo "mount       : $MOUNT_LINE"
case "$MOUNT_LINE" in
  *cache=none*) echo "            ✅ cache=none — this is the AC6 condition" ;;
  *)            echo "            ⚠️  NOT cache=none — AC6 requires it; this run is NOT AC6 evidence" ;;
esac
echo

g++ -O2 -std=gnu++23 -I"$REPO_ROOT/ScriviCore/include" \
    "$PROBE_SRC" -o "$PROBE_BIN" "$LIB" -lcrypto

# One run per variant. ⚠️ Each is a COLD-ish run only insofar as the mount allows;
# `cache=none` is what makes that true, which is why the mount is reported above.
run_variant() {
    local label="$1"; shift
    echo "--- $label ---"
    local t0 t1
    t0=$(date +%s.%N)
    strace -f -c -e trace=openat,read,write,rename,newfstatat,statx \
        "$PROBE_BIN" "$PROJECT" "$APPSUP" "$@" 2>"${TMPDIR:-/tmp}/sp144.strace" >/dev/null || true
    t1=$(date +%s.%N)
    awk -v a="$t0" -v b="$t1" 'BEGIN{printf "wall-clock  : %.2f s\n", b-a}'
    grep -E "^(% time|-|[0-9])" "${TMPDIR:-/tmp}/sp144.strace" | tail -9
    echo
}

# ⚠️ ORDER MATTERS FOR HONESTY: run the OLD path first. On a `cache=none` mount
# there is little cross-run caching, but running the new path first would still
# be the flattering order, and a measurement that chooses its own flattering
# order is not evidence.
run_variant "BEFORE — double open + per-scene workspace writes" --old
run_variant "AFTER  — single open handed over + bulk-load scenes"

echo "=== AC3 — the ABSENT world binding.json ==="
# ⚠️ [I-0231] measured 188 ENOENT opens of ONE absent binding on this project.
# ✅ Counting them by NAME is the only honest check: a total-call figure would
# hide it.
strace -f -e trace=openat "$PROBE_BIN" "$PROJECT" "$APPSUP" 2>&1 >/dev/null \
  | grep -c "binding.json" \
  | awk '{printf "binding.json opens across the whole load: %s\n", $1}'
echo
echo "⚠️  Paste this whole output into the SP-144 AC6 record."
