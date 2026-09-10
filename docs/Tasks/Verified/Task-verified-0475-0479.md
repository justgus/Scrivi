# Verified Tasks — T-0477, T-0478, T-0479 (SP-124, EP-038)

**Sprint:** SP-124 — `[Linux]` Drive-loss ground truth + the Linux refinement
**Epic:** [EP-038](../../Epics/Epic-active.md) — `[Linux]` The Real Hardware Rig (sprint 2 of 2)
**Verified:** 2026-09-07 — **user-approved after review** (*"mark T-0479 verified. my review is complete."*)
**Codebase:** ⚠️ **Documentation only — this Task ships NO code.**

⚠️ **This file OPENS the 0475–0479 decade.** ✅ **T-0478 and T-0477 were ADDED 2026-09-10.**
⚠️ **T-0475/T-0476 are NOT here** — they belong to earlier sprints. ✅ **SP-124's Tasks are now all
verified** (T-0477, T-0478, T-0479 here; ⚠️ **T-0498 archives to the 0495–0499 decade**).

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


---

## T-0477 — ✅ **Drive-loss instrumentation (S1 / S2 / S3)**

**Verified:** 2026-09-10 — **user-approved**
**Codebase:** ⚠️ **FINDINGS ONLY — this Task ships NO code.**

### What it delivered

✅ **THREE loss scenarios OBSERVED ON REAL HARDWARE**, widened from one by user ruling at activation —
⚠️ **and the widening was load-bearing**: **S1** a clean `umount`, ⚠️ **S2 a `cifs` share killed AT THE
SOURCE** (full streamed capture, two passes, 3,110 lines), ⚠️ **S3 the physical USB yank.**
⚠️ **S2 and S3 do NOT substitute for each other** — ✅ **a clean unmount cannot strand an FD, and a
stranded FD is the state that cost Apple six Issues.**

✅ **THE HEADLINE FINDING IS THAT THE OBVIOUS SIGNALS LIE — FIVE OF THEM**, which is the Linux
counterpart to Apple's `volumeIsRemovable == false` on a hand-unplugged drive:

| ⚠️ Signal | ⚠️ The lie |
| -------- | --------- |
| `mountpoint -q` | ⚠️ reports mounted after the device is gone |
| a directory listing | ⚠️ succeeds, with **zeroed sizes** |
| `statvfs` | ⚠️ **SUCCEEDS on an unmounted path**, reporting the ROOT filesystem's block counts — ⚠️ **a confident success with a plausible number** |
| a successful `read` | ⚠️ transiently correct |
| ⚠️ **a held FD** | ⚠️ **survived `umount -l` + `losetup -D` ENTIRELY** |

✅ **`st_dev` works** (⚠️ **with a stated limit — see T-0498**); ✅ **`EHOSTDOWN` (112) was the strongest
UNUSED signal** — ⚠️ **and T-0478 then built `offline` on it.**

✅ **S3 PRODUCED THE FINDING THAT NARROWED T-0498:** ⚠️ **udisks2 REMOVES the mountpoint it created**, so
`/run/media/<user>/<label>` — ✅ **what a real writer's automounted drive uses** — ⚠️ **never reaches the
defective branch at all.** ✅ **The `/mnt` half was RULED not worth running** (a hand-made directory is
an ordinary directory; nothing has a mandate to delete it) — ⚠️ **recorded as a REASONED RULING, not an
untested branch.**

### ⚠️ **How S3 was gathered — and the process defect it exposed**

⚠️ **THE INSTRUMENTATION DID NOT PRODUCE THE S3 FINDING. THE USER DID.** ⚠️ **`volume-loss-probe.sh` ran
on the WRONG MACHINE** — on the MacBook, pointed at a Linux path that does not exist there — ⚠️ **so it
watched nothing and its output was discarded.**

⚠️ **THE CAUSE WAS MY INSTRUCTIONS, NOT THE USER'S EXECUTION.** ⚠️ **The runbook never said WHICH MACHINE
each command belonged to**, ⚠️ **and it told the user to build a binary that did not exist in the
repository yet, on a box with no dev environment.** ⚠️ **`s3-baseline-capture.sh` then redirected its own
failure into a FILE instead of stdout**, ✅ **so the run appeared to proceed while its most important
probe had already failed.** ✅ **The machine-discipline table in `T-0477-FINDINGS-S3.md` §0 is the fix.**

### ✅ **`before/during/after` RETIRED as a PHANTOM REQUIREMENT** — user ruling, 2026-09-10

⚠️ **The DoD required BEFORE/DURING/AFTER capture for every scenario. ✅ It is RETIRED — it cannot be
accurately measured for a PHYSICAL YANK:** ✅ **the event is instantaneous and operator-driven**, ⚠️ **and
the during-window is exactly when the machine is least able to report on itself.** ⚠️ **S3 WILL NOT BE
RE-RUN.**

✅ **THE LESSON THAT SURVIVES IS ABOUT EVIDENCE, NOT CADENCE:** ⚠️ **a finding must state HOW it was
gathered** — ✅ **"the user watched the screen" is legitimate evidence, and here it was STRONGER than
instrumented output pointed at the wrong machine.** ⚠️ **Do not re-add this checkbox to a future
platform's rig work** (rig doc §7.8).

### ⚠️ Open questions, recorded as UNKNOWNS blocking nothing (rig doc §7.8)

⚠️ **The "network off / black hole" S2 variant** (only a REFUSED connection was run — ⚠️ **and that
already blocks 102 s**); ⚠️ **NFS** (`cifs` only — ⚠️ **an NFS `hard` mount is the known hazard, and
`soft` is the load-bearing premise of `AsyncCall`'s "parked, not leaked" claim**); ⚠️ **a held FD across
a yank**; ⚠️ **torn writes**; ⚠️ **whether S3's successful read DECAYS or PERSISTS.**
