---
sprint: SP-199
epic: EP-055
status: Closed
closed: 2026-11-03
platform: Apple
created: 2026-10-31
---

# SP-199 — `[Apple]` [EP-055] **S2**: panels by size class, the iOS gestures, and the toolbar

**Status:** ✅ **CLOSED 2026-11-03** (user, 2026-11-03: *"close both sprints."*) — activated 2026-10-31. It came before [SP-198]'s iPhone and Duo passes (user, 2026-10-31: *"We shouldn't do either
one of those until we have implemented the capability to hide the Navigator and inspector and timeline."*).
**Carries:** [T-0636]–[T-0642] → `../Tasks/Verified/` (all ✅).
**Epic:** [EP-055] (AC6, AC8) → [`../Epics/Epic-active.md`](../../Epics/Epic-active.md).
**Design:** [`../Scrivi_iOS_Manuscript_Design_v0_1.md`](../../Scrivi_iOS_Manuscript_Design_v0_1.md) §8.
**Goal:** at compact width the writer sees only the Manuscript and moves Navigator ↔ Manuscript ↔ Inspector; at regular width
the panels show and hide with edge swipes; the Timeline comes up from a grabber; the Mac's toolbar buttons sit where the
system places them.

## What the code has today (read 2026-10-31)

- iOS is a two-column `NavigationSplitView` (`EditorView.swift:193`). The Inspector is an `HStack` member that is shown only when
  `userInterfaceIdiom != .phone` (`:481`), and Apple's Duo guidance says not to branch on idiom. The toolbar is a single ⋯ Actions
  menu (`:725`).
- The Mac's toolbar is SwiftUI `.toolbar` content in `ProjectWindowManager.swift` (Scene, Chapter, the Show toggles, Worlds,
  Project Settings), and its buttons call `session.*Action` closures.

## Rulings (user, 2026-10-31; design §8)

- ✅ **G1** A full-width swipe combines the edge rules: right across shows the Navigator and hides the Inspector; left across does
  the reverse.
- ✅ **G2** The Timeline uses a grabber strip above the bottom edge: swipe up on it to show, swipe down to hide.
- ✅ **G3** At compact width the Inspector is a pushed page, the third step in the navigation stack.
- ✅ **G4** Without gestures: the View menu and shortcuts, VoiceOver actions, and the ⋯ menu. No show/hide buttons on the toolbar.
- Carried over: [SP-198] R3. Branch on size class only, and put no placement code in the toolbar.

## Acceptance Criteria

- [x] AC1 — ✅ 2026-11-03 (three panels at once, user's A7 ruling) [T-0636] Width compact: one stack Navigator → Manuscript → Inspector, opening on the Manuscript, with standard back
      navigation. Width regular: the split and the `.inspector` column. Two- and three-panel versions are shown to the user, who
      chooses one. No `userInterfaceIdiom` branch remains in `EditorView`.
- [x] AC2 — ✅ 2026-11-03 (⚠️ iOS shortcuts are ⇧⌘I / ⇧⌘T, see T-0637; the grabber is tap-only, G2 revised) [T-0637] The gestures in design §8 for both size classes and the Timeline grabber, plus G4's routes that need no
      gesture. None of them takes the manuscript's scrolling, selection or the system's three-finger text gestures.
- [x] AC3 — ✅ 2026-11-03 (reordered by use on every Apple platform; New and Merge disabled on iOS until structural edits) [T-0638] The Mac's toolbar buttons (Scene, Chapter, Worlds, Project Settings) on iOS at the default placement. Each
      button is either enabled and working, or visibly disabled where its action is not wired on iOS yet.
- [x] AC4 — ✅ 2026-10-31: iOS 14/14, Mac interop 327/327. The iOS tests stay green and the Mac is unchanged (`scripts/run-interop-tests.sh`, N > 0).
- [x] AC6 — [T-0639]–[T-0642] Timeline navigation: the zoom and position come back, a zoom-to-current-scene button, a time
      scale, and no jump after a pinch. ✅ iPad and Mac (user, 2026-11-01); T-0639 and T-0642 verified. ✅ Linux built for
      T-0640 and T-0641 (2026-11-01); ➡️ rig live pass → [T-0652] ([EP-057]) (rule: `Sprint-GUIDELINES.md`, 2026-11-02).
- [x] AC5 — ✅ 2026-11-03 Live passes on the iPad, iPhone and iPhone Duo simulators, then [SP-198]'s held iPhone and Duo passes.

## Progress log

### 🟠 2026-11-03 — AC5 iOS passes 1 and 2 (iPad A, iPhone B)
- ✅ A7 ruled: **three panels at once** (AC1). ✅ A9/B5 grabber tap; a swipe is not wanted (G2 as revised).
- ✅ Re-pass (user): A5 edge swipes (any inward travel from an owned edge counts and the text does not scroll —
  `PanelSwipe.ownsEdge`/`isSwipe`), A11, A12, A13, B4, B5, B6, B7. The blank compact-width sheet did not recur.
- ✅ Toolbar (user ruled 2026-11-03): on iOS every item is `.secondaryAction` under `.toolbarRole(.editor)`; the iPad shows
  them mid-bar and moves them into the one system ⋯ as the width shrinks, restoring them as it grows (user confirmed).
- 🟠 The `openWindow` runtime warning (`ScriviApp.swift:610`): the Welcome bridge is installed on the Mac only.
- 🟠 Launch screen (user ruled option (a)): `INFOPLIST_KEY_UILaunchScreen_Generation = YES` on ScriviApp-iOS.
- 🟠 Toolbar reordered by use, ONE shape on every Apple platform (user ruled 2026-11-03): Inspector, Timeline, Settings,
  Worlds, Navigate ▾, New ▾, Merge ▾, View ▾ (World Warnings; Mac: Buffers), Close Project (`ProjectToolbar.swift`).
  Supersedes the 2026-09-22 Scene/Chapter `ControlGroup`s. Navigate is wired on iOS through the shared
  `ManuscriptSceneSync.navigateTarget`, which the Mac now uses too. iOS 19/19, Mac interop 333/333. ✅ Re-pass steps 1–6 (Mac, iPad, iPhone) passed (user, 2026-11-03).
- ✅ A1–A4, A6, A8, A10, B1–B3 passed (user, 2026-11-03). ✅ D1–D4 (Mac) passed → [I-0326], [I-0328] verified.
- ⏳ The delay before the Navigator shows — MEASURED (iPad, `[SCRIVI-NAV]`): main thread blocked 3,575 ms on the first show,
  ~1,230 ms on a later one; hiding costs 112 ms. Our row building is < 5 ms and the manuscript's layout never logged. A
  simulator probe on a copy of dumas (`NavigatorTimingProbe`, temporary) and a `sample` profile put ~75% of it in SwiftUI's
  List walking all ~2,350 rows (1,175 scenes + chapter headers) and building each row's modifiers; a bare List of the same
  rows is 12 ms. One view per row (`NavigatorRow`) cut ~95 → ~75 ms in the simulator (~20%); Mac 333/333. The real fix →
  [T-0657] (backlog, user 2026-11-03: collapsible chapters). `[SCRIVI-SWIPE]`/`[SCRIVI-NAV]` logging and the probe removed. ✅ C1–C4 on the iPhone Duo simulator (user, 2026-11-03: *"all pass! Keeping to the size classes helped immensely, the app looks great on the Duo."*). **AC1, AC2, AC3, AC5 met; T-0636–T-0638 ✅ Verified and archived.** The Navigator delay → [T-0657] (backlog).

### ➡️ 2026-11-02 — Rig pass moved to [EP-057] (T-0652)
- Under the new rule (user, 2026-11-02: *"1 and 2 approved"*; `Sprint-GUIDELINES.md`, *"The rig pass never holds a
  Sprint open"*): the rig AC is carried by T-0652, steps in [`Rig-Live-Pass-Held.md`](../Rig-Live-Pass-Held.md); the AC counts as met
  for the close. Linux evidence: the Docker ctest and smokes recorded in this log.

### 🟠 2026-11-01 — AC6: Apple passed; T-0640 and T-0641 on Linux
- User: *"All these steps passed on both iPad and the Mac."* T-0639 and T-0642 (Apple only) → ✅ Verified, archived.
- Linux: the `◎` zoom-to-current-scene button beside +/− and the tick row along the top of the strip, Apple's rules
  (`TimelinePanel.cpp`). Linux already restores zoom and position (T-0338) and has no pinch. `timeline_cluster_smoke`
  carries Apple's scale cases and a zoom-to-scene check; smokes 38/38 (Docker, Qt 6.4, offscreen, non-root).

- ⏳ Rig steps → [`Rig-Live-Pass-Held.md`](../Rig-Live-Pass-Held.md) (SP-199 AC6).

*Last Updated: 2026-11-03 (all ACs met; T-0636–T-0638 ✅ Verified; ⏳ close awaits the user.)*

*Last Updated: 2026-11-01 (AC6: Apple passed, T-0639 and T-0642 verified; T-0640 and T-0641 built on Linux, rig steps held.)*

*Last Updated: 2026-10-31 (T-0639–T-0642 filed from the live pass, 🟠; AC6 added.)*

*Last Updated: 2026-10-31 (T-0638 → 🟠; Mac 327/327, iOS 12/12.)*

*Last Updated: 2026-10-31 (created and activated; G1–G4 ruled.)*


### ✅ 2026-11-02 — [T-0640], [T-0641] → ✅ Verified (user, 2026-11-02: *"on my authority, closed all the objects that have now tested completed"*)

iPad and Mac passes (2026-11-01) and the Linux rig pass [T-0652] on the VM (Pass). AC6 is met on every platform; AC1–AC3 and AC5 (the iOS passes) keep the Sprint open.

*Last Updated: 2026-11-03 (**✅ CLOSED** (user, 2026-11-03: *"close both sprints."*).)*
