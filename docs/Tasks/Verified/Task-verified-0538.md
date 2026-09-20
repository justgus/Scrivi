# T-0538 — `[Cross]` Project open cost ✅ **VERIFIED 2026-09-20**

**Sprint:** [SP-144] · **Epic:** [EP-042] · **Codebase:** `[ScriviCore]` + `[Linux]` + `[Apple]`
**Verified by:** the user, live pass on 🐧 `Oathkeeper` (build 45/46), real project on a `cache=none` CIFS share.

---

## ✅ What was asked

⚠️ **Kill the read amplification in `scrivi_open_project` ([I-0231]) and get
Linux's landing open off the UI thread ([I-0232]).** ⛔ **The fix SHAPE was
deliberately NOT chosen up front** — ✅ **AC1 was to LOCATE the amplification by
measurement, not by reading.**

## ✅ What was delivered

| | |
| - | - |
| ✅ **[I-0231]** | `util::ReadThroughCache` (scoped to one open, write-invalidating) + a shared `BindingCache` in `repairDangling` |
| ✅ **[I-0232]** | `openProjectAsync` (AC4) + the landing hands its envelope to `EditorShell` (AC5) |
| ✅ **[I-0233]** | Apple now calls `scrivi_close_project` from `ProjectSession.close()` |
| ✅ **[I-0234]** | `scrivi_open_scene_for_bulk_load` — writes went O(scenes) → O(1) |
| ✅ **[I-0235]** | The scroll-offset regression this Task itself introduced, caught by the user's live pass |

## ✅ Measured on the real rig, `cache=none`

| | ⚠️ BEFORE | ✅ AFTER |
| - | --------- | -------- |
| Wall-clock | ⚠️ **24.01 s** | ✅ **13.46 s** (−44%) |
| Syscalls | ⚠️ **5,002** | ✅ **2,999** (−40%) |
| `binding.json` opens | ⚠️ **188** | ✅ **2** |

---

## ⚠️ THE PREMISE DID NOT SURVIVE CONTACT — ✅ **the part worth keeping**

⚠️ **This Task was planned as a READ problem.** ✅ **The read amplification was
real and was fixed.** ⛔ **But it was not the whole answer, and the Task would
have closed believing it was.**

✅ **The user's observation is what broke it open**: ⚠️ *"none of the projects
I've loaded lately have been large"* — ⛔ **which falsified "it is slow because
the manuscript is big"** and forced a measurement of the FULL app load rather
than just `openProject`. ⚠️ **That measurement found the dominant cost was a
per-scene WRITE of `workspace-state.json` ([I-0234]) — 61 atomic
read-modify-write cycles to record a value only the last of which survives.**

⚠️ **The manuscript was never the problem**: every sidecar was opened just 3×.

## ⚠️ Two platform drifts, in opposite directions

⚠️ **Linux opened the project TWICE where Apple opened it once** ([I-0232]) —
✅ **`ProjectSession.loadAsync` had always had the right shape.**
⚠️ **Apple LEAKED a `ProjectIndex` per project where Linux released it**
([I-0233]) — ⚠️ **and [T-0512] had been marked ✅ Verified claiming both platforms
were wired, when only Linux was.**

✅ **Both produced a STANDING RULE (user, 2026-09-20): Linux must ALWAYS adopt
Apple's shape, and a shape change on Apple must be made the same way on Linux, in
the same work.**

## ⚠️ A regression this Task introduced, and how it was caught

⚠️ **[I-0235]**: removing `openScene`'s implicit surface stamp was safe on Apple
(`stampWritingSurfaceBlocking` already existed) ⛔ **but not on Linux, which saved
only DIRTY scenes.** ⚠️ **The replacement then delegated to `saveScene`, which
zeroes the scroll when the caret is elsewhere** — ✅ **correct for a background
flush, wrong for a stamp whose entire purpose is the writer who scrolled and never
typed.**

⛔ **Invisible to 617 green tests.** ✅ **Found by a human scrolling without
typing, quitting, and reopening** — ⚠️ **the check this Sprint wrote down
precisely because no suite performs it.**
