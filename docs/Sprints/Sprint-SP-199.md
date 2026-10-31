---
sprint: SP-199
epic: EP-055
status: Active
platform: Apple
created: 2026-10-31
---

# SP-199 — `[Apple]` [EP-055] **S2**: panels by size class, the iOS gestures, and the toolbar

**Status:** 🟡 **ACTIVE 2026-10-31.** It comes before [SP-198]'s iPhone and Duo passes (user, 2026-10-31: *"We shouldn't do either
one of those until we have implemented the capability to hide the Navigator and inspector and timeline."*).
**Carries:** [T-0636], [T-0637] → [`../Tasks/Task-active.md`](../Tasks/Task-active.md); [T-0638] (🟠) → [`../Tasks/Task-unverified.md`](../Tasks/Task-unverified.md).
**Epic:** [EP-055] (AC6, AC8) → [`../Epics/Epic-active.md`](../Epics/Epic-active.md).
**Design:** [`../Scrivi_iOS_Manuscript_Design_v0_1.md`](../Scrivi_iOS_Manuscript_Design_v0_1.md) §8.
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

- [ ] AC1 — [T-0636] Width compact: one stack Navigator → Manuscript → Inspector, opening on the Manuscript, with standard back
      navigation. Width regular: the split and the `.inspector` column. Two- and three-panel versions are shown to the user, who
      chooses one. No `userInterfaceIdiom` branch remains in `EditorView`.
- [ ] AC2 — [T-0637] The gestures in design §8 for both size classes and the Timeline grabber, plus G4's routes that need no
      gesture. None of them takes the manuscript's scrolling, selection or the system's three-finger text gestures.
- [ ] AC3 — ⏳ (built 2026-10-31; Scene and Chapter disabled on iOS until structural edits) [T-0638] The Mac's toolbar buttons (Scene, Chapter, Worlds, Project Settings) on iOS at the default placement. Each
      button is either enabled and working, or visibly disabled where its action is not wired on iOS yet.
- [ ] AC4 — The iOS tests stay green and the Mac is unchanged (`scripts/run-interop-tests.sh`, N > 0).
- [ ] AC5 — Live passes on the iPad, iPhone and iPhone Duo simulators, then [SP-198]'s held iPhone and Duo passes.

*Last Updated: 2026-10-31 (T-0638 → 🟠; Mac 327/327, iOS 12/12.)*

*Last Updated: 2026-10-31 (created and activated; G1–G4 ruled.)*
