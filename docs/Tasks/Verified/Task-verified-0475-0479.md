# Verified Tasks — T-0478, T-0479 (SP-124, EP-038)

**Sprint:** SP-124 — `[Linux]` Drive-loss ground truth + the Linux refinement
**Epic:** [EP-038](../../Epics/Epic-active.md) — `[Linux]` The Real Hardware Rig (sprint 2 of 2)
**Verified:** 2026-09-07 — **user-approved after review** (*"mark T-0479 verified. my review is complete."*)
**Codebase:** ⚠️ **Documentation only — this Task ships NO code.**

⚠️ **This file OPENS the 0475–0479 decade.** ✅ **T-0478 was ADDED 2026-09-10** (verified after its two
rig-found defects were fixed). ⚠️ **T-0475–T-0477 are NOT here**: T-0477 remains open in SP-124;
T-0475/T-0476 belong to earlier sprints.

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


---

## T-0478 — ✅ **`WorldVolumeStatus` for Linux**

**Verified:** 2026-09-10 — **user-approved** (*"I'm willing to call the Task verified."*)
**Codebase:** `[Linux]` + `[ScriviCore]`

### What it delivered

✅ **`unmounted` / `offline` / `missing` for Linux, written AGAINST T-0477's measured findings rather than
from documentation.** ⚠️ **The gate was the POINT of the sprint** — ⚠️ **had this been written from the
Porting Outline §9 as it stood, it would have shipped a stale-mount defence that is not needed, an
`EIO`/`ESTALE` handler for errors that never arrive, no timeout at all, and an `offline` branch that
never fires.**

✅ **`offline` / `hostUnreachable` EMITTED FOR THE FIRST TIME IN THIS PROJECT'S HISTORY**, from a real
killed `cifs` share on the rig, reproducibly, at BOTH endpoints (`scrivi_list_worlds` and
`scrivi_get_world_status`). ⚠️ **Before this, `offline` was a documented lie** — an enum value no code
path could produce. ✅ **It now rests on a positive `EHOSTDOWN` (112)** — ⚠️ **the strongest signal
T-0477 found UNUSED**, against ⚠️ **FIVE signals T-0477 proved LIE** (`mountpoint -q`, a directory
listing with zeroed sizes, `statvfs`, a transiently successful `read`, and a held FD that survived
`umount -l` + `losetup -D`).

### ⚠️ **What the live pass BROKE — and why that is the Task's most valuable output**

⚠️ **The 2026-09-08 live pass PROVED the core and CONDEMNED the feature in the same session.** ✅ **Two
defects were filed against this Task's OWN output, by its OWN pass, neither findable from any suite:**

| | ⚠️ Defect | ✅ Disposition |
| - | -------- | ------------- |
| **[I-0193]** | ⚠️ **A scene click froze the app ~102 s → Force Quit** — ⚠️ **for a writing application that is DATA LOSS, not latency** | ✅ **VERIFIED + ARCHIVED 2026-09-10** |
| **[I-0194]** | ⚠️ **`lastKnownPackagePath` returned a six-`../` traversal, WRITER-FACING** | ✅ **RESOLVED `e978360`** |

⚠️ **[I-0193] took TWO passes to close**, ✅ **and the reason is worth keeping**: build 34 fixed the
scene-click path and proved it on the rig, ⚠️ **but left the SECOND call site the Issue had NAMED FROM
THE START untouched**, so `Project > Manage Worlds` froze identically. ⚠️ **An Issue that names two call
sites is not closed by fixing one, however well the first is proven.**

✅ **The sharpest fix was in the ERROR HANDLER itself:** `EditorShell::writerFacingError()` called
`listWorlds()` synchronously to recover a world's NAME — ⚠️ **a branch that runs ONLY when a world is
already known unusable, so it hit the dead volume EVERY time it executed**, ⚠️ **blocking ~102 s while
composing the sentence explaining that very failure.** ⚠️ **The error handler froze the app to explain
the error.** ✅ **The name never needed the volume: it is cached from every successful read.**

### ⚠️ Residual risk, recorded rather than hidden

⚠️ **The OFFLINE branches of BOTH fixes are UNPROVEN BY ANY SUITE**, and this is structural, not
laziness: ⚠️ **[I-0194]'s fallback cannot be staged in-process** (`weakly_canonical` calls
`std::filesystem` DIRECTLY, bypassing the injectable `FileSystem`, and SUCCEEDS on a merely non-existent
path — `ec=0`), ⚠️ **and [I-0193]'s timeout needs a real blocking mount** (Docker has no dead network
share). ✅ **Both were verified the only way they CAN be — from the writer's seat on real hardware**
(`feedback_live_pass_finds_what_suites_cannot`).

⚠️ **`AsyncCall`'s "parked, not leaked" claim depends on the `cifs` mount being `soft`** — ✅ **confirmed
in `/proc/mounts`** — ⚠️ **and a `hard` mount, the cifs DEFAULT, would invalidate it.**

### ✅ **Two DoD items settled by USER RULING, both CLOSED rather than deferred**

⚠️ **WOL (NIC vs firmware): NOT PURSUED.** ✅ **WOL is impossible on this rig by OPERATING PRACTICE, not
hardware** — ⚠️ **the operator powers the machine OFF at night, so no NIC remains to answer a magic
packet.** ✅ **Consequence to plan around: `ssh` timing out is the EXPECTED state, not a fault to
diagnose.**

⚠️ **"Every inconclusive branch returns the core's status": SATISFIED.** ✅ **The writer-facing messages
already distinguish the two claims correctly** — an unreachable world is reported as unreachable
(*"Nothing has been lost."*), ⚠️ **never as damaged.** ✅ **The user's rule is SHARPER than the DoD line
and supersedes it:** ⚠️ **`corrupt`/`damaged` is a DETERMINISTIC claim, made ONLY when the world CAN BE
REACHED and something EXPECTED is missing or something UNEXPECTED is present.**

⚠️ **A CLAIM MADE DURING THIS TASK IS WITHDRAWN:** ⚠️ **[I-0183] and [I-0192] were cited as instances of
that DoD item. ⚠️ THEY ARE NOT** — ✅ **both were wrong-status defects inside the core's RESOLUTION
logic, both fixed, and neither was the app telling a writer their world was corrupt.** ⚠️ **What remains
of that class belongs to [I-0181]/T-0498.**
