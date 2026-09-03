#!/usr/bin/env bash
# volume-loss-probe.sh — T-0477 (EP-038 / SP-124) INSTRUMENTATION ONLY.
#
# ⚠️ THIS SCRIPT SHIPS NO PRODUCT CODE AND MAKES NO DECISION. It records what
# Linux actually reports when a volume carrying a world goes away, so that
# T-0478 can be written AGAINST FINDINGS rather than against documentation.
#
#   platforms/linux/tools/volume-loss-probe.sh <mountpoint> [label]
#
# ⚠️ RUN IT BEFORE THE EVENT, NOT AFTER. It samples continuously and writes a
# timestamped log; you trigger the loss (umount / kill the share / YANK THE
# DRIVE) while it is running, then Ctrl-C it.
#
# ⚠️ WHY CONTINUOUS SAMPLING: an "after" snapshot alone cannot distinguish a
# removed mount entry from a STALE one, because a stale entry looks exactly
# like a live one until you touch it. The transition is the evidence.
#
# ⚠️ THE PROBE THAT MATTERS MOST is the HELD-OPEN FD (§4). A container cannot
# produce a stranded descriptor at all, which is precisely why Docker was blind
# to the failure mode that cost Apple six Issues.
# ⚠️ HARNESS VALIDATED 2026-08-31 on macOS, against a force-detached RAM disk —
# ⚠️ a DRY RUN OF THE PROBE, emphatically NOT a Linux finding and NOT evidence
# for T-0478. It proved the probe survives the event and records errno instead
# of dying. It also produced one result worth carrying to the rig as a QUESTION:
#
#     fd_read  = EIO        fd_write = EIO        fd_fstat = EBADF
#     fd_fsync = OK  ←⚠️ REPORTED SUCCESS ON A DEVICE THAT WAS GONE
#     path_stat / statvfs / listdir = ENOENT
#
# ⚠️ On macOS, fsync LIED. Anything using it as a liveness check would have
# concluded the volume was healthy. ⚠️ DO NOT ASSUME LINUX BEHAVES THE SAME —
# that assumption is the exact error this Epic exists to prevent. It is recorded
# only so the rig run CHECKS this specific cell rather than skimming past it.
set -uo pipefail       # ⚠️ deliberately NOT -e: every probe here is EXPECTED to
                       # fail at some point, and a failing probe is the DATA.

MOUNT="${1:-}"
LABEL="${2:-unlabelled}"
if [[ -z "$MOUNT" ]]; then
    echo "usage: $0 <mountpoint> [label]" >&2
    echo "  e.g. $0 /mnt/scriviworlds S2-network-killed-at-source" >&2
    exit 2
fi

STAMP="$(date +%Y%m%d-%H%M%S)"
OUT="${SCRIVI_PROBE_OUT:-$HOME/scrivi-probe}/${STAMP}-${LABEL}"
mkdir -p "$OUT"

echo "==> T-0477 probe"
echo "    mountpoint : $MOUNT"
echo "    scenario   : $LABEL"
echo "    output     : $OUT"
echo
echo "⚠️  TRIGGER THE LOSS NOW (umount / stop the share / PULL THE DRIVE)."
echo "⚠️  Then wait ~15s and press Ctrl-C."
echo

# --- 0. Static context, captured ONCE ------------------------------------
{
    echo "# T-0477 probe — $LABEL"
    echo "# host   : $(hostname)"
    echo "# kernel : $(uname -srvm)"
    echo "# date   : $(date -Is)"
    echo "# mount  : $MOUNT"
    echo
    echo "## findmnt (before)"
    findmnt -T "$MOUNT" -o TARGET,SOURCE,FSTYPE,OPTIONS 2>&1
    echo
    echo "## the backing device, if any"
    df -P "$MOUNT" 2>&1
    echo
    echo "## lsblk"
    lsblk -o NAME,SIZE,TYPE,MOUNTPOINT,TRAN,FSTYPE 2>&1
    echo
    echo "## ⚠️ wake-on-lan state (SP-124 asks; the rig ignored magic packets)"
    for IF in $(ls /sys/class/net | grep -v lo); do
        echo "### $IF"
        ethtool "$IF" 2>&1 | grep -i "wake\|link detected" || echo "  (ethtool unavailable)"
    done
} > "$OUT/00-context.txt" 2>&1

# --- 1. Kernel's own account, streamed -----------------------------------
# ⚠️ dmesg -w is the kernel's version of events and is the ONLY source for
# what the hardware layer saw. udevadm sees the device going away; dmesg sees
# the filesystem's reaction to it. They are different stories — keep both.
( dmesg -w             > "$OUT/01-dmesg.txt"  2>&1 ) &  DMESG_PID=$!
( udevadm monitor --udev --kernel > "$OUT/02-udev.txt" 2>&1 ) & UDEV_PID=$!
( journalctl -f -n 0   > "$OUT/03-journal.txt" 2>&1 ) & JOURNAL_PID=$!

# --- 2. The held-open FD ---------------------------------------------------
# ⚠️ THE CENTRAL PROBE. Opens a real file on the volume, then reads AND writes
# it every second across the event, recording the exact errno each returns.
#
# ⚠️ READ AND WRITE ARE SEPARATE QUESTIONS. A read may be served from page
# cache long after the device is gone — a read that SUCCEEDS post-event is a
# finding, not a non-result, because it means cached reads LIE about
# availability. The write is what actually touches the device.
FDPROBE="$OUT/fd-probe.py"
cat > "$FDPROBE" << 'PYEOF'
import errno, os, sys, time
mount = sys.argv[1]
target = os.path.join(mount, ".scrivi-fd-probe.tmp")

# Create and hold it open BEFORE the event. This is the descriptor whose fate
# we care about; opening after the fact tests nothing.
try:
    fd = os.open(target, os.O_RDWR | os.O_CREAT, 0o644)
    os.write(fd, b"scrivi-probe\n")
    os.fsync(fd)
except OSError as e:
    print(f"SETUP FAILED: {e}", flush=True)
    sys.exit(1)
print(f"held-open fd={fd} on {target}", flush=True)

def report(what, fn):
    try:
        r = fn()
        return f"{what}=OK({r})"
    except OSError as e:
        return f"{what}=ERR[{errno.errorcode.get(e.errno, e.errno)}:{e.errno}] {e.strerror}"
    except Exception as e:
        return f"{what}=EXC {type(e).__name__}: {e}"

while True:
    t = time.strftime("%H:%M:%S")
    parts = [
        # ⚠️ pread on the HELD fd — the stranded-descriptor question.
        report("fd_read",  lambda: len(os.pread(fd, 32, 0))),
        # ⚠️ pwrite + fsync — fsync is where a lost write SURFACES; a bare
        # write can succeed into cache and vanish silently.
        report("fd_write", lambda: os.pwrite(fd, b"x", 0)),
        report("fd_fsync", lambda: (os.fsync(fd), "synced")[1]),
        report("fd_fstat", lambda: os.fstat(fd).st_size),
        # ⚠️ PATH-based probes, for contrast: these re-resolve through the
        # mount table, so they can fail while the held fd still works, or the
        # reverse. Which one fails FIRST is the signal T-0478 needs.
        report("path_stat",  lambda: os.stat(target).st_size),
        report("mount_stat", lambda: os.stat(mount).st_ino),
        report("statvfs",    lambda: os.statvfs(mount).f_blocks),
        report("listdir",    lambda: len(os.listdir(mount))),
    ]
    print(f"{t} " + " | ".join(parts), flush=True)
    time.sleep(1)
PYEOF
( python3 "$FDPROBE" "$MOUNT" > "$OUT/04-fd-probe.txt" 2>&1 ) & FD_PID=$!

# --- 3. Mount-table sampling ----------------------------------------------
# ⚠️ Sampled rather than snapshotted, because THE QUESTION IS WHETHER THE
# ENTRY PERSISTS. Apple's lesson was that a stale mount point outlives an
# unclean unmount and reads as "mounted" to anything that only checks existence.
( while true; do
    {
        echo "=== $(date -Is) ==="
        echo "--- /proc/mounts (matching lines) ---"
        grep -F " $MOUNT " /proc/mounts 2>&1 || echo "(no entry)"
        echo "--- /proc/self/mountinfo (matching lines) ---"
        grep -F " $MOUNT " /proc/self/mountinfo 2>&1 || echo "(no entry)"
        echo "--- mountpoint(1) ---"
        mountpoint -q "$MOUNT" && echo "IS a mountpoint" || echo "NOT a mountpoint"
        echo "--- test -d / -r ---"
        [[ -d "$MOUNT" ]] && echo "dir: yes" || echo "dir: NO"
        [[ -r "$MOUNT" ]] && echo "readable: yes" || echo "readable: NO"
        echo
    }
    sleep 1
  done > "$OUT/05-mounttable.txt" 2>&1 ) & MT_PID=$!

# --- 4. Device-node sampling ----------------------------------------------
( while true; do
    { echo "=== $(date -Is) ==="; lsblk -o NAME,SIZE,TYPE,MOUNTPOINT,TRAN 2>&1; echo; }
    sleep 2
  done > "$OUT/06-lsblk.txt" 2>&1 ) & LSBLK_PID=$!

cleanup() {
    echo
    echo "==> stopping probes"
    kill $DMESG_PID $UDEV_PID $JOURNAL_PID $FD_PID $MT_PID $LSBLK_PID 2>/dev/null
    wait 2>/dev/null

    # ⚠️ AFTER-state, captured once the dust settles. Compared against
    # 00-context.txt, this is what a naive "check it afterwards" approach
    # would have seen — and the contrast with the streamed logs is itself a
    # finding about whether snapshot-only checking is sufficient.
    {
        echo "# AFTER-state — $(date -Is)"
        echo "## findmnt"; findmnt -T "$MOUNT" -o TARGET,SOURCE,FSTYPE,OPTIONS 2>&1
        echo; echo "## /proc/mounts"; grep -F " $MOUNT " /proc/mounts 2>&1 || echo "(no entry)"
        echo; echo "## mountpoint(1)"; mountpoint "$MOUNT" 2>&1
        echo; echo "## stat"; stat "$MOUNT" 2>&1
        echo; echo "## statvfs via df"; df -P "$MOUNT" 2>&1
        echo; echo "## ls"; ls -la "$MOUNT" 2>&1 | head -20
        echo; echo "## lsblk"; lsblk -o NAME,SIZE,TYPE,MOUNTPOINT,TRAN 2>&1
        echo; echo "## processes still holding it"; lsof "$MOUNT" 2>&1 | head -20
    } > "$OUT/07-after.txt" 2>&1

    echo "==> findings in: $OUT"
    ls -la "$OUT"
}
trap cleanup EXIT INT TERM

# Idle until interrupted; the probes above are doing the work.
while true; do sleep 1; done
