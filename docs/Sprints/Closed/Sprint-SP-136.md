---
sprint: SP-136
epic: EP-040
status: Active
activated: 2026-09-22
task: T-0546
planned: 2026-09-22
platform: Apple
---

# SP-136 — ⚠️ **The Inspector as a REAL column** (`.inspector`)

**Status:** ✅ **CLOSED 2026-09-23 (user-approved).** ✅ **All 9 ACs VERIFIED by the user's live pass.**

✅ **EVIDENCE:** ⚠️ **`xcodebuild test` 132/132 in 12 suites · `ctest` 626/626 — both run 2026-09-23 AFTER the final fix, with the app quit** (⛔ [I-0150]: `xcodebuild test` LAUNCHES the app and has rewritten a real project). ✅ **AC9 live pass by the user: Inspector resize → quit → relaunch → width returned · menu AND toolbar toggle · Detail Sheet opens with the Inspector open · Esc/X dismiss · save/revert status banner appears and clears on the next keystroke.**

⚠️ **THREE ISSUES CAME OUT OF AC6/AC9, ALL FOUND BY THE LIVE PASS AND NONE BY ANY SUITE:** ✅ **[I-0245]** (the Detail Sheet's `HStack` shape made the window's widths unsatisfiable and CRASHED the app — now a real `.sheet`) — **VERIFIED**; ✅ **[I-0246]** (the sheet stays open on Save/Cancel, so a status banner now says which happened) — **VERIFIED**; ⚠️ **[I-0247]** (D1-E's NON-MODAL intent is superseded by that fix) — ⛔ **OPEN, raised deliberately for [SP-137] to rule.**

⚠️ **[I-0245] TOOK FOUR ATTEMPTS AND THE USER CORRECTED THE DIAGNOSIS TWICE.** ⛔ **A green suite (132/132 + 626/626) held through every failed attempt** — ✅ **`feedback_live_pass_finds_what_suites_cannot`, demonstrated again at full cost.**
✅ **Q1–Q3 ruled before activation.**
**Epic:** 🟡 [EP-040](../Epics/Epic-active.md) — `[Apple]` **The Editor Shell**
**Task:** **T-0546** — ✅ **ISSUED 2026-09-22 at activation.**
**Step:** **S4** of the app-shape study §4.3
**Serves:** **[EP-040] AC6**
**Blocks on:** ✅ **NOTHING.** ⚠️ **It CONTENDS with the Detail Sheet for the same trailing edge, and
the study says S6 must follow this Sprint** — ✅ **Q1 ruled that contention DEFERS to [SP-137], with
AC6 as the guard.**
**Design:** ✅ [`../Scrivi_Apple_App_Shape_Trade_Study_v0_1.md`](../Scrivi_Apple_App_Shape_Trade_Study_v0_1.md) §4.1, §4.3

---

## ✅ Goal

⚠️ **The Scene Inspector stops being an `HStack` member with a hand-rolled resize handle and becomes a
REAL trailing column** — ✅ **`.inspector(isPresented:content:)`, which macOS has provided since 14.0,
six major versions below this app's deployment target.**

⚠️ **THIS IS THE THIRD INSTANCE OF THE EPIC'S CENTRAL DEFECT:** ⛔ **a surface hand-building what the
platform already provides.** ✅ **[SP-134] did it for the toolbar, [SP-135] for the bars; ⚠️ this is
the pane itself.**

---

## ⚠️ What is actually there — measured 2026-09-22

| Piece | Today | ✅ What the platform provides |
| ----- | ----- | --------------------------- |
| The column | ⛔ **an `HStack` member** (`EditorView.swift:417`, `:421`) | ✅ **`.inspector(isPresented:)`** |
| Width | ⛔ **`.frame(width: paneWidth)`** + `@AppStorage("inspectorPaneWidth")` | ✅ **`inspectorColumnWidth(min:ideal:max:)`** |
| Resizing | ⛔ **a hand-rolled `Rectangle` + `DragGesture` + `NSCursor.resizeLeftRight` push/pop** (`SceneInspectorView.swift:142-161`) | ✅ **built in** |
| Show/hide persistence | ✅ `session.inspectorVisible` → `inspector-layout.json` | ⚠️ **`.inspector` ALSO restores presentation state — see Q2** |
| The menu command | ✅ a hand-written View-menu `Toggle` | ✅ **`InspectorCommands()`** |

✅ **THE STUDY'S ONE UNSOURCED CLAIM IS NOW ANSWERED.** ⚠️ **§4.1 says: *"whether `.inspector` exposes
resizing to the same degree was NOT sourced and must be checked before S4 is specified."***
✅ **CHECKED 2026-09-22: `inspectorColumnWidth(min:ideal:max:)` EXISTS and type-checks against the
macOS 27 SDK.** ⛔ **Not assumed from documentation — `swiftc -typecheck`d.**
✅ **`InspectorCommands()` likewise verified.**

✅ **So all three hand-built pieces have a platform equivalent, and the width range (220–560pt,
`SceneInspectorView.swift:36-37`) maps directly onto `min:`/`max:`.**

---

## ⛔ THE COUPLING THE STUDY WARNED ABOUT — ⚠️ **and it is real**

✅ **§4.4: *"(c) and S4 CONTEND FOR THE SAME TRAILING EDGE. S6 must therefore be sequenced after
S4."*** ⚠️ **Confirmed in the code:**

```swift
if session.inspectorVisible { inspector(loader: loader) }     // EditorView.swift:421
…
if showDetailSheet { Divider(); ObjectDetailSheet(…) }        // EditorView.swift:428
```

⛔ **BOTH ARE `HStack` SIBLINGS ON THE TRAILING EDGE, AND BOTH CAN BE OPEN AT ONCE.**
⚠️ **Today that "works" because they simply sit side by side and squeeze the manuscript.**
⛔ **`.inspector` is a SINGLE trailing column** — ✅ **so making the Inspector one forces the question
of what the Detail Sheet then is.**

✅ **RULED (Q1): the QUESTION defers to [SP-137]; ⚠️ the CONSEQUENCE does not.** ⛔ **AC6 requires the
sheet still open and be usable after this conversion** — ✅ **so if the two genuinely cannot coexist,
this Sprint FINDS that out and hands [SP-137] evidence instead of a guess.**

---

## Acceptance Criteria

- [x] **AC1** — ✅ **The Scene Inspector is presented with `.inspector(isPresented:)`**, ⛔ **not as an
      `HStack` member.** ✅ **[EP-040] AC6.**
- [x] **AC2** — ⚠️ **THE WRITER'S PANE WIDTH SURVIVES THE CONVERSION.** ✅ **Today it is
      `@AppStorage("inspectorPaneWidth")`, 220–560pt, and a writer who has set it MUST NOT lose it.**
      ⚠️ **See Q2: `.inspector` restores its own width, which may make the `@AppStorage` redundant —
      ⛔ but "redundant" and "silently discarded" are different outcomes and the difference is the
      writer's.**
- [x] **AC3** — ⛔ **THE HAND-ROLLED RESIZE HANDLE IS DELETED**, ✅ **with a tombstone** — ⚠️ **including
      its `NSCursor.resizeLeftRight` push/pop, which the platform now owns.**
- [x] **AC4** — ✅ **Show/hide still works from BOTH the View menu and the toolbar**, ⚠️ **and the two
      still cannot disagree** ([SP-134] AC3's guarantee). ⛔ **If `InspectorCommands()` is adopted
      (Q3), the hand-written `Toggle` must GO, not sit beside it.**
- [x] **AC5** — ✅ **The inspector's own contents are UNCHANGED**: ⚠️ **the tab bar still selects, its
      selection still persists at PROJECT level, and the [SP-135] `safeAreaBar` conversion survives.**
      ⚠️ **[SP-135] verified exactly this; ⛔ this Sprint must not quietly undo it.**
- [x] **AC6** — ⚠️ **THE DETAIL SHEET STILL OPENS AND IS USABLE**, ✅ **per the Q1 ruling.**
      ⛔ **Whatever Q1 decides, a writer must still be able to open an object and edit it when the
      Inspector is also open.**
- [x] **AC7** — ⛔ **NOTHING ELSE MOVES.** ✅ **`ManuscriptTextView`'s internals untouched; ✅ the
      Navigator and its `columnVisibility` binding untouched; ✅ the bars stay bars.**
- [x] **AC8** — ✅ **`xcodebuild` build + test clean on macOS.** ⚠️ **iOS/visionOS remain DEFERRED**
      (user ruling 2026-09-22) — ⛔ **but `EditorView` is SHARED, and the iOS branch has its own
      `inspector(loader:)` call site at `:417`.** ✅ **Do not break its compile.**
- [x] **AC9** — ⚠️ **A LIVE PASS.** ✅ **Open the Inspector, RESIZE it, quit, relaunch, confirm the
      width returned; ✅ open an object's Detail Sheet with the Inspector open; ✅ toggle the Inspector
      from the menu AND the toolbar.**

---

## ✅ THE THREE RULINGS — user, 2026-09-22

---

### ✅ Q1 — RULED: **DEFER THE DETAIL SHEET TO [SP-137]**

✅ **The Inspector converts; ⛔ the Detail Sheet is left exactly as it is.**
⚠️ **[SP-137] already exists to rule its hosting** (app-shape §4.4 lists three options and says the
decision is that Sprint's) — ⛔ **pre-empting it here would make this Sprint carry a ruling it was
not scoped for.**

⚠️ **"DEFER" IS NOT "IGNORE", AND AC6 IS THE GUARD:** ✅ **the sheet must still OPEN and be USABLE with
the Inspector open.** ⛔ **Two trailing panes and one trailing column is a real tension** — ⚠️ **if the
conversion makes the sheet unreachable, that is a finding for [SP-137], not something to paper over.**

---

### ✅ Q2 — RULED: **THE PLATFORM OWNS THE WIDTH**

✅ **`.inspector` restores its own presentation state; `inspectorColumnWidth(min:ideal:max:)` sets the
range.** ⛔ **`@AppStorage("inspectorPaneWidth")` is retired.**

⚠️ **THE STORED VALUE MUST BE MIGRATED ONCE, NOT DROPPED.** ⛔ **A writer who has dragged that pane has
a real value in `UserDefaults`, and retiring the key silently RESETS her** — ✅ **which is precisely
what AC2 forbids.** ⚠️ **Seed `ideal:` from the stored width on first run after the change.**

⚠️ **AND THE RETIREMENT IS CONDITIONAL ON THE LIVE PASS.** ✅ **If AC9 shows the platform does NOT
restore width across a relaunch, the `@AppStorage` comes back as `ideal:`** — ⛔ **the ruling is "the
platform owns it", not "trust it unverified".**

---

### ✅ Q3 — RULED: **DO NOT ADOPT `InspectorCommands()`**

⛔ **It would introduce a SECOND control path.** ✅ **Scrivi's View-menu `Toggle` and [SP-134]'s toolbar
control are already bound to the SAME `session.inspectorVisible`**, ⚠️ **so they cannot disagree** —
✅ **which is the shape [EP-040] AC2 requires, not a defect to replace.**

⚠️ **THE DISTINCTION WORTH KEEPING: a hand-written TOGGLE over one source of truth is FINE.**
⛔ **What this Epic removes is hand-built CHROME — a pane, a bar, a toolbar the platform would supply.**
✅ **A menu item bound to app state is neither.**
⚠️ **Recorded as considered and declined, so it is not re-proposed as an oversight.**

---


## Plan of work

| Step | Work | ⚠️ Note |
| ---- | ---- | ------ |
| ~~S0~~ | ✅ **DONE 2026-09-22 — all three ruled before activation** | ✅ **Gate discharged** |
| **S1** | ✅ Convert to `.inspector(isPresented:)` on the macOS branch | ⚠️ **AC1** |
| **S2** | ✅ `inspectorColumnWidth(min:220, ideal:<migrated>, max:560)`; ⚠️ **seed `ideal:` from the stored width ONCE** | ⚠️ **AC2** |
| **S3** | ⛔ Delete `resizeHandle` + its cursor handling; leave a tombstone | ⚠️ **AC3** |
| **S4** | ✅ Confirm the toolbar + menu toggles still share one source | ⚠️ **AC4** |
| **S5** | ✅ **Leave the Detail Sheet UNTOUCHED (Q1); ⚠️ verify it still opens** | ⚠️ **AC6** |
| **S6** | ✅ `xcodebuild` build + test | ⚠️ **AC8** |
| **S7** | ⚠️ **Live pass — RESIZE, quit, relaunch** | ⚠️ **AC9 — the only thing that proves AC2** |

---

## ⚠️ Risks

- ⛔ **SILENTLY LOSING THE WRITER'S PANE WIDTH.** ⚠️ **`@AppStorage("inspectorPaneWidth")` holds a real
  value for anyone who has dragged the pane** — ✅ **AC2 and Q2 exist for this**, ⛔ **and it is the
  kind of loss no test would catch.**
- ⚠️ **UNDOING [SP-135].** ✅ **The tab bar became a `safeAreaBar` one Sprint ago** — ⚠️ **inside the
  view this Sprint re-hosts.** ⛔ **AC5 exists because that was the accepted cost of [SP-135]'s Q3.**
- ⚠️ **THE DETAIL SHEET BECOMING UNREACHABLE.** ⛔ **Two trailing panes, one trailing column** —
  ✅ **AC6 requires it still work whatever Q1 rules.**
- ⚠️ **BREAKING THE iOS BRANCH.** ✅ **`EditorView` is SHARED and iOS has its own `inspector(loader:)`
  call site** — ⛔ **and those targets cannot be built from the `ScriviApp` scheme**, ⚠️ **so a
  compile error there would go unseen.** ✅ **Gate the change on `#if os(macOS)` unless ruled otherwise.**
- ⚠️ **`.inspector` behaving differently from a plain column.** ✅ **It is a PRESENTATION, not a view
  in the hierarchy** — ⛔ **so state owned by the old `HStack` member may not survive the move.**

---

## ⛔ Out of scope

- ⛔ **The Object Detail Sheet's HOSTING RULING** — ✅ **[SP-137] / S6** (⚠️ **unless Q1 rules otherwise**).
- ⛔ **`NSSplitViewController`** — ✅ **[SP-138], recorded NOT scheduled.**
- ⛔ **The Inspector's CONTENTS** — ⚠️ **cards, tabs, layout persistence all stay as they are.**
- ⛔ **Linux parity.** ✅ **Tracked as [I-0244]** — ⚠️ **not this Sprint's.**
- ⛔ **iOS / visionOS layout.** ✅ **Deferred to a future `[Apple]` Epic.**

---

## ⚠️ Estimate

✅ **MEDIUM-HIGH — the study's own rating, and the first Sprint in this Epic that REBUILDS a pane
rather than adding to one.**
⚠️ **The conversion itself is small.** ⛔ **The cost is Q1 (a coupling the study predicted), Q2 (a
writer's stored state), and the live pass** — ✅ **because AC2 cannot be proven any other way.**
