# Active Tasks

*Last Updated: 2026-10-31 (**T-0636, T-0637, T-0638 filed → active** with SP-199's activation (EP-055 S2).)*

## 🔵 T-0636 — `[Apple]` EP-055 S2: panels by size class — the compact stack and the regular split

**Epic:** [EP-055] (AC6, AC8) · **Status:** 🔵 **Active** · **Sprint:** [SP-199] (🟡 Active) → [`../Sprints/Sprint-SP-199.md`](../Sprints/Sprint-SP-199.md) · **Date Requested:** 2026-10-31

**What:** `EditorView`'s iOS container laid out by horizontal size class. **Compact:** one navigation stack, Navigator →
Manuscript → Inspector, the Inspector pushed as a page (G3), opening on the Manuscript. **Regular:** the `NavigationSplitView`
sidebar plus the `.inspector` trailing column, as on the Mac, with two- and three-panel versions shown to the user for a
choice. The `userInterfaceIdiom` branches (`EditorView.swift:481`) go.
**Tests:** the iOS target, the Mac interop tests, live passes on the iPad, iPhone and Duo simulators.

## 🔵 T-0637 — `[Apple]` EP-055 S2: the iOS gestures and the Timeline grabber, with routes that need no gesture

**Epic:** [EP-055] (AC6, AC8) · **Status:** 🔵 **Active** · **Sprint:** [SP-199] (🟡 Active) → [`../Sprints/Sprint-SP-199.md`](../Sprints/Sprint-SP-199.md) · **Date Requested:** 2026-10-31

**What:** design §8. Compact: swipe left goes to the next page and swipe right to the previous one (the system back swipe).
Regular: edge swipes show and hide the Navigator and Inspector, and full-width swipes combine them (G1). Both: the Timeline
grabber (G2). G4: the View menu and shortcuts, VoiceOver actions, and the ⋯ menu. ⛔ Nothing may take the manuscript's
scrolling, selection or the system's text gestures.
**Tests:** an iOS test for the gesture → panel-state mapping; live passes.

