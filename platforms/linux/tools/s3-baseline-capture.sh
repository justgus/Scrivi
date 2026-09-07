#!/usr/bin/env bash
# s3-baseline-capture.sh — T-0477 (EP-038 / SP-124) S3 PRE-YANK BASELINE.
#
# ⚠️ THIS SCRIPT SHIPS NO PRODUCT CODE AND MAKES NO DECISION. It records the
# state of a LOCAL removable volume BEFORE it is physically pulled, so that
# `volume-loss-probe.sh`'s during/after streams have something to be surprising
# against.
#
#   platforms/linux/tools/s3-baseline-capture.sh <mountpoint> <projectRoot> [worldID]
#
# ⚠️ WHY THIS EXISTS SEPARATELY FROM THE PROBE. The probe samples CONTINUOUSLY
# and is started seconds before the event; it is deliberately cheap per tick.
# The baseline is the opposite: run ONCE, minutes before, and allowed to be
# slow and thorough. §4 of SP-124 requires capture BEFORE / DURING / AFTER —
# ⚠️ the probe covers during and after; NOTHING covered before.
#
# ⚠️ THE GAP THIS CLOSES. §4's table names "what `scrivi_get_world_status`
# returns at each phase" as a required probe, calling the core's verdict THE
# INPUT TO T-0478 — and no existing artifact captured it at all. Without it the
# run measures the OS and infers Scrivi's reaction, which is precisely the
# inference this Epic exists to stop.
#
# ⚠️ RUN THIS BEFORE `volume-loss-probe.sh`, NOT INSTEAD OF IT.
#
#   1. s3-baseline-capture.sh   ← once, thorough, minutes before   (THIS FILE)
#   2. volume-loss-probe.sh     ← streaming, then YANK
#
# ⚠️ FILESYSTEM IS A VARIABLE, NOT A DETAIL. SP-124 §2 already makes this point
# about `cifs` vs `nfs` failing differently; it applies to the local case too.
# ⚠️ The S3 rig drive is a FAT32 (`vfat`) partition on a drive whose OTHER
# partition remains APFS -- APFS is not mountable on Linux, so the world under
# test is a COPY on the FAT32 side. That is recorded by this script (§2) rather
# than assumed, because:
#   - ⚠️ `vfat` has NO POSIX ownership or permissions. Everything reports the
#     uid/gid/mode baked into the mount options. ⚠️ ANY permission-shaped
#     observation on this volume is an ARTEFACT OF vfat, not ground truth --
#     the same masking VirtioFS produced in the 2b container pass.
#   - ✅ The yank behaviour S3 actually wants -- stranded FDs, stale mount
#     entries, torn writes -- is BLOCK-LAYER and filesystem-agnostic, so it
#     remains valid on vfat.
# ⚠️ State this limitation in the findings; do NOT let it be discovered later.
set -uo pipefail       # ⚠️ deliberately NOT -e: a probe that fails IS the data.

MOUNT="${1:-}"
PROJECT_ROOT="${2:-}"
WORLD_ID="${3:-}"

if [[ -z "$MOUNT" || -z "$PROJECT_ROOT" ]]; then
    cat >&2 <<USAGE
usage: $0 <mountpoint> <projectRoot> [worldID]

  mountpoint  the LOCAL removable volume about to be yanked
              e.g. /media/\$USER/SCRIVI3
  projectRoot a .scrivi project whose world lives on that volume
              e.g. \$HOME/ScriviLinux/projects/the-stairs-of-tintagael.scrivi
  worldID     optional; omitted = every world the project binds

⚠️ Use a COPY of a world, never live writing work.
USAGE
    exit 2
fi

STAMP="$(date +%Y%m%d-%H%M%S)"
OUT="${SCRIVI_PROBE_OUT:-$HOME/scrivi-probe}/${STAMP}-S3-baseline"
mkdir -p "$OUT"

echo "==> T-0477 S3 baseline"
echo "    mountpoint : $MOUNT"
echo "    project    : $PROJECT_ROOT"
echo "    worldID    : ${WORLD_ID:-<all bound worlds>}"
echo "    output     : $OUT"
echo

# --- 1. Host and kernel ----------------------------------------------------
# ⚠️ Recorded per-run, not assumed from the rig doc: a kernel upgrade between
# runs changes udisks2/vfat behaviour and would otherwise be invisible.
{
    echo "# T-0477 S3 BASELINE — captured BEFORE the yank"
    echo "# host    : $(hostname)"
    echo "# kernel  : $(uname -srvm)"
    echo "# date    : $(date -Is)"
    echo "# distro  : $(. /etc/os-release 2>/dev/null && echo "$PRETTY_NAME")"
    echo "# udisks2 : $(systemctl is-active udisks2 2>&1)"
    echo "# mount   : $MOUNT"
    echo "# project : $PROJECT_ROOT"
} > "$OUT/00-host.txt" 2>&1

# --- 2. ⚠️ The filesystem under test --------------------------------------
# ⚠️ THE VARIABLE MOST LIKELY TO BE GLOSSED. Record fstype and mount options
# verbatim; `vfat`'s uid/gid/fmask come from HERE, and every ownership reading
# on this volume is downstream of them.
{
    echo "## findmnt (authoritative fstype + options)"
    findmnt -T "$MOUNT" -o TARGET,SOURCE,FSTYPE,OPTIONS,PROPAGATION 2>&1
    echo
    echo "## lsblk with filesystem detail"
    lsblk -o NAME,SIZE,TYPE,FSTYPE,LABEL,UUID,MOUNTPOINT,TRAN,RM,HOTPLUG 2>&1
    echo
    echo "## blkid — the partition table as the kernel sees it"
    # ⚠️ Shows BOTH partitions: the APFS side is expected to appear with no
    # usable TYPE, which is itself the reason the FAT32 side exists.
    sudo blkid 2>&1 || blkid 2>&1
    echo
    echo "## ⚠️ who owns this mountpoint — udisks2 or fstab?"
    # ⚠️ THE QUESTION THAT DECIDES T-0498's BRANCH. §2b: udisks2 may REMOVE the
    # mountpoint on a yank, which changes whether the false-`missing` path is
    # reached at all. A /media/<user>/<label> path is udisks2's and is likely
    # cleaned up; a hand-mounted /mnt path is not.
    case "$MOUNT" in
        /media/*) echo "PATH SHAPE: /media/* → almost certainly udisks2-managed (auto-cleanup EXPECTED)" ;;
        /mnt/*)   echo "PATH SHAPE: /mnt/*   → almost certainly hand-mounted (mountpoint should SURVIVE)" ;;
        *)        echo "PATH SHAPE: unrecognised — determine ownership by hand before drawing conclusions" ;;
    esac
    echo
    echo "### udisks2's own view"
    udisksctl status 2>&1 || echo "(udisksctl unavailable)"
    echo
    echo "### fstab entries touching this path"
    grep -F "$MOUNT" /etc/fstab 2>&1 || echo "(none — not an fstab mount)"
} > "$OUT/01-filesystem.txt" 2>&1

# --- 3. Device identity ----------------------------------------------------
# ⚠️ st_dev IS THE PRIMITIVE T-0498 IS BUILT ON. Capture it now so the
# post-yank value has a real "before" to be compared against. 2b measured this
# working on a real kernel; S3 is where it meets a physical removal.
{
    echo "## st_dev of the mountpoint, its parent, and the world package"
    echo "# ⚠️ T-0498's rule: package-absent AND same-device-as-parent → missing."
    echo "# ⚠️ A MATCH here means 'not a mount NOW' -- it does NOT prove a volume left."
    for P in "$MOUNT" "$(dirname "$MOUNT")" "$PROJECT_ROOT"; do
        printf '%-60s ' "$P"
        stat -c 'dev=%d (0x%D)  ino=%i  mode=%A  uid=%u gid=%g  fs=%T' "$P" 2>&1
        echo
    done
    echo
    echo "## ⚠️ statvfs — CAPTURED ONLY TO PROVE IT LIES LATER"
    echo "# 2b measured statvfs SUCCEEDING on an unmounted path, reporting the"
    echo "# ROOT filesystem's block counts. T-0498 RULES IT OUT as a signal."
    echo "# Recorded here so the post-yank comparison shows the lie explicitly."
    stat -f -c 'blocks=%b free=%f avail=%a fsid=%i type=%T' "$MOUNT" 2>&1
    echo
    echo "## df"
    df -PT "$MOUNT" 2>&1
} > "$OUT/02-device-identity.txt" 2>&1

# --- 4. ⚠️ THE CORE'S VERDICT — the probe §4 requires and nothing had -----
# ⚠️ This is the INPUT TO T-0478. Everything else in this capture describes
# what LINUX reports; this is what SCRIVI CONCLUDES from it. Without a healthy
# baseline here, a post-yank `missing` cannot be shown to be WRONG -- only
# different.
WORLDPROBE="$OUT/run-world-probe.sh"
cat > "$WORLDPROBE" << 'WPEOF'
#!/usr/bin/env bash
# Calls the C ABI through the built Linux binary tree. Reusable: run it again
# DURING and AFTER the yank and diff the envelopes against this baseline.
set -uo pipefail
PROJECT_ROOT="${1:-}"
WORLD_ID="${2:-}"

BIN="${SCRIVI_WORLD_PROBE_BIN:-}"
if [[ -z "$BIN" ]]; then
    for CAND in \
        "$HOME/Dev/Scrivi/build-linux/platforms/linux/scrivi_world_probe" \
        "$HOME/Dev/Scrivi/build/platforms/linux/scrivi_world_probe"; do
        [[ -x "$CAND" ]] && BIN="$CAND" && break
    done
fi

if [[ -z "$BIN" || ! -x "$BIN" ]]; then
    cat <<'NOBIN'
⚠️ NO world-status probe binary found.

⚠️ DO NOT SKIP THIS AND PROCEED. §4 names scrivi_get_world_status as a required
probe and calls the core's verdict THE INPUT TO T-0478. A run without it
measures the OS and INFERS Scrivi's reaction -- the exact inference this Epic
exists to prevent.

✅ Two ways to satisfy it, either is fine:
  (a) Run the Qt app against the project and read Manage Worlds… at each phase
      (SP-127 shipped that surface); screenshot it. ⚠️ Note I-0182: an
      unavailable object may name itself by raw objectID.
  (b) Build a 20-line C harness against libScriviCore.a calling
      scrivi_list_worlds + scrivi_get_world_status, and point
      SCRIVI_WORLD_PROBE_BIN at it.
NOBIN
    exit 3
fi

echo "# scrivi_get_world_status via $BIN"
echo "# project : $PROJECT_ROOT"
echo "# date    : $(date -Is)"
echo
"$BIN" "$PROJECT_ROOT" ${WORLD_ID:+"$WORLD_ID"} 2>&1
WPEOF
chmod +x "$WORLDPROBE"
"$WORLDPROBE" "$PROJECT_ROOT" "$WORLD_ID" > "$OUT/03-world-status-BEFORE.txt" 2>&1
WORLD_RC=$?

# --- 5. The binding, read from local disk ---------------------------------
# ⚠️ binding.json lives INSIDE THE PROJECT, on LOCAL disk -- NOT on the volume
# being pulled. It therefore survives the yank intact, which is what makes
# I-0182 a defect rather than a limitation: the cached displayNames are still
# readable when the world is gone.
{
    echo "## binding.json files under the project (LOCAL — survive the yank)"
    find "$PROJECT_ROOT/worlds" -name 'binding.json' 2>/dev/null | while read -r B; do
        echo "=== $B"
        cat "$B" 2>&1
        echo
    done || echo "(no worlds/ directory)"
} > "$OUT/04-binding.txt" 2>&1

# --- 6. Volume contents + checksums ---------------------------------------
# ⚠️ THE TORN-WRITE ORACLE. Without per-file checksums taken BEFORE, a partial
# write found afterwards cannot be distinguished from a file that was always
# that way. §5's optional torn-write test is unfalsifiable without this.
{
    echo "## tree"
    find "$MOUNT" -maxdepth 4 -not -path '*/.Spotlight*' -not -path '*/.fseventsd*' 2>&1 | head -200
    echo
    echo "## ⚠️ checksums of every file on the volume (the torn-write oracle)"
    find "$MOUNT" -type f -not -path '*/.Spotlight*' -not -path '*/.fseventsd*' \
        -exec md5sum {} \; 2>&1 | sort -k2
} > "$OUT/05-contents.txt" 2>&1

# --- 7. Anything already holding the volume -------------------------------
{
    echo "## lsof"
    lsof "$MOUNT" 2>&1 | head -40 || echo "(lsof unavailable / nothing open)"
    echo
    echo "## fuser"
    fuser -vm "$MOUNT" 2>&1 || echo "(nothing)"
    echo
    echo "## ⚠️ is the Scrivi app running? It MUST be, for the stranded-FD case"
    pgrep -a scrivi 2>&1 || echo "(scrivi not running — see the checklist)"
} > "$OUT/06-holders.txt" 2>&1

# --- 8. Kernel ring buffer, pre-event -------------------------------------
# ⚠️ So the post-yank dmesg can be diffed rather than read whole.
dmesg 2>&1 | tail -50 > "$OUT/07-dmesg-before.txt"

# --- Summary ---------------------------------------------------------------
echo
echo "==> baseline captured: $OUT"
echo
FSTYPE="$(findmnt -T "$MOUNT" -no FSTYPE 2>/dev/null)"
echo "    filesystem : ${FSTYPE:-UNKNOWN}"
echo "    mount path : $MOUNT"
case "$MOUNT" in
    /media/*) echo "    ⚠️ udisks2-managed path — EXPECT the mountpoint to be REMOVED on yank" ;;
    /mnt/*)   echo "    ⚠️ hand-mounted path — EXPECT the mountpoint to SURVIVE the yank" ;;
esac
if [[ $WORLD_RC -ne 0 ]]; then
    echo
    echo "    ⚠️ WORLD STATUS NOT CAPTURED (rc=$WORLD_RC) — see 03-world-status-BEFORE.txt"
    echo "    ⚠️ §4 REQUIRES it. Satisfy it before yanking, or the run is incomplete."
fi
cat <<'NEXT'

==> ⚠️ BEFORE YOU PULL — the checklist

  [ ] The world on the volume is a COPY, never live writing work
  [ ] 03-world-status-BEFORE.txt shows a HEALTHY status (or (a)/(b) is done)
  [ ] The Scrivi app is RUNNING with the project open  ← the stranded FD
  [ ] 05-contents.txt lists the files you expect
  [ ] volume-loss-probe.sh is started and streaming

==> then, in a second terminal:

  platforms/linux/tools/volume-loss-probe.sh <mountpoint> S3-physical-yank

==> ⚠️ PULL THE DRIVE BY HAND. Do NOT eject first — a clean eject is S1.
==> Wait ~15s, Ctrl-C the probe, then re-run the world-status probe:

  <baseline-dir>/run-world-probe.sh <projectRoot> [worldID]

⚠️ The BEFORE/AFTER diff of that envelope is T-0478's actual input.
NEXT
