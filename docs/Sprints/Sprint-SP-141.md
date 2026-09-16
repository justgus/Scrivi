---
sprint: SP-141
epic: EP-040
status: Planned
platform: Apple + ScriviCore
---

# Sprint SP-141 — `[Apple]`+`[ScriviCore]` Core-owned `inspector-layout.json` — endpoints + Apple adoption

**Epic:** [EP-040](../Epics/Epic-active.md) · **Status:** 🔵 **PLANNED — ⛔ BLOCKED ON A RULING**
**Task:** **T-0507** · **Issue:** [I-0197] (Class B)
**Blocks on:** ⚠️ **an ENDPOINT-SHAPE RULING (below)** · ✅ **[SP-140] should land first** (lossless
round trip becomes the core's contract, not a thing to reinvent).

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
