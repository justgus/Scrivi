# Verified Tasks — T-0479 (SP-124, EP-038)

**Sprint:** SP-124 — `[Linux]` Drive-loss ground truth + the Linux refinement
**Epic:** [EP-038](../../Epics/Epic-active.md) — `[Linux]` The Real Hardware Rig (sprint 2 of 2)
**Verified:** 2026-09-07 — **user-approved after review** (*"mark T-0479 verified. my review is complete."*)
**Codebase:** ⚠️ **Documentation only — this Task ships NO code.**

⚠️ **This file OPENS the 0475–0479 decade.** ⚠️ **T-0475–T-0478 are NOT here**: T-0477 and T-0478 remain
open in SP-124; T-0475/T-0476 belong to earlier sprints.

---

## T-0479 — ✅ **Correct Porting Outline §9 from what the rig actually taught**

### What it delivered

✅ **`docs/Scrivi_Platform_Porting_Outline_v0_1.md` §9 was rewritten from THREE OBSERVED EVENTS** on the
Linux rig, replacing a section that had been ⚠️ **explicitly labelled a PLAN, unproven, and owing
corrections to the first Epic that executed it.**

⚠️ **§9's RULE HELD. FOUR of its PREDICTIONS DID NOT.** ✅ **Both halves are recorded** — the section was
right that only real hardware can answer these questions, and ⚠️ **wrong about most of the answers it
guessed.** ✅ **Each correction names the prediction beside the measurement**, which is what makes the
document trustworthy the next time.

| § | ⚠️ Predicted | ✅ Measured |
| - | ------------ | ----------- |
| **9.1** | *"possible stale mount point"* — ⚠️ **one behaviour** | ⚠️ **TWO, decided by WHO MOUNTED IT**: udisks2 REMOVES `/run/media/...`; a hand-mounted `/mnt/...` SURVIVES. ⚠️ **This decides whether the core's false `missing` is reachable at all** |
| **9.2** | Hangs are an ⚠️ **NFS-hard-mount** concern; `cifs` + `soft` is safe | ⚠️ **`cifs` + `soft` BLOCKS ~10 s per call** against a dead server (`EHOSTDOWN`), ⚠️ **identical under `cache=strict` and `cache=none`.** ✅ **`soft` prevents an UNKILLABLE hang, not a stall** |
| **9.3** | ⚠️ **ONE lying signal** (Apple's `volumeIsRemovable`) | ⚠️ **FIVE on Linux**: `mountpoint -q`, a directory listing (⚠️ **zeroed sizes**), `statvfs`, a transient successful `read`, and ⚠️ **a held FD that survived `umount -l` + `losetup -D`** |
| **9.4** | `offline` is the network case | ⚠️ **The network case returned `unavailable` — TWICE.** ⚠️ **`offline` has NEVER been produced on any platform** |

### ✅ The checklist grew from SIX items to EIGHT

⚠️ **Both additions were earned by LOSING something to their absence, not reasoned into existence:**

7. ⚠️ **Every command in a rig procedure is TAGGED with the machine it runs on.** ⚠️ **An untagged
   command cost an entire S3 attempt on 2026-09-07** — the probe ran on the workstation against a
   rig-only path, watched nothing, and produced noise ⚠️ **while the drive was pulled, unrepeatably.**
   ⚠️ **Also records what CANNOT cross: architectures differ (arm64 vs x86-64), so SOURCE ships and the
   RIG compiles** — ⚠️ **and `sudo` over `ssh` needs a TTY.**
8. ⚠️ **A world copy on LOCAL disk, served separately from the removable one** — ✅ **this is what made
   S2 runnable on a day when the drive was out and the rig's console was in use by someone else.**
   ⚠️ **Plus: verify the served share does not ALSO carry the project or the appSupportRoot**, or the
   kill removes those too and the run measures nothing. ✅ **Caught by the USER; it was not in the plan.**

### ⚠️ Why this Task mattered more than a documentation edit

⚠️ **Had T-0478 been written from §9 as it stood, it would have shipped:**

- ⚠️ a stale-mount defence **not needed** on the automounted path,
- ⚠️ an `EIO`/`ESTALE` handler for errors that **never arrive**,
- ⚠️ **no timeout at all**, and
- ⚠️ an `offline` branch that **never fires**.

⚠️ **Every one of those is a plausible reading of the documentation.** ✅ **That is *instrument, then
implement* vindicated by a margin** — and ⚠️ **the reason the sprint's gate was the point rather than
ceremony.**

### Verification

✅ **User review, 2026-09-07.** ⚠️ **No code, no suite** — the deliverable is the document, and ✅ **its
corrections are traceable to `Scrivi_Linux_Rig_Setup_v0_1.md` §7 and the two findings files
(`T-0477-FINDINGS-S2.md`, `T-0477-FINDINGS-S3.md`).**

⚠️ **The Windows rig EXECUTES §9. It does not re-derive it** — ⚠️ **and it should expect its OWN liars,
not inherit Linux's.**
