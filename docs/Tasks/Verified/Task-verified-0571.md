# Verified Task: T-0571

| ID | Task | Sprint | Epic | Verified |
| -- | ---- | ------ | ---- | -------- |
| **T-0571** | ✅ **Navigator search → caret at the FIRST MATCH** | [SP-151] | — | **2026-10-01** |

✅ **VERIFIED by the user 2026-10-01** by live pass on `dumas-prose-timelines`: *"Ok this run passed. both items behaved correctly."*
✅ The console shows it working: `navigateToScene … boundaryStart=1850620 caret=1850721` and `boundaryStart=1835601 caret=1836519` (match, not scene start); later clicks with no match land at `caret=boundaryStart`.

## What shipped
- ✅ `SceneNavigatorView.swift` — the listSelection setter writes `loader.searchCaretHint` (scene + query) BEFORE the selection.
- ✅ `ViewportSceneLoader.swift` — `searchCaretHint`, `@ObservationIgnored`.
- ✅ `ManuscriptTextView.swift` — `searchMatchOffset`: first match inside the scene's own text, `.caseInsensitive + .diacriticInsensitive`, current locale (same semantics as the filter). Not consumed (a click runs `navigateToScene` 2–3×); expires after 0.5 s.
- ✅ Rulings 2026-10-01: chapter-title-only match → scene start; caret only, no selection; first match only.
