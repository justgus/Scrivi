# T-0477 — PRELIMINARY container findings

**EP-038 / SP-124 · 2026-08-31 · ⚠️ NOT the T-0477 deliverable**

---

## ⚠️ READ THIS FIRST — what these findings are NOT

⚠️ **These were produced in a privileged Docker container on a loopback ext4 filesystem, NOT on the
rig and NOT from a drive pull.** ⚠️ **They do NOT close AC4, AC5, or any part of T-0477**, and
⚠️ **T-0478 is still HARD-GATED on the real session.**

⚠️ **The container is STRUCTURALLY BLIND to the failure mode this Epic exists for** (Epic §1). It was
used here for a NARROWER question it CAN answer honestly:

> ✅ **How does a Linux kernel report MOUNT STATE through the path API?**

⚠️ **That question is about kernel semantics, not hardware**, which is why a container is admissible
for it. ⚠️ **Every hardware question — stranded FDs on a real yank, stale mount entries, torn writes,
udev traffic — remains UNANSWERED and belongs to the real session.**

---

## ✅ FINDING 1 — ⚠️ **`statvfs` LIES, and it is the most dangerous signal on Linux**

⚠️ **Measured**, mount → unmount, same path:

| Probe | Mounted | ⚠️ **After unmount** | |
| ----- | ------- | ------------------- | - |
| `exists(mountpoint)` | `True` | ⚠️ **`True`** | ⚠️ **LIES** — the mountpoint DIRECTORY survives |
| `isdir(mountpoint)` | `True` | ⚠️ **`True`** | ⚠️ **LIES** |
| `listdir(mountpoint)` | `[World.scrivworld, …]` | `[]` | ⚠️ **Empty ≠ absent.** Indistinguishable from an empty dir |
| **`statvfs().f_blocks`** | `14325` | ⚠️ **`15300561` — SUCCEEDS** | ⚠️ **LIES WORST** |
| `exists(package)` | `True` | `False` | ✅ Useful, but see Finding 3 |
| **`st_dev` vs parent's** | ⚠️ **DIFFERENT** (1792 vs 63) | **SAME** (63 vs 63) | ✅ **WORKS** |
| **`/proc/mounts` entry** | `1` | `0` | ✅ **WORKS** |

⚠️ **`statvfs` DOES NOT FAIL on an unmounted path. It silently answers about the ROOT filesystem**,
because the mountpoint reverts to an ordinary directory on the parent fs.

⚠️ **THIS IS LINUX'S `volumeIsRemovable`.** ⚠️ **It is worse than Apple's**, because Apple's lie was a
wrong boolean while this one is a CONFIDENT SUCCESS carrying a plausible number. ⚠️ **Anything asking
"how much space is on the world's volume" gets an answer about the SYSTEM DISK and cannot tell.**

## ✅ FINDING 2 — ⚠️ **`st_dev` comparison is the signal that works**

✅ **A mounted filesystem has a DIFFERENT `st_dev` from its parent directory. An unmounted one has the
SAME.** ⚠️ **This is exactly what `mountpoint(1)` does internally**, and it agreed in every trial.

⚠️ **CAVEAT, and it matters:** a directory that NEVER held a mount is ALSO `st_dev == parent`.
⚠️ **So `st_dev` proves "not a mount point RIGHT NOW" — it does NOT prove "a volume went away."**
⚠️ **Distinguishing those two needs the BINDING's record of what was expected**, not the filesystem.
✅ **T-0478 must not treat `st_dev` alone as evidence of `unmounted`.**

## ⚠️ FINDING 3 — a held-open FD survives `umount -l` ENTIRELY

⚠️ **Measured across `umount -l` + `losetup -D`:**

```
fd_read=OK | fd_write=OK | fd_fsync=OK | fd_fstat=OK   ← for the FULL 8s after
path_stat=ENOENT                                        ← the PATH broke immediately
```

⚠️ **The descriptor kept reading AND WRITING SUCCESSFULLY after the mount was gone and the loop device
destroyed.** ✅ **Correct Linux semantics** — the inode lives while a descriptor holds it.

⚠️ **DO NOT GENERALISE THIS TO A PHYSICAL YANK.** ⚠️ **`umount -l` is an ORDERLY detach; the hardware
is still present.** ⚠️ **On a real pull the backing store is GONE and `EIO`/`ESTALE` are expected** —
⚠️ **but that is a PREDICTION, and predicting is what this Epic forbids.** ✅ **S3 must measure it.**

⚠️ **What this DOES establish:** ⚠️ **an open FD and a path can disagree completely.** ⚠️ **An app
holding a manuscript open may see NO error at all while every path probe fails** — so
⚠️ **"the writer's editor still works" is NOT evidence the volume is present.**

## ⚠️ FINDING 4 — ⚠️ **THE BIG ONE: an unmounted Linux volume is reported `missing`, not `unavailable`**

⚠️ **This is the defect I-0115 was written about, re-earned on Linux through a DIFFERENT trigger.**

**The core's rule** (`WorldStore.cpp:330-348`): package absent **AND** parent directory exists →
`WorldStatus::missing`.

⚠️ **An unmounted Linux volume satisfies BOTH.** ⚠️ **Measured:**

| State | package exists | parent exists | ⚠️ **core reports** |
| ----- | -------------- | ------------- | ------------------ |
| Mounted | `True` | `True` | ✅ `available` |
| ⚠️ **Unmounted, mountpoint dir SURVIVES** | `False` | ⚠️ **`True`** | ⚠️ **`missing` — WRONG** |
| Unmounted, mountpoint dir also removed | `False` | `False` | ✅ `unavailable` |

### ⚠️ CORRECTED 2026-08-31 (user) — ⚠️ **this is an AUTOMOUNTER POLICY difference, NOT an OS semantic**

⚠️ **An earlier draft of this finding claimed "macOS removes the mountpoint, Linux keeps it" as an OS
difference. THAT WAS WRONG**, and the user identified why: ⚠️ **a mountpoint is just a directory on
both platforms**, and `mount` takes an arbitrary one.

✅ **MEASURED on macOS with a HAND-SPECIFIED mountpoint** (a RAM disk mounted to a normal directory
rather than letting the system pick `/Volumes/<name>`):

| After unmount | Result |
| ------------- | ------ |
| directory exists | ⚠️ **TRUE** |
| contents | ⚠️ **the PRE-MOUNT contents reappear** (`README.txt`) |
| `st_dev` vs parent | ⚠️ **SAME** |
| `statvfs` | ⚠️ **SUCCEEDS**, reporting the enclosing fs |

⚠️ **IDENTICAL TO LINUX.** ✅ **So the real difference is narrow: `diskarbitrationd` cleans up the
`/Volumes/<name>` entries IT created; udisks2 does not always clean up `/media/<user>/<label>`.**
⚠️ **That is a policy of the automounter, not a property of the OS** — ⚠️ **and it therefore may NOT
hold for a hand-mounted path on EITHER platform.**

### ⚠️ What this means for I-0181 — ⚠️ **the risk is WIDER, not narrower**

⚠️ **Apple is NOT structurally immune.** ⚠️ **A world on a hand-mounted volume could hit the same false
`missing` on macOS**, and ⚠️ **the reason it has not been seen is that the real rig uses `/Volumes/`
auto-mounts, where the cleanup masks it.**

✅ **The user's other point is the fix's foundation:** ⚠️ **"what is missing from those directories are
the specific indicators that an actual device is mounted there"** — ✅ **and `stat` supplies exactly
that.** ⚠️ **Directory EXISTENCE is not the signal; `st_dev` vs the parent IS** (Finding 2).
⚠️ **The core's `exists(parent)` test is asking the wrong question on BOTH platforms.**

⚠️ **Consequence for the writer:** `missing` is the ONE status that invites destructive remedies —
restoring from backup, clearing references — ⚠️ **against a world that is perfectly intact on a drive
she merely unplugged.** ⚠️ **Doc 2 §7.2.1: "a wrong `missing` is materially worse than an honest
`unavailable`."**

⚠️ **Whether `/media/<user>/<label>` survives a PHYSICAL yank (as opposed to the clean `umount`
measured here) is exactly what S3 must determine** — ⚠️ **udisks2 may remove it, which would change
which branch fires.** ✅ **Filed as I-0181 so it is not lost, and it is NOT fixed from container
evidence.**

---

## ✅ CONFIRMED ON THE RIG (2026-08-31) — ⚠️ **partially**

⚠️ **The rig came up mid-session and the kernel-semantics findings were re-checked THERE**, on
⚠️ **Linux 7.0.0-30-generic** — ⚠️ **a different kernel and different filesystems from the container's
Ubuntu 24.04 / ext4-on-loopback.** ✅ **Same conclusions, independently reproduced:**

| Path | `st_dev` vs parent | `statvfs` | `mountpoint(1)` |
| ---- | ------------------ | --------- | --------------- |
| `/boot/efi` (**real vfat mount**) | ⚠️ **DIFFERENT** | succeeds (274658) | `True` |
| `/snap/firefox/8763` (**squashfs mount**) | ⚠️ **DIFFERENT** | succeeds (2083) | `True` |
| `/home/justgus` (**plain directory**) | **SAME** | ⚠️ **SUCCEEDS (239804694)** | `False` |

✅ **Findings 1 and 2 hold on the real rig kernel** — ⚠️ **`statvfs` succeeds on a NON-MOUNT and reports
the enclosing filesystem**, and ✅ **`st_dev`-vs-parent separates the cases cleanly** across two
unrelated filesystem types.

⚠️ **STILL NOT ESTABLISHED, and still owed by the physical session:** ⚠️ **everything that requires a
volume to GO AWAY.** ✅ **The above tests mount-state DETECTION on volumes that are present;**
⚠️ **it says NOTHING about stranded FDs, stale mountpoints after a yank, torn writes, or udev traffic.**

### ⚠️ Why S1/S2 could NOT be run despite the rig being reachable

| Blocker | Detail |
| ------- | ------ |
| ⚠️ **`sudo` requires a password** | ⚠️ **Mounting is inherently privileged.** ✅ **`gio`/gvfs offers an UNPRIVILEGED SMB mount and gvfs-daemon starts fine** — ⚠️ **but a gvfs FUSE mount is NOT a kernel `cifs` mount and would answer a DIFFERENT question** |
| ⚠️ **No SMB server on the LAN** | ⚠️ **The Mac has `445` listening but ZERO share points**; the Windows box and two other hosts refuse `445`. ⚠️ **Enabling File Sharing is the USER's call** |
| ⚠️ **`cifs-utils` / `nfs-common` NOT installed** | ⚠️ **Installing needs `sudo`** |

✅ **`ctest` 571/571 non-root on the rig, 2026-08-31** — the suite is green and the build (Aug 30
16:35) matches SP-126's completion, so ⚠️ **no stale-binary risk** (`feedback_confirm_the_build_under_test`).

⚠️ **The rig had been UP for 1 day 13 hours.** ⚠️ **It was never asleep — it was OFF THE NETWORK.**
⚠️ **The §1 "the box sleeps" heuristic MISREAD this**, and ⚠️ **wake-on-LAN was a red herring for a
machine that was already awake.** ✅ **Worth correcting in the rig doc (T-0479).**

---

## ⚠️ What remains OWED by the real session

| Question | Status |
| -------- | ------ |
| ⚠️ **Stranded-FD errno on a PHYSICAL yank** (`EIO`? `ESTALE`?) | ⚠️ **UNANSWERED — S3** |
| ⚠️ **Does a STALE `/proc/mounts` entry survive an unclean pull?** | ⚠️ **UNANSWERED — S3.** ⚠️ **The clean case removed it; the unclean case is the question** |
| ⚠️ **Network share killed at source — error or HANG?** | ⚠️ **UNANSWERED — S2** |
| ⚠️ **Torn writes** | ⚠️ **UNANSWERED — S3** |
| ⚠️ **udev/dmesg traffic on removal** | ⚠️ **UNANSWERED — container has none** |
| ⚠️ **Does `fsync` lie on Linux as it did on macOS?** | ⚠️ **NOT YET SEEN** — it never failed here because the FD stayed valid |

⚠️ **Reproduce with** `platforms/linux/tools/volume-loss-probe.sh`; the container scripts live in the
session scratchpad and are NOT tracked — ✅ **the rig is the source of record.**
