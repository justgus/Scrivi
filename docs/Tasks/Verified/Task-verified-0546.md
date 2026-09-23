# Verified Task: T-0546

| ID | Task | Sprint | Epic | Verified |
| -- | ---- | ------ | ---- | -------- |
| **T-0546** | ✅ **The Inspector as a REAL column** — `.inspector(isPresented:)` (S4) | [SP-136] | [EP-040] | **2026-09-23** |

✅ **VERIFIED by the user's live pass 2026-09-23.** ✅ **All NINE of [SP-136]'s ACs met.**

---

## ✅ What shipped

⛔ **THE INSPECTOR WAS AN `HStack` MEMBER WITH A HAND-ROLLED RESIZE HANDLE** — ⚠️ **a `Rectangle` with a
`DragGesture`, manual `NSCursor.resizeLeftRight` push/pop, and `.frame(width:)` fed by
`@AppStorage("inspectorPaneWidth")`.** ⛔ **Three pieces of platform behaviour re-implemented by hand —
the defect class [EP-040] exists to remove, after the toolbar ([SP-134]) and the bars ([SP-135]).**

✅ **`.inspector(isPresented:)` has existed since macOS 14.0** — ⚠️ **SIX major versions below this app's
deployment target, so availability was never the obstacle.** ✅ **It owns the drag, the cursor, and the
width.**

⚠️ **AC2 — THE WRITER'S WIDTH SURVIVED.** ✅ **`inspectorColumnWidth(min: 220, ideal:
SceneInspectorView.migratedIdealWidth, max: 560)`, where `ideal:` is seeded ONCE from the retired
`@AppStorage` key.** ⛔ **"Redundant" and "silently discarded" are different outcomes, and the
difference is the writer's.**

⛔ **`InspectorCommands()` DECLINED (Q3):** ✅ **it would add a SECOND control path over state that
already has one** — ⚠️ **`isPresented` binds the same `session.inspectorVisible` the View menu and the
toolbar share ([SP-134] AC3), so all three cannot disagree.**

## ⚠️ Three Issues came out of the live pass — and NONE from any suite

⛔ **`xcodebuild test` 132/132 and `ctest` 626/626 stayed GREEN through every one of them, including
FOUR failed attempts at [I-0245].**

| Issue | What it was | State |
| ----- | ----------- | ----- |
| **[I-0245]** | ⛔ **The Detail Sheet's `HStack` shape made the window's widths UNSATISFIABLE — the app CRASHED** | ✅ **VERIFIED** |
| **[I-0246]** | ⚠️ **The sheet stays open on Save/Cancel, so success had no surface** | ✅ **VERIFIED** |
| **[I-0247]** | ⚠️ **D1-E's NON-MODAL intent is superseded by the [I-0245] fix** | ⛔ **OPEN — [SP-137] to rule** |

## ⛔ [I-0245] took FOUR attempts, and the user corrected the diagnosis TWICE

⚠️ **THE CRASH:** `NSGenericException: … more Update Constraints in Window passes than there are views
in the window` — ✅ **on opening an object's Detail Sheet.**

⚠️ **THE USER NAMED THE ROOT CAUSE, not the assistant:** ✅ ***"It cannot resolve the widths because
showing the detail sheet requires that one or more views are now smaller than their minimum. It adjusts
one, which pushes the other past its minimum, it adjusts and pushes the first back again."***

⛔ **THREE WRONG FIXES, and what each got wrong:**

| # | Attempted | Why it failed |
| - | --------- | ------------- |
| **1** | ⛔ Removed a vestigial `HStack` | ⚠️ **Cosmetic. Diagnosed from the STACK ALONE while the oscillating `[SCRIVI-TL]` line sat in the SAME paste.** |
| **2** | ⛔ Deferred the Timeline's height write via `Task { @MainActor }` | ⚠️ **The Timeline was a PASSENGER — `zoom` and `shown` never moved, only `usable`.** |
| **3** | ⛔ Pinned the pane to `.frame(width: 520)` | ⚠️ **A FIXED demand is still unsatisfiable; only the jitter pattern changed.** |

✅ **THE EVIDENCE THAT SETTLED IT:** ⚠️ **the Timeline sat at its FLOOR (`usable=126`, `visible=2`) for
the entire failing run, jittering 126→134→127→130** — ⛔ **AppKit shuffling an overflow it could never
absorb. There was no space left to divide.**

✅ **FIX (user's ruling): a REAL `.sheet`.** ⛔ **Zeroing the minimums was REJECTED** — ⚠️ **it resolves
the arithmetic by letting the manuscript collapse to nothing, and re-opens the same negotiation for
every future pane.**

⚠️ **AND A SECOND CORRECTION:** ⛔ **`.interactiveDismissDisabled()` killed Esc.** ✅ **The user ruled
that wrong — *"Escape to cancel is correct, it is expected behavior and people would notice if it
didn't."*** ⚠️ **But SwiftUI's built-in Esc BYPASSES `requestClose()`, where the unsaved-changes guard
lives.** ✅ **The X button now carries `.keyboardShortcut(.cancelAction)`: Esc closes THROUGH the guard;
click-outside stays blocked.**

## ✅ How it was verified

✅ **AC9 live pass (user):** ⚠️ **Inspector resize → quit → relaunch → width RETURNED** · ✅ **toggle from
BOTH the View menu and the toolbar** · ✅ **Detail Sheet opens and is usable with the Inspector open** ·
✅ **Esc and X dismiss; the unsaved-changes prompt still fires** · ✅ **save/revert banner appears and
clears on the next keystroke.**

✅ **AC8:** ⚠️ **`xcodebuild test` 132/132 in 12 suites · `ctest` 626/626 — both run 2026-09-23 AFTER the
final fix, with the app QUIT** (⛔ [I-0150]: `xcodebuild test` LAUNCHES the app and has rewritten a real
project).

---

## ⚠️ What this Task cost, and the rule it re-proved

⛔ **A GREEN SUITE HELD THROUGH A CRASH THAT MADE THE FEATURE UNUSABLE** — ✅ **and through three wrong
fixes for it.** ⚠️ **`feedback_live_pass_finds_what_suites_cannot`, demonstrated again at full cost.**

⚠️ **The narrower lesson, recorded because it recurred:** ⛔ **the assistant twice diagnosed from a stack
trace while the disconfirming evidence was in the same paste.** ✅ **The oscillating `usable=` line said
which component was moving and which was frozen; reading it FIRST would have skipped attempts 1 and 2.**

---

*Verified 2026-09-23 by the user's live pass. [SP-136] CLOSED user-approved the same day; [EP-040] AC6 met.*
