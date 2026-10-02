# Verified Task: T-0572

| ID | Task | Sprint | Epic | Verified |
| -- | ---- | ------ | ---- | -------- |
| **T-0572** | ✅ **The caret skips the gap between scenes — chapter headings AND the divider** | [SP-151] | — | **2026-10-02** |

✅ **VERIFIED by the user 2026-10-02** by live pass on `dumas-prose-timelines`: *"These all pass."*

## What shipped
- ✅ `ManuscriptNSTextView.setSelectedRanges(_:affinity:stillSelecting:)` — the one choke point every caret
  placement passes; redirects a SETTLED zero-length caret proposed in the gap.
- ✅ `Coordinator.caretOutsideSceneGap` — gap = after a divider up to the next scene's first character (the
  divider's `\n` + any heading). Backward → the divider position (previous scene's END, kept by ruling: typing
  there appends to that scene); otherwise → the next scene's start. Read from the text's own markers
  (attachment + `scriviHeading`), not `sceneBoundaries`.
- ✅ Selections with length and mid-drag selections untouched.
