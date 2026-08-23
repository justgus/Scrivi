# Active Sprint

**No Sprint is currently active.**

**SP-118 was ✅ CLOSED 2026-08-23 (user-approved)** → [`Closed/Sprint-SP-118.md`](Closed/Sprint-SP-118.md).
**EP-034 remains 🟡 Active** — SP-119 is next.

---

## What SP-118 delivered

EP-034's **fourth** sprint. **All five Tasks and thirteen Issues ✅ Verified.**

| Task | Delivered |
| ---- | --------- |
| T-0441 | ⚠️ **T-0416** — seeded relation types reconciled on open; ⚠️ **complete only with I-0149** |
| T-0442 | **R3** — the related-objects section, labels **read** from the core and never recomputed |
| T-0443 | **D4-A** — inline creation; ⚠️ **a second picker, RULED IN FAVOUR by the user** |
| T-0444 | **R5/R7** — double-click **and** right-click → **"Show"** |
| T-0445 | **Pending far-endpoints** — listed, named, explained, never dropped |

**Closes AC5, AC6 and AC7.**

**Suites at close:** `ctest` **561/561** (arm64 · x86-64 · ASan+UBSan) · **Linux 565/565** (GCC 13,
non-root) · interop **120/120** · app **BUILD SUCCEEDED**.

---

## ⚠️ The number that defines this sprint: 13 Issues, 0 found by tests

Every one of **I-0149 – I-0161** was found by the user's **live click-through**, after the suites were
green. ⚠️ **The suites were not wrong** — they assert edge creation, duplicate rejection, both-endpoint
visibility and pending presentation, and all of it held up. **What no test covered was whether a writer
could reach any of it.**

⚠️ **Third occurrence of `capability_without_surface` in this Epic** — and the first at the **core**
(I-0149) rather than in the UI.

### ⚠️ Four of the thirteen were ONE failure: a rule already written in this repo

| Issue | The rule that already existed |
| ----- | ----------------------------- |
| I-0152 | the Navigator names an untitled scene "Scene N", never an ID |
| I-0155 | `ObjectCardModel.rename` re-reads before patching |
| I-0157 | I-0132 ruled `selectedSceneID` the source of truth |
| I-0158 | `SceneNavigatorView` uses `List(selection:)` |

> ⚠️ *"A Swift standard List View handles all this automatically, which makes me wonder why it is so hard
> for you."* — the user. It was not hard; new machinery was built beside machinery that already worked.

### ⚠️ Standing guidance recorded at close — SwiftUI events are asynchronous

> *"Many event detection operations do not occur in the 'intuitive' place in the codebase… they are run
> asynchronously between the time they are triggered and the time they are actually executed. We need to
> make sure we target the right callback function."* — the user

⚠️ **The tell is "it works, but the other half of the UI doesn't agree."** Almost never focus, gesture
arbitration or rendering — it is two views driven from different points in one async sequence.

---

## ⚠️ Two data-safety findings that outlive this sprint

1. ⚠️ **`xcodebuild test` is NOT read-only on this project (I-0150).** It launches the hosted app, which
   restores the writer's real projects from bookmarks and writes to them with just-compiled code — this is
   what silently repaired `the-twisted-remains-of-myself.scrivi` while Scrivi was closed. Fixed
   structurally. ⚠️ **`pgrep Scrivi` never protected against this and reads as though it does.**
2. ⚠️ **A scroll region with no affordance is a data-loss report waiting to happen (I-0159).** Myton
   appeared to show 3 of 8 relationships. **Whether rows are absent or merely unreachable-looking, the
   writer concludes her work is gone.**

---

## ⚠️ Carried out of SP-118 — do not read as delivered

| Item | Owner |
| ---- | ----- |
| ⚠️ **`tags` deferred** (user ruling) — **R2 not fully met, AC2 CANNOT CLOSE** | **SP-119** |
| ⚠️ **AC9's second half** — a world going away *while a sheet is open* is a DIFFERENT trigger from the one verified (`feedback_verify_each_half_separately`) | **SP-119** |
| **AC3, AC4, AC8** — images and sources | **SP-119 – SP-120** |
| **I-0147** — the 60 s post-crash lock window; ✅ **Accepted**, regression-tested | **Network-worlds design** |
| ⚠️ **`ObjectIndex::loadWorldIndex`'s rebuild is still UNLOCKED** — `WorldLock` is not reentrant | **Network-worlds design** |

---

## Next

⚠️ **No Sprint is planned.** SP-119 has not been defined — ⚠️ **Claude does not plan or activate one
without the user's word.**

Its likely scope, carried from here: **`tags` (closing AC2)**, **AC9's second half**, and **AC3/AC4
images**.

⚠️ **SP-107–SP-114 remain RESERVED to EP-032** and are not available.

**Next available:** Sprint **SP-119** · Task **T-0446** · Issue **I-0162**.

---

*Last Updated: 2026-08-23 (**SP-118 ✅ CLOSED, user-approved.** Five Tasks + thirteen Issues Verified and
archived in the same step. ⚠️ **Thirteen Issues, none found by any suite** — the strongest evidence yet
that the live click-through is not optional. Active Sprints 1 → 0; EP-034 stays 🟡 Active, **4 of 8
sprints closed**.)*
