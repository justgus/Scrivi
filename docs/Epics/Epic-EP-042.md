# EP-042 — `[Cross]` ⚠️ **Project Open Cost**

**Status:** 🟡 **ACTIVE — created 2026-09-18 (user-approved).**
**Codebase:** `[Cross]` — ⚠️ **`ScriviCore/src/manuscript/` (`SceneIndex`, `ChapterIndex`,
`ManuscriptOrderResolver`), `ScriviCore/src/objects/`, `ScriviCore/src/worlds/WorldStore.cpp`,
`platforms/linux/qml/Landing.qml`, `platforms/linux/src/EditorShell.cpp`.**
**Goal:** ⚠️ **Opening a project costs what its DATA costs, not a multiple of it** — ✅ **and no
platform layer blocks its UI thread while that cost is paid.**
**Date Created:** 2026-09-18 · **Target Close:** —

---

## ⚠️ Why this Epic exists, and why it is NOT [EP-039]

⚠️ **[EP-039] `[Cross]` Project Load Performance CLOSED 2026-09-15.** ⛔ **THIS EPIC DOES NOT REOPEN
IT** (user ruling 2026-09-18: ⚠️ *"I'm not going backwards"*). ✅ **A closed Epic keeps its record.**

⚠️ **EP-039 measured and fixed what it could SEE.** ✅ **[SP-131] took project open from `~300 s` to
`1.06 s` by adding indexes** — ⚠️ **measured on LOCAL DISK, where the page cache is absorbing the
defect this Epic exists to remove.** ⚠️ **THE READ AMPLIFICATION WAS ALWAYS THERE; it was INVISIBLE.**

✅ **WHAT MADE IT VISIBLE (2026-09-18):** ⚠️ **a user report that a REAL project
(`the-stairs-of-tintagael.scrivi`, 10 chapters / 60 scenes) opened "totally empty" on Linux from a
`cache=none` CIFS mount.** ✅ **The project was INTACT and the core's envelope was CORRECT** —
⚠️ **it simply took 6m11s, and looked like a hang for the first 2m35s.**

---

## ⚠️ The two defects, and why they are ONE Epic

| | ⚠️ **[I-0231]** `[ScriviCore]` | ⚠️ **[I-0232]** `[Linux]` |
| - | ---------------------------- | ------------------------- |
| **What** | ⚠️ **Every sidecar re-read ~96×** | ⚠️ **Landing open is SYNCHRONOUS on the UI thread** |
| **Measured** | ⚠️ **55,574 reads / 8,044 opens for 150 FILES** | ⚠️ **155 s in state `D`, one thread** |
| **Effect** | ⚠️ **Local `0.21 s` → CIFS `154 s` (~730×)** | ⚠️ **Frozen, silent launch screen before any progress UI** |
| **Fix pays out** | ✅ **Every platform, every slow volume** | ✅ **Linux; ⚠️ the PATTERN is every platform's to avoid** |

✅ **THEY ARE THE SAME EVENT SEEN FROM TWO LAYERS.** ⚠️ **[I-0231] is WHY the open is slow; [I-0232]
is why slow looks BROKEN.** ⛔ **Fixing either alone leaves a bad outcome:** ⚠️ **[I-0232] alone gives a
RESPONSIVE 6-minute open; ⚠️ [I-0231] alone leaves the UI thread blocked for whatever cost remains.**

⚠️ **ORDER MATTERS: [I-0231] FIRST.** ✅ **It is the one that makes the wait go away**, ⚠️ **and fixing
[I-0232] first would make [I-0195] LOOK verified while the underlying cost is untouched.**

---

## ⚠️ What this Epic inherits, and must not re-derive

✅ **[I-0193] built `AsyncCall` (`platforms/linux/src/AsyncCall.hpp`) — the off-the-UI-thread
machinery.** ⚠️ **[I-0232] is NOT a missing mechanism; it is an UNCONVERTED CALL SITE.**

⚠️ **[I-0195] is `RESOLVED - Not Verified` and MUST NOT BE VERIFIED UNTIL [I-0232] IS FIXED.** ✅ **Its
own status line scoped itself honestly** — ⚠️ *"Docker has NO slow mount, so responsiveness under
`cache=none` — the actual defect — is unproven until T-0501's rig pass."* ⛔ **That pass ran
2026-09-18 and the fix FAILED it.**

⚠️ **`EditorShell.cpp:63` carries an assumption this Epic FALSIFIED:** ⚠️ *"10 minutes is chosen to be
beyond any plausible honest read (the rig's slowest measured project open is seconds, not minutes)."*
✅ **A 371 s honest read is now measured.** ⚠️ **The 10-minute budget still did not fire, so it is not
itself wrong** — ⛔ **but its stated REASON is no longer true and must not be cited as evidence.**

---

## Sprints

| Sprint | Task | Title | Status | ⛔ Blocks on |
| ------ | ---- | ----- | ------ | ----------- |
| 🔵 **[SP-144]** | **T-0538** | ⚠️ **Kill the read amplification in project open; get Linux's landing open off the UI thread** | 🔵 **Planned** | ⛔ **nothing** |

⚠️ **ONE Sprint so far, deliberately.** ✅ **The measurement is in hand and the two defects are
understood** — ⛔ **but the FIX shape for [I-0231] is not yet chosen, and choosing it is [SP-144]'s
first job, not this Epic's.**

---

## Issues

| ID | Title | Severity | Sprint |
| -- | ----- | -------- | ------ |
| ⚠️ **[I-0231]** | ⚠️ **Project open re-reads every sidecar ~96× — 55,574 reads for 150 files** | **High** | ✅ **[SP-144]** |
| ⚠️ **[I-0232]** | ⚠️ **Linux's landing-flow `openProject` is synchronous on the UI thread** | **High** | ✅ **[SP-144]** |
| ⚠️ **[I-0195]** | ⚠️ **Project open blocks the UI with no progress** (`RESOLVED - Not Verified`) | **Medium** | ⚠️ **[SP-128] — ⛔ blocked from Verify by [I-0232]** |

---

## ⚠️ Out of scope, said plainly

⛔ **This Epic does NOT tune mount options.** ⚠️ **`cache=none` is the rig's DRIVE-LOSS TEST
configuration and is not a bug to configure away** — ✅ **it is the condition that makes the real
defect observable.** ⚠️ **`cache=strict` MASKS this; it does not fix it** (the [I-0195] finding,
restated because it will be tempting again).

⛔ **It does NOT redesign the world-binding model.** ⚠️ **The 188 `ENOENT` opens on an empty world
directory are IN scope as a read-count defect** — ⛔ **but what an unbound world directory MEANS is
[I-0223]'s territory, not this Epic's.**

⛔ **It does not promise a number.** ⚠️ **No target figure is stated here** — ✅ **[SP-144] sets one
from its own measurement**, ⚠️ **and a goal invented before the fix shape is chosen is a guess wearing
a target's clothes.**
