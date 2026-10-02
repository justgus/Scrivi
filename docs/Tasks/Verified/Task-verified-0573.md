# Verified Task: T-0573

| ID | Task | Sprint | Epic | Verified |
| -- | ---- | ------ | ---- | -------- |
| **T-0573** | ✅ **The caret keeps its height in the viewport across create/merge of a scene or chapter** | [SP-151] | — | **2026-10-02** |

✅ **VERIFIED by the user 2026-10-02:** scenes — *"scene create/merge work ok"*; chapters, after [I-0272] — *"The Chapter create/merge is ok now."*

## What shipped
- ✅ `rebuildStorage` measures the caret's offset below the viewport top BEFORE rebuilding (`caretOffsetInViewport`).
- ✅ `revealCaretAfterRebuild`: `scrollRangeToVisible` → `layoutViewport()` → nudge by the RELATIVE difference, measured
  inside laid-out text → `layoutViewport()`. All five rebuild-then-place-caret paths use it.
- ⛔ **First attempt FAILED live** (scrolled near Chapter 44): it used the caret's ABSOLUTE y after the rebuild, which
  TextKit 2 only ESTIMATES. ✅ Measured in a standalone TextKit 2 harness before the second attempt.
- ⚠️ The chapter half needed [I-0272] (a second rebuild after the handler's).
