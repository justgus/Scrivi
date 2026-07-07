# Closed Issue (Not Verified)

## I-0019: Undo and Redo have no effect in the manuscript editor

**Status:** ⚪ Closed — Not Verified (2026-07-06, user-approved)
**Reason for Closure:** **Superseded / Design Decision** — overtaken by events (OBE). The Issue's
prescribed resolution (restore standard AppKit per-keystroke undo via `NSTextView`'s built-in
`NSUndoManager`) was rejected by the approved EP-019 design, which replaces native undo entirely
with a custom, sentence-granular, tree-structured history system and deliberately sets
`allowsUndo = false`. The user-facing requirement ("⌘Z/⇧⌘Z must work in the manuscript editor")
is carried forward, in better-specified form, by **EP-019 AC1** (target sprint **SP-053**), which
cannot close without user verification.
**Re-open condition:** if EP-019 is cancelled, or descoped such that AC1 no longer delivers working
undo/redo in the manuscript editor, this Issue must be re-opened.

**Platform:** macOS, iPadOS
**Component:** `ManuscriptTextView.swift`
**Severity:** High
**Sprint:** Was assigned to SP-053 (Planning) at closure; originally Not Assigned
**Date Identified:** 2026-06-09
**Date Closed:** 2026-07-06
**Related:** EP-019 (successor), `docs/Scrivi_UndoRedo_History_and_Copy_Buffers_Design_v0_1.md`
(✅ Approved baseline 2026-07-06), T-0121 (SP-035 — Edit menu; earlier inconclusive investigation)

---

**Description:**
Pressing `⌘Z` in the manuscript editor produces the system "nothing to undo" flash — the undo
manager is empty even after typing. Redo has no effect either.

**Expected Behavior (as originally written — now obsolete):**
Undo reverses the most recent text edit. Redo re-applies it. Standard AppKit per-keystroke undo via
`NSTextView`'s built-in undo manager.

**Actual Behavior:**
`⌘Z` produces a screen flash (empty undo stack). Nothing is undone.

**Steps to Reproduce:**
1. Open a project; type in the manuscript editor.
2. Press ⌘Z — system "nothing to undo" flash; no text change.

**Impact:**
- No undo/redo capability in the core writing surface (High severity).

**Root Cause Analysis (confirmed 2026-07-06):**
The editor's authoritative state is `ViewportSceneLoader.segments[].text`; the single
`NSTextStorage` is rebuilt/re-extracted around `NSTextView` on every keystroke, so `NSUndoManager`
never accumulates a usable action stack — ⌘Z correctly reports an empty undo stack. The earlier
attempt (wrapping `rebuildStorage` with `disableUndoRegistration`, and T-0121's responder-chain
restoration) could not fix this because the problem is not spurious registrations; it is that
content edits are never registered against a stable storage. Restoring native per-keystroke undo is
architecturally impossible without abandoning the editor's storage model.

**Resolution (why closed instead):**
Per user decision (2026-07-06), undo/redo is being implemented from scratch as
**EP-019 — Custom Undo/Redo History & Multiple Copy Buffers**: sentence-granular history events,
tree-structured per-project persistent history (branching, sessions, capacity/purge), and multiple
copy buffers. Design approved as baseline 2026-07-06 with all six trade studies ruled. The Issue's
underlying defect (no working undo today) is acknowledged — closure records that this Issue's
tracking role and prescribed fix are superseded, not that the defect is fixed. Delivery and user
verification of working undo/redo occur under EP-019 AC1 / SP-053.

**Files Affected:** none (closed without code change).

---

*Closed 2026-07-06 with user approval ("Please help me decide if I-0019 is now OBE. If it is, it should be closed." — assessment: OBE, superseded by EP-019).*
