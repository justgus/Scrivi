# T-0547 · T-0548 — ✅ VERIFIED 2026-09-23

| Field | Value |
| ----- | ----- |
| **Sprint** | ✅ **[SP-137]** (CLOSED 2026-09-23) |
| **Epic** | **[EP-040]** `[Apple]` **The Editor Shell** |
| **Status** | ✅ **VERIFIED — user live pass 2026-09-23** |

## 🟢 SP-137 — T-0547 · T-0548 (EP-040) — ✅ **IMPLEMENTED 2026-09-23, NOT VERIFIED**

✅ **Both Tasks COMPLETE** → [`../Sprints/Sprint-SP-137.md`](../Sprints/Sprint-SP-137.md).
✅ **All EIGHT ACs met; AC2 and AC8 USER-VERIFIED 2026-09-23.**
⚠️ **T-0548 shipped BOTH halves, ⛔ but the runtime assertion was REMOVED by user ruling** ([I-0249]) —
✅ **the static CI guard remains.**
✅ **`xcodebuild test` 134/134 · `ctest` 626/626 · boundary + TextKit + layout guards all green.**

⚠️ **TWO THINGS THE IMPLEMENTATION FOUND THAT NO SUITE WOULD HAVE:**
⛔ **The debug assertion's first version could NEVER have fired** — ⚠️ **it tested
`window.contentView === self`, but AppKit drives the pass from `NSThemeFrame` (the content view's
SUPERVIEW).** ✅ **A probe caught it; `superview == nil` is the correct test.** ⚠️ **It built clean,
installed clean, and logged nothing.**
⛔ **TWO `DerivedData` directories exist** — ⚠️ **the first probe run launched an HOUR-STALE binary,
which presents exactly like "the code is not reached".**

### ✅ **T-0547 — Real sheet chrome for the Object Detail Sheet** (AC1–AC5) — **IMPLEMENTED**

⚠️ **The hand-built `toolbar` `HStack` in `ObjectDetailSheet.swift` is window chrome inside a content
pane.** ✅ **Back/forward and close become REAL `.toolbar` items, with a tombstone naming the
replacement.** ⛔ **Save and Cancel do NOT move** — ⚠️ **they are document actions, and Q2 ruled they
stay content-level.**

⚠️ **THE TRAP (AC2):** ⛔ **if the close control moves to a toolbar,
`.keyboardShortcut(.cancelAction)` MUST move with it and `.interactiveDismissDisabled()` MUST stay on
the host** — ✅ **otherwise Esc silently bypasses `requestClose()` and the unsaved-changes prompt
stops firing** ([I-0245]). ⚠️ **Verify with UNSAVED EDITS present.**

### ✅ **T-0548 — A layout-convergence guard for the [I-0245] class** (AC6) — **IMPLEMENTED**

✅ **Q4 RULED 2026-09-23 (user): (iii) + (i) — BOTH halves, neither alone.**
✅ **(iii) STATIC GUARD** — ⚠️ **a `scripts/` check that fails if a flexible-width view is added as an
`HStack` sibling in `manuscriptDetail`.** ✅ **Runs in CI on every push** ([SP-149] precedent);
⚠️ **it guards the SHAPE, not the property.**
✅ **(i) DEBUG ASSERTION** — ⚠️ **counts Update-Constraints passes per window and traps past a sane
bound.** ✅ **Catches the general case on any surface;** ⛔ **debug-only, so CI never sees it.**
⛔ **(ii) a headless layout test is DECLINED** — ⚠️ **SwiftUI layout under test is fragile and least
likely to reproduce the defect.**

⚠️ **AC6 APPLIES TO EACH HALF: both must be proven by INJECTING the [I-0245] shape and watching them
go red / trap.** ⛔ **A green test that cannot fail is not evidence** — ⚠️ **[I-0245] took FOUR
attempts with `xcodebuild test` 134/134 and `ctest` 626/626 GREEN throughout.**


---

## ⚠️ What this Task cost, recorded because the number matters

⛔ **T-0547 WAS IMPLEMENTED FOUR TIMES.** ✅ **Each version built clean and passed every suite.**

1. ⛔ **`.toolbar` on a bare sheet** — rendered NOTHING ([I-0248]). ⚠️ Took `.cancelAction` with it, so
   Esc discarded edits SILENTLY — ✅ **the exact trap AC2 was written to prevent**, marked met by
   READING the code.
2. ⛔ **A hand-built `VStack` bar** — ⚠️ it rendered, ⛔ but hand-built chrome inside the Epic that
   exists to remove it. ✅ **The user caught it.**
3. ⛔ **A `NavigationStack` whose bar was still populated entirely by hand** — ⚠️ the stack was present
   and never allowed to DO anything.
4. ✅ **The stack owns navigation** — ⚠️ push/pop, title, chevron, transitions all the platform's.

✅ **THE USER'S RULINGS ARE WHAT ENDED IT**, each dissolving a problem rather than solving it:
⚠️ *"the nav stack owns the toolbar"* · *"put them where they are supposed to go"* · *"the confirm
action is the close action because it dismisses the sheet"* · ⚠️ **and the one that unblocked the
impossible part:** *"This confirmation dialog shouldn't attempt to 'Cancel' the navigation … rather it
should require either a save or a revert."* ✅ **No SwiftUI hook can arrest a pop; a SETTLED pop needs
no interception.**

## ✅ T-0548 — built in full, HALF retired

✅ **Both halves shipped and both were PROVEN RED against an injected defect.**
⛔ **The runtime assertion was REMOVED by user ruling** ([I-0249]) — ⚠️ it fired twice on real work,
MISDIAGNOSED both times, and turned a recoverable layout renegotiation into a CRASH.
✅ **`scripts/check-layout-convergence.sh` remains and runs in Apple CI on every push.**
