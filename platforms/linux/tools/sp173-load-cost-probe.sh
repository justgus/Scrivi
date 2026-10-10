#!/usr/bin/env bash
# SP-173 Plan 6 / I-0285 AC3 — attribute the Linux load's cost, phase by phase, ON THE RIG against the real share.
# Run 1: untraced, for honest wall times. Run 2: under strace, every file call attributed to its phase and to WHAT it
# touched (scene text, scene sidecar, chapter metadata, project files, worlds, …), with the time spent inside the calls.
# ⚠️ Quit Scrivi first: the app and the probe opening the same project would distort both.
#
# Usage: platforms/linux/tools/sp173-load-cost-probe.sh <projectRoot> [appSupportRoot]
set -euo pipefail
PROJECT="${1:?usage: sp173-load-cost-probe.sh <projectRoot> [appSupportRoot]}"
APPSUP="${2:-$HOME/.local/share/Scrivi}"
REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../../.." && pwd)"
LIB=""
for cand in "$REPO_ROOT/build-native/ScriviCore/libScriviCore.a" "$REPO_ROOT/build/ScriviCore/libScriviCore.a"; do
    [ -f "$cand" ] && { LIB="$cand"; break; }
done
[ -n "$LIB" ] || { echo "⛔ libScriviCore.a not found (build it first)" >&2; exit 1; }
command -v strace >/dev/null || { echo "⛔ strace not installed" >&2; exit 1; }
BIN="${TMPDIR:-/tmp}/sp173_load_cost_probe"
TRACE="${TMPDIR:-/tmp}/sp173.trace"
g++ -O2 -std=gnu++23 -I"$REPO_ROOT/ScriviCore/include" "$REPO_ROOT/platforms/linux/tools/sp173_load_cost_probe.cpp" \
    -o "$BIN" "$LIB" -lcrypto -lpthread

echo "=== SP-173 Plan 6 — load cost on $(hostname), $(date -u +%FT%TZ) ==="
echo "project : $PROJECT"
echo "mount   : $(findmnt -T "$PROJECT" -o TARGET,FSTYPE,OPTIONS -n 2>/dev/null | cut -c1-160 || echo unknown)"
echo
echo "--- run 1: untraced (wall times) ---"
"$BIN" "$PROJECT" "$APPSUP"
echo
echo "--- run 2: strace (attribution; strace itself slows every call) ---"
strace -f -T -e trace=openat,read,write,close,newfstatat,statx,getdents64,rename,unlink,mkdir \
    -o "$TRACE" "$BIN" "$PROJECT" "$APPSUP"
python3 - "$TRACE" "$PROJECT" <<'PY'
import re, sys, collections
trace, root = sys.argv[1], sys.argv[2].rstrip('/')
phase = 'pre'
calls = collections.defaultdict(lambda: collections.Counter())      # phase -> syscall -> n
secs  = collections.defaultdict(lambda: collections.Counter())      # phase -> syscall -> seconds
cats  = collections.defaultdict(lambda: collections.Counter())      # phase -> category -> n (path calls)
enoent = collections.Counter()
fdcat = {}
def cat(p):
    if not p.startswith(root): return 'outside project'
    r = p[len(root):]
    if re.search(r'/manuscript/[^/]+/[^/]+\.md$', r): return 'scene text (.md)'
    if re.search(r'/manuscript/[^/]+/[^/]+\.meta\.json$', r): return 'scene sidecar (.meta.json)'
    if re.search(r'/manuscript/[^/]+/(chapter|_chapter)[^/]*\.json$', r): return 'chapter metadata'
    if r.startswith('/manuscript'): return 'manuscript (other)'
    if r.startswith('/worlds'): return 'worlds'
    if r.startswith('/objects'): return 'objects'
    if r.startswith('/history'): return 'history'
    return 'project files (' + (r.split('/')[1] if r.count('/') else r) + ')'
rx = re.compile(r'^\d+\s+(\w+)\((.*)\)\s+=\s+(-?\d+)(.*)<([\d.]+)>$')
for line in open(trace, errors='replace'):
    m = rx.match(line.strip())
    if not m: continue
    sc, args, ret, rest, t = m.group(1), m.group(2), int(m.group(3)), m.group(4), float(m.group(5))
    pm = re.search(r'"(/scrivi-sp173-phase/(\w+))"', args)
    if pm: phase = pm.group(2); continue
    calls[phase][sc] += 1; secs[phase][sc] += t
    path = re.search(r'"([^"]*)"', args)
    if path and sc in ('openat','newfstatat','statx','rename','unlink','mkdir'):
        c = cat(path.group(1)); cats[phase][c] += 1
        if 'ENOENT' in rest: enoent[(phase, c)] += 1
        if sc == 'openat' and ret >= 0: fdcat[ret] = c
    elif sc in ('read','write','getdents64'):
        fd = re.match(r'(\d+)', args)
        if fd: cats[phase][(sc + ' on ' + fdcat.get(int(fd.group(1)), '?'))] += 1
for ph in ('open', 'bodies', 'timeline'):
    if ph not in calls: continue
    tot = sum(secs[ph].values())
    print(f"\n== phase: {ph} — {sum(calls[ph].values())} calls, {tot:.2f} s inside them ==")
    for sc, n in calls[ph].most_common():
        print(f"   {sc:12s} {n:7d}   {secs[ph][sc]:8.2f} s")
    print("   by what they touched:")
    for c, n in cats[ph].most_common(14):
        e = enoent[(ph, c)]
        print(f"      {n:7d}  {c}" + (f"   ({e} ENOENT)" if e else ""))
PY
