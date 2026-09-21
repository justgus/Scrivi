---
sprint: SP-141
epic: EP-041
status: Complete
task: T-0507
verified: 2026-09-21
platform: Apple + ScriviCore
---

# Sprint SP-141 — `[Apple]`+`[ScriviCore]` Core-owned `inspector-layout.json` — endpoints + Apple adoption

**Epic:** [EP-041](../Epics/Epic-EP-041.md) · **Status:** 🟠 **COMPLETE 2026-09-21 — ⛔ awaiting user
approval to close.**
**Task:** **T-0507** ✅ **VERIFIED 2026-09-21** → [`../Tasks/Verified/Task-verified-0507.md`](../Tasks/Verified/Task-verified-0507.md)
**Issue:** [I-0197] (Class B — ⚠️ **HALF closed; Linux's duplicate awaits [SP-142]**)

---

## ✅ OUTCOME — 2026-09-21

✅ **BOTH RULINGS WERE MADE AND THE SPRINT DELIVERED.**
⚠️ **Everything below this line is the PRE-IMPLEMENTATION PLAN**, ⛔ **including a "blocked on a
ruling" status that is no longer true and an `epic: EP-040` front-matter that was stale from the
2026-09-18 split.** ✅ **Kept for its reasoning, which is why the plan is not deleted.**

| Ruling | Date | ✅ Outcome |
| ------ | ---- | --------- |
| **Endpoint shape** | 2026-09-18 | ✅ **ONE OPAQUE DOCUMENT GET/PUT** |
| **Absence semantics** | 2026-09-21 | ✅ **"core reports, app decides"** — `ok` \| `absent` \| `unreadable` |

⚠️ **THE PLAN BELOW ASKS "WHICH OF FOURTEEN MUTATORS?" — ✅ AND THE OPAQUE RULING DISSOLVED THAT
QUESTION ENTIRELY.** ✅ **TWO endpoints serve all fourteen**, ⚠️ **because the core never interprets
the document and therefore does not need one entry point per property.** ✅ **That is the clearest
evidence the shape ruling was the right one.**

✅ **Shipped:** `scrivi_get_inspector_layout` / `scrivi_put_inspector_layout`; `JsonDoc::isObject()`;
`ScriviEngine.getInspectorLayout` / `.putInspectorLayout`; `InspectorLayoutStore.swift` converted with
⛔ **ZERO direct file I/O left.**
✅ **Evidence:** `ctest` **621/621** (was 613 — 8 new ABI tests), `xcodebuild test` **132/132**,
⚠️ **and the new tests PROVEN FAILING against an injected [I-0215]-shaped defect.**
✅ **User live pass 2026-09-21: all checks pass.**

---

## ⛔ THE RULING THIS SPRINT CANNOT START WITHOUT

⚠️ **`InspectorLayoutStore` has FOURTEEN mutators** — `setSelectedTab`, `setInspectorHidden`,
`setSort`, `setEntries`, `addCard`, `removeCard`, `moveCards`, `setCollapsed`,
`applyStackToAllScenes`, `removeScene`, … ⚠️ **"Add core endpoints" does not say WHICH.**

| option | ✅ for | ⚠️ against |
| ------ | ----- | --------- |
| **A — 14 endpoints, one per mutator** | ✅ **The core owns the RULES, not just the bytes; both platforms get identical semantics for free** | ⚠️ **Chatty across the ABI; every future card operation is an ABI change.** ⛔ **Six mutators call `save()` SYNCHRONOUSLY — including drag-reorder — so this puts a C-ABI crossing inside a drag gesture** |
| **B — 2 endpoints (`get`/`set` document)** | ✅ **Small ABI; the core owns the FILE, its validation and its repair; platforms keep their own mutation logic** | ⚠️ **The core owns bytes, NOT semantics** — ⚠️ **the two platforms can still drift on what a mutation MEANS** |

⚠️ **RECOMMENDATION: B**, ⚠️ **but it is the user's ruling, not mine.**
⛔ **DO NOT PICK ONE SILENTLY TO GET STARTED** — ⚠️ **that is exactly what [T-0504]/[T-0505] were
deferred to avoid.**

## ⚠️ What makes this bigger than its line count

⚠️ **`InspectorLayoutStore` is NOT a file reader — it is LIVE OBSERVABLE UI STATE.** ✅ **It is
`@Bindable`, injected into `SceneInspectorView` and `InspectorCardStackView`.** ⚠️ **Moving ownership
means deciding what the VIEWS bind to.**

⚠️ **SIX mutators call `save()` SYNCHRONOUSLY** — ⚠️ **including `moveCards` (drag-reorder).**
⛔ **Routing every save through the ABI without addressing cadence trades an architectural defect for
a STUTTER** — ⚠️ **the same trade [I-0193] made at 102 s.**

⚠️ **THE FILE IS ALREADY ON DISK IN REAL PROJECTS**, ✅ **carrying `"schema": "scrivi.inspector-layout.v1"`.**
⛔ **[I-0214] IS FOUR DAYS OLD: a stricter reader made the user's data VANISH.** ⚠️ **This sprint MUST
rule what happens to a file with a MISSING or UNKNOWN schema value — ⛔ before writing the reader.**

## Task

| ID | Task | Priority | Status |
| -- | ---- | -------- | ------ |
| **T-0507** | ⚠️ **CLASS B — core endpoints for `inspector-layout.json`; route APPLE through them.** ⛔ **Read the EXISTING format — no migration** | **High** | 🔵 Not started — ⛔ **needs the ruling** |

⛔ **LINUX IS NOT IN THIS SPRINT.** ✅ **Retiring its duplicate is [SP-142]** — ⚠️ **it cannot call
endpoints that do not exist yet.**

## Definition of Done

- [ ] ⚠️ **The endpoint-shape ruling is RECORDED** with its reasoning.
- [ ] ⚠️ **The missing/unknown-schema behaviour is RULED and TESTED** ([I-0214]'s lesson).
- [ ] ⚠️ **An EXISTING file — the user's real one — loads unchanged.** ⛔ **No migration.**
- [ ] ⚠️ **Drag-reorder is MEASURED before and after** — ⛔ **no new stutter.**
- [ ] ⚠️ **`ctest` + `xcodebuild` green for macOS, iOS AND visionOS.**
- [ ] ⚠️ **A LIVE PASS.** ✅ **[SP-129] proved a green suite does not mean usable.**

⚠️ **NOT DONE UNTIL [SP-142] LANDS:** ⛔ **two owners of one file is the defect.** ⚠️ **This sprint
closing does NOT close [I-0197] Class B.**
