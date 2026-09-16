---
sprint: SP-143
epic: EP-040
status: Planned
platform: Cross
---

# Sprint SP-143 — `[Cross]` The regression guard ([I-0197] closure)

**Epic:** [EP-040](../Epics/Epic-active.md) · **Status:** 🔵 **PLANNED**
**Task:** **T-0510** · **Blocks on:** ⛔ **[SP-141] AND [SP-142]** — ⚠️ **both, not either.**

## ⚠️ WHY THIS GOES LAST — the reason it is not folded into SP-130

⚠️ **The guard must FAIL when Swift touches a project-package path.** ⚠️ **But TODAY, Swift touching
`inspector-layout.json` is LEGITIMATE** — ✅ **it is how the feature works until [SP-141] lands.**

| phase | ⚠️ allow-list must contain |
| ----- | ------------------------- |
| **Before [SP-141]** | ⚠️ **`InspectorLayoutStore` (3 sites)** + the 2 permanent non-violations + T-0508's ruled sites |
| **After [SP-141]/[SP-142]** | ⛔ **`InspectorLayoutStore` entries REMOVED** |

⛔ **A GUARD WRITTEN EARLY WOULD PASS GREEN ACROSS THE VERY CHANGE IT EXISTS TO POLICE.**
⚠️ **That is not a guard; it is a rubber stamp.** ✅ **Written last, it FAILS until BOTH owners are
gone** — ⚠️ **which is what makes splitting [SP-141]/[SP-142] safe: the drift cannot quietly survive.**

## Task

| ID | Task | Priority | Status |
| -- | ---- | -------- | ------ |
| **T-0510** | ⚠️ **A test or CI grep that FAILS when Swift touches a project-package path directly**, ✅ **with an explicit allow-list carrying each exception's RECORDED REASON** | **Medium** | 🔵 Not started |

## Definition of Done

- [ ] ⚠️ **The guard FAILS on a deliberately-introduced violation** — ✅ **PROVEN by introducing one,
      ⛔ not assumed** (the [I-0214] discipline).
- [ ] ⚠️ **Every allow-list entry carries its REASON**, ✅ **traceable to the sprint that ruled it.**
- [ ] ⚠️ **It runs where it will actually be seen** — ⛔ **a guard nobody runs is not a guard.**
- [ ] ✅ **[I-0197] is CLOSED.**

## ⚠️ Risk

| Risk | ⚠️ Mitigation |
| ---- | ------------ |
| ⚠️ **The allow-list becomes a dumping ground** | ⚠️ **An entry without a reason is a silent exception.** ✅ **DoD requires each be traceable to a ruling** |
| ⚠️ **A grep-based guard is brittle** | ✅ **Accepted: it is a TRIPWIRE, not a type system.** ⚠️ **Its job is to make the class LOUD when it returns, not to prove absence** |
