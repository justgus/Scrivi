# Verified Task — T-0689

**Sprint:** [SP-227] · **Epic:** [EP-055] · **Platform:** `[Apple]`
✅ **USER-VERIFIED 2026-11-20** (user, 2026-11-20: *"I also ran all these steps.  They all pass."*)
**Archived:** 2026-11-20.

---

## ✅ T-0689 — `[Apple]` EP-055 S7: the whole app at iPhone width

**Epic:** [EP-055] (AC8; AC10's iPhone and Duo passes) · **Status:** ✅ **Verified 2026-11-20** (user) · **Sprint:** [SP-227] (✅ Closed) →
[`../../Sprints/Closed/Sprint-SP-227.md`](../../Sprints/Closed/Sprint-SP-227.md) · **Date Requested:** 2026-11-20 (user: *"plan SP-227"*)

**What:** every sheet fits an iPhone in portrait (the worlds sheet re-flowed, the detail sheet and New Project without their Mac
minimum widths) and the four commands only the iPad's menu bar has today (Import, Show Markup Hints, Purge Stale History Branches,
About) reachable by touch. Findings, questions and ACs: the Sprint.

**Implemented ([SP-227], 2026-11-20):** the worlds sheet re-flows with `ViewThatFits` — each row (`worldRow`: `worldName` at a
fixed ideal width, then `worldControls`) and the bottom bar (`worldActions`, `doneButton`); 460 pt is Mac-only and the iOS sheet
scrolls (`WorldsView.swift`). The detail sheet's and New Project's minimum widths are Mac-only (`EditorView.swift`,
`ObjectDetailSheet.swift`, `NewProjectSheet.swift`). The toolbar gains Import (`ImportMenuItems`, shared with File ▸ Import), Markup
Hints in View, Purge Stale History Branches… and About Scrivi (`ProjectToolbar.swift`; `env.showAbout`). iOS test
`PhoneWidthSheetTests`: ten sheets at 402 pt.

**Tested:** iOS 38/38 (`PhoneWidthSheetTests`: ten sheets at 402 pt, and a control); Mac interop 352/352; macOS and iOS builds;
CI green. ✅ the iPhone, Duo (folded and open), Mac and iPad live passes (user, 2026-11-20).
