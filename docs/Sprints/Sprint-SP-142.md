---
sprint: SP-142
epic: EP-040
status: Planned
platform: Linux + ScriviCore
---

# Sprint SP-142 — `[Linux]` Retire the duplicate `InspectorLayoutStore`

**Epic:** [EP-040](../Epics/Epic-active.md) · **Status:** 🔵 **PLANNED**
**Task:** **T-0537** · **Issue:** [I-0197] (Class B, Linux half)
**Blocks on:** ⛔ **[SP-141]** — ⚠️ **Linux cannot call endpoints that do not exist yet.**

## ⚠️ Why this is a SEPARATE sprint, and why it is NOT optional

⛔ **TWO INDEPENDENT IMPLEMENTATIONS WRITE THE SAME FILE INTO THE SAME PACKAGE:**
✅ **Apple's Swift `InspectorLayoutStore` (310 lines)** and ✅ **Linux's C++
`InspectorLayoutStore.cpp` (145 lines)** — ⚠️ **with NO shared owner.**
⚠️ **That is the drift the standing rule exists to prevent, and it has ALREADY HAPPENED:**
⛔ **[I-0215] — Linux preserves unknown keys; Apple drops them.** ✅ **Same file. Two behaviours.**

⚠️ **IT IS SEPARATE BECAUSE IT IS A DIFFERENT CODEBASE ON A HARD DEPENDENCY.** ⚠️ **Bundled into
[SP-141], a slip on the Linux half would block closing a sprint whose Apple half is finished and
verified** — ⛔ **which is exactly how [SP-132] produced tasks that no Epic owned.**

⛔ **IT IS NOT OPTIONAL.** ⚠️ **Retiring Apple's store alone leaves the drift fully intact** —
✅ **the file would still have two writers.**

## Task

| ID | Task | Priority | Status |
| -- | ---- | -------- | ------ |
| **T-0537** | ⚠️ **Route Linux's `InspectorLayoutStore` through the core endpoints; ✅ DELETE the duplicate document handling** | **High** | 🔵 Not started |

## Definition of Done

- [ ] ⛔ **Linux no longer parses or writes `inspector-layout.json` itself.**
- [ ] ⚠️ **Its lossless round trip SURVIVES** — ✅ **it is the behaviour [SP-140] gave Apple and
      [SP-141] gave the core.** ⛔ **Retiring the C++ must not lose what it did RIGHT.**
- [ ] ⚠️ **A tombstone comment** where the duplicate was, ✅ **as [SP-129] left for `readImportedTimelineFile`** —
      ⛔ **so it is not reintroduced.**
- [ ] ⚠️ **Linux Docker build green + smokes pass** (⚠️ **`SCRIVI_BUILD_TESTS=OFF` in that image —
      ✅ run `ctest` on macOS**).
- [ ] ✅ **[I-0197] Class B is CLOSED here**, ⚠️ **not in [SP-141].**
