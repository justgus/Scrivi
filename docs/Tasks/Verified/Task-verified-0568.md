# Verified Task: T-0568

| ID | Task | Sprint | Epic | Verified |
| -- | ---- | ------ | ---- | -------- |
| **T-0568** | ✅ **Go to Manuscript Start / End** — `Project` menu | [SP-151] | — | **2026-09-30** |

✅ **VERIFIED by the user 2026-09-30** (user-directed): *"Manuscript start/end both work ok."*

## What shipped
- ✅ `Project ▸ Go to Manuscript Start / End` (`ScriviApp.swift`), acting through
  `ProjectSession.manuscriptStart/EndAction` → `moveToManuscriptBoundary` (`ManuscriptTextView.swift`),
  the end measured in UTF-16 (`NSString.length`), ⛔ not `String.count`.
- ⛔ **NO toolbar group** — built, then REMOVED by user ruling: its buttons read as Scene Start/End.
  ✅ ⌘↑/⌘↓ already reach the manuscript's ends from the text view (user-confirmed).
- ⚠️ The navigator half became [I-0258] (the list now FOLLOWS the viewport scene with `anchor: nil`).
