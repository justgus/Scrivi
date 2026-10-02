# Verified Task: T-0569

| ID | Task | Sprint | Epic | Verified |
| -- | ---- | ------ | ---- | -------- |
| **T-0569** | ✅ **Scene navigator search** — always-visible field at the BOTTOM of the navigator | [SP-151] | — | **2026-10-01** |

✅ **VERIFIED by the user 2026-10-01** by live pass on `dumas-prose-timelines`: *"The live test worked great."*

## What shipped
- ✅ Search field as a `.safeAreaBar(edge: .bottom)` on the navigator list (`SceneNavigatorView.swift`, `searchField`).
- ✅ Full scan, NO word index, built PER PLATFORM (ruled 2026-10-01) — body scan off the main actor
  (`@concurrent scenesMatching`), cancelled per keystroke via `.task(id:)`, matched with
  `localizedStandardContains` (case- and diacritic-insensitive). ~24 ms on 4.2 MB / 1,200 scenes.
- ✅ A chapter whose title matches keeps all its scenes; otherwise a scene stays when its derived title or
  LIVE body text matches.
- ⚠️ Drag-reorder disabled while filtering; results do not re-filter while typing in the manuscript.
- ➡️ Follow-up: caret to the first MATCH (not the scene start) on a searched navigation — proposed 2026-10-01, not yet filed.
