---
sprint: SP-140
epic: EP-040
status: Planned
platform: Apple
---

# Sprint SP-140 — `[Apple]` Stop discarding unknown `inspector-layout.json` keys ([I-0215])

**Epic:** [EP-040](../Epics/Epic-active.md) · **Status:** 🔵 **PLANNED** · **Task:** **T-0536**
**Issue:** [I-0215] · **Blocks on:** ⛔ **NOTHING** — ✅ **can run in parallel with [SP-130].**

## ⚠️ This is a LIVE defect, not preparation for the refactor

⚠️ **Apple's `InspectorLayoutDocument` is a fixed 6-property `Codable` struct**
(`InspectorLayoutStore.swift:63`). ⚠️ **Any key it does not know is DROPPED on the next save.**
✅ **Linux deliberately does the opposite** (`InspectorLayoutStore.cpp:97-100`):

> ⚠️ *"THE WHOLE DOCUMENT is kept, including every key this build does not understand … This is what
> makes the round trip lossless."*

⚠️ **CONSEQUENCE TODAY:** ⚠️ **open a project in a newer Scrivi (or on Linux, with a key Apple's build
predates), then open it on Apple** — ⛔ **Apple's next save DESTROYS the unknown keys.**
⚠️ **The user runs BOTH platforms against the same projects.**

✅ **WHY IT IS ITS OWN SPRINT:** ⚠️ **it is live data loss NOW.** ⛔ **Burying it inside [SP-141]'s
ownership refactor would delay a real fix behind a design ruling** — ⚠️ **and would make SP-141's
diff impossible to review, because a behavioural fix and an ownership move would be entangled.**

## Task

| ID | Task | Priority | Status |
| -- | ---- | -------- | ------ |
| **T-0536** | ⚠️ **Make Apple's `inspector-layout.json` round trip LOSSLESS** — ✅ **preserve keys the build does not understand, as Linux already does** | **High** | 🔵 Not started |

## Definition of Done

- [ ] ⚠️ **A key Apple does not know SURVIVES a load→mutate→save cycle.**
- [ ] ⚠️ **PROVEN BY A TEST that FAILS without the fix** — ⚠️ **not merely passes with it**
      (✅ **the [I-0214] discipline: a guarded check over absent data passes vacuously**).
- [ ] ⚠️ **Apple and Linux agree on the round trip** — ✅ **same file, both platforms, no loss.**
- [ ] ⚠️ **`xcodebuild` green for macOS, iOS AND visionOS.**

## ⚠️ Risks

| Risk | ⚠️ Mitigation |
| ---- | ------------ |
| ⚠️ **`Codable` cannot express "keep the rest"** | ⚠️ **A fixed struct structurally cannot.** ✅ **Expect to keep the raw object alongside the typed view — which is what Linux does** |
| ⚠️ **The fix is written, then [SP-141] moves the file to the core anyway** | ✅ **The core will need the SAME lossless rule.** ⚠️ **Landing it here first means SP-141 INHERITS a proven behaviour instead of inventing one** |
