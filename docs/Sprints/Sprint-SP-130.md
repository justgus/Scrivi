---
sprint: SP-130
epic: EP-040
status: Active
platform: Apple
---

# Sprint SP-130 — `[Apple]` Rule the asset presence-checks (Class C of [I-0197])

**Epic:** [EP-040](../Epics/Epic-active.md) · **Activated:** 2026-09-15 · **Status:** 🟡 **ACTIVE**
**Task:** **T-0508** · **Issue:** [I-0197] (Class C only)

⚠️ **RESTRUCTURED 2026-09-15 (user-ruled).** ✅ **SP-130 was ONE sprint of four tasks; ⚠️ it is now
FIVE sprints** — ✅ **see [`Sprint-SP-130-RESTRUCTURE.md`](Sprint-SP-130-RESTRUCTURE.md) for why.**
✅ **This sprint keeps ONLY T-0508**, ⚠️ **the one piece that blocks on nothing and decides nothing.**

## ⛔ T-0509 IS STRUCK — its remainder is provably ZERO

⚠️ **T-0509 was "sweep the remaining hits in the 11 files", written 2026-09-10 when the count was 35.**
✅ **RE-MEASURED 2026-09-15: 10 hits, 6 files, and they partition EXACTLY:**

| bucket | n |
| ------ | - |
| T-0507 — `InspectorLayoutStore` (Class B) | 3 |
| T-0508 — `fileExists` (Class C) | **5** |
| Recorded non-violations | 2 |
| ⛔ **T-0509 remainder** | **0** |

⚠️ **SP-129 and prior work consumed the sweep.** ⛔ **T-0509 is STRUCK, not deferred** — ✅ **there is
no work behind it.**

## T-0508 — the five Class C sites

| file:line | ⚠️ context |
| --------- | ---------- |
| `ObjectCard.swift:980,984` | ✅ **ALREADY inside `Task.detached(.utility)`** — ⛔ **NOT on the UI thread** |
| `ObjectImageSection.swift:106,137` | ⚠️ **In a view builder** (`preview(_:)`) — ✅ **already comments that a missing file means an UNAVAILABLE WORLD, not damage** |
| `ExistingAssetPicker.swift:130` | ⚠️ **In a `@ViewBuilder` thumbnail** — ✅ **already comments that a failed load is EXPECTED, not corruption** |

⚠️ **THE PLANNING TEXT MISJUDGED THIS.** ⚠️ **It warned these are "the BLOCKING-STAT pattern that cost
[I-0193] 102 s".** ✅ **Two are already off-thread and three already degrade gracefully with the
reasoning written in-line.** ⚠️ **So this is mostly a RULING TO RECORD, not a defect to fix** — ⛔ **but
the ruling must be WRITTEN, because "whatever the code happens to do" is how it drifts.**

## Definition of Done

- [ ] ⚠️ **The Class C ruling is WRITTEN DOWN** — ✅ **for each of the 5 sites: legitimate UI
      presence-check, or a core question?** ⛔ **Not left implicit.**
- [ ] ⚠️ **Whatever is ruled MUST NOT block the UI thread** ([I-0193] cost 102 s).
- [ ] ⚠️ **If any site changes, `xcodebuild` green for macOS, iOS AND visionOS** (⚠️ **separate
      schemes: `ScriviApp`, `ScriviApp-iOS`, `ScriviApp-visionOS`**).

⛔ **NO GUARD IN THIS SPRINT.** ✅ **T-0510 is [SP-143]** — ⚠️ **a guard written now would have to
allow-list `InspectorLayoutStore`, then be EDITED when [SP-141] removes it.** ⚠️ **It would pass green
across the very change it exists to police.**
