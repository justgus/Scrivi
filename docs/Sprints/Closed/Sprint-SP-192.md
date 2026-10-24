---
sprint: SP-192
epic: none
status: Closed
platform: Apple
created: 2026-10-24
---

# SP-192 — `[Apple]` The branch-point marker: where ⇧⌘Z will ask which branch

**Status:** ✅ **CLOSED** 2026-10-24 (user: *"tests passed. go ahead and mark the Task verified and the sprint closed (It was apple only)."*) — activated 2026-10-24 (user: *"create and activate the T-0620 sprint now"*). ✅ Q1–Q4 ruled 2026-10-24
(user: *"Q1–Q4 agreed as recommended"*).
**Carries:** [T-0620] (✅ Verified) → [`../../Tasks/Verified/Task-verified-0620.md`](../../Tasks/Verified/Task-verified-0620.md).
**Epic:** ⚠️ none. The Linux half goes with the Linux branch chooser in [EP-026] S3 (not yet created).
**Builds on:** [I-0320] (the chooser on ⇧⌘Z only; the core's `redoFork`) and [I-0321] (a branch with no text is not
offered). Both were done in [SP-188], which stays active for its Linux checks.
**Size:** S–M.

**Ruled 2026-10-24 (user), [T-0620] Q1–Q4:** the marker sits in the **left margin** beside the line where the branches part;
it shows **only while Show Markup Hints (⇧⌘H) is on**; **clicking it opens the branch chooser**, the same as ⇧⌘Z; Linux
*"eventually"* ([EP-026] S3).

---

## What reading the code found (2026-10-24)

- **The margin exists:** `textContainerInset` is 60 pt wide (`ManuscriptTextView.swift:76`), and nothing draws in it.
  `lineRect(forCharacterIndex:in:)` (`:1634`) gives a line's rect for a character, the view rect the margin mark needs.
- **⇧⌘H is "Show Markup Hints"** (`ScriviApp.swift:453`), per project (`ProjectPreferences.markupHints`), reaching the editor
  as `markupHints` → `presenter.setHints` (`:62`, `:245`). The marker reads the same flag.
- **When there is a branch point:** the core's `redoFork` ([I-0320]), on every `scrivi_capture_*` report, and absent while
  an edit is pending. A text undo reports it in the same turn. A structural undo lands later, through the off-main path.
- ⚠️ **The fork's report names no PLACE, though the core knows it:** `ForkChild` (`HistoryService.hpp:166`) carries
  eventID / preview / timestamp / isPrimary only. ✅ The user (2026-10-24): *"somewhere we know where a branchpoint is
  because the History Card automatically filters based on the scene the caret is currently in."* Every node keeps its
  `sceneID` (I-0102), and the tree reports it with `changeOffsetUtf8` (2026-08-06, for the card's caret highlight). So
  `ForkChild` gains those two, from the branch's first node. Its offset is in the current text, because a fork's children
  apply to the node the pointer is on. These are fields on `redoFork` / `forkAhead`, not a new endpoint.
- **The chooser:** `presentForkPopover(_:in:)` anchors at the caret (`ForkPopover.swift`). A click on the marker opens it
  at the marker instead.

## Goal

With Show Markup Hints on, a small mark in the left margin shows the line where ⇧⌘Z will ask which branch to redo. It is
drawn only, never stored, and stays out of the text, caret stops, Find, copy and word count. Clicking it opens the
chooser.

## Plan

0. **Red tests first** (interop): after an undo lands on a fork, the marker's line is the branch's line; after typing,
   there is no marker; with hints off, none; a click on the marker offers the chooser.
1. **Core:** each offered `ForkChild` gains `sceneID` and `changeOffsetUtf8`, from the branch's FIRST node (where the
   branch parts from the current text; a structural node has no diff, so its scene's start). Boundary-tested through
   `scrivi_capture_state`.
2. **Apple model:** `HistoryForkChild` decodes the two fields; `HistoryCapture` keeps the last `redoFork`. The coordinator
   recomputes the marker's line(s) when a report changes it, and after a structural landing.
3. **Drawing:** `ManuscriptNSTextView` draws the mark in the left inset, at the line rect of the branch's character
   position (byte → UTF-16 via the existing converter), only when `presenter.hintsEnabled`, redrawing just that rect.
   It also needs a tooltip and an accessibility label, ("⇧⌘Z will ask which branch to redo").
4. **Click:** a mouse-down in the mark's rect opens the chooser for the current fork, anchored at the mark; anywhere else
   in the margin behaves as today.
5. **Tests** (Plan 0) green; mutation-checked in a copy of the repo. Core ctest, interop.
6. **Design doc:** the undo design gains the marker (one paragraph).
7. **Live pass (user, Mac).**

## Questions — ✅ all ruled 2026-10-24 (as recommended)

| # | Question | Recommendation |
| - | -------- | -------------- |
| **Q1** | **Branches that start in different places.** After undo-then-type they part at one spot. But undo is global: two branches of one fork can begin in different scenes. One mark, or one per place? | **One mark per distinct line** among the offered branches. Clicking any of them opens the same chooser (the whole fork). Usually there is just one. |
| **Q2** | **A branch point off screen** (another scene, far away). Show anything else? | **No.** The mark is only seen where the line is; ⇧⌘Z still asks wherever the caret is. The whole-history view ([T-0621]) is where a far-away fork gets found. |
| **Q3** | **The mark's look.** | The SF Symbol `arrow.triangle.branch` in the secondary label colour, at the body text's size, vertically centred on the line, right-aligned in the 60 pt inset (just left of the text). Following light/dark like the hints. |
| **Q4** | **Hover.** | A tooltip, *"⇧⌘Z will ask which branch to redo"*, and the same as its VoiceOver label. |

## Acceptance Criteria

- [x] AC1 — With Show Markup Hints on, after an undo lands on a branch point, a mark appears in the left margin on the
      line(s) where the offered branches apply (Q1); none with hints off, and none once the writer types
- [x] AC2 — The mark is drawn only: the scene text, caret stops, Find, copy and word count are unchanged
- [x] AC3 — Clicking the mark opens the branch chooser for that fork; choosing redoes the branch, dismissing redoes
      nothing ([I-0320])
- [x] AC4 — Core: each offered branch carries its scene and byte position; boundary-tested through `scrivi_*`
- [x] AC5 — Tooltip and VoiceOver label (Q4)
- [x] AC6 — Tests, mutation-checked; ctest and interop green
- [x] AC7 — Live pass (user, Mac)

⛔ **Not in scope:** Linux ([EP-026] S3); the "other versions of this passage" markers and the whole-history view
([T-0621]).

## Progress log

### 🟡 2026-10-24 — CREATED and ACTIVATED
- User: *"create and activate the T-0620 sprint now"*. Issued via `next-id.py`. [T-0620] → active.
- The code read (above); Plan 0–7; ⏳ Q1–Q4 for ruling.

### 🟡 2026-10-24 — Q1–Q4 ruled
- User: *"Q1–Q4 agreed as recommended."* The user also pointed out that the core already knows where a branch is (the
  History Card's per-scene filter). It does: Plan 1 now reuses the tree's `sceneID` + `changeOffsetUtf8`.

### 🟡 2026-10-24 — IMPLEMENTED (AC1–AC6 met); ⏳ AC7 live pass
- **Core:** `ForkChild` gains `sceneID` + `changeOffsetUtf8` (the branch's first node; a structural node 0), written on
  `redoFork` / `forkAhead`; `scrivi.h` documents them. No new endpoint. Core test `[I-0321]` extended.
- **Mac:** `HistoryForkChild` decodes them. `HistoryCapture.redoForkNow` and `onReport` (called after every report).
  The coordinator's `refreshBranchMarks` places one borderless `NSButton` per distinct line (Q1) in the 60 pt left inset,
  right-aligned (Q3, `arrow.triangle.branch`, secondary label colour). It shows only while the hints are on. It
  refreshes after each report (on the next turn: a report comes before its step is applied), after a structural landing,
  on scroll and resize, and on the hints toggle. Off screen = no mark (Q2). The click → `presentForkPopover(at:)`, the
  popover anchored at the mark (Q3). Tooltip + VoiceOver label (Q4).
- ⚠️ **Found by the test:** a branch at the very END of the text (typing at a scene's end, the common case) had no line:
  the end position holds no character. Now the line of the character before.
- **Design doc:** the undo design's T2 refinements 1, 4, 5 marked superseded by [I-0320] (⚠️ missed in [SP-188]), and
  the mark added.
- **Tests:** interop `branchPointMark` (one mark on the branch's line, in the margin, a line that is not the scene's
  first; tooltip and label; the click offers both branches; the text unchanged; none with hints off; none after typing).
  Mutation-checked in a copy of the repo: the hints gate removed, and the mark at the scene's start: both caught.
- **Counts:** ctest 766/766; interop 314/314; no warnings.
- [T-0620] → 🟠 Implemented - Not Verified.

### 🟡 2026-10-24 — Mac live pass: ✅ all 6 steps (user); one change asked for, made; ⏳ its re-check
- User: *"All 6 steps pass. If you Cmd-Shift-Z into the change the branch chooser appears at the bottom of the scene. It
  should now appear at the branch symbol, same as if you clicked it."*
- ✅ Done: ⇧⌘Z opens the chooser at the branch's mark (the one nearest the caret when there are several). With the hints
  off it opens at the same place in the margin; only when that line is off screen does it fall back to the caret.
  `branchMarkFrames(for:in:)` is shared by the marks and ⇧⌘Z. The interop `branchPointMark` checks the anchor for the
  click, for ⇧⌘Z, and for ⇧⌘Z with the hints off. Mutation-checked (⇧⌘Z at the caret again: caught). Interop 314/314.
- ⏳ AC7: the user re-checks ⇧⌘Z's position; [T-0620] stays 🟠 until then.

### ✅ 2026-10-24 — CLOSED (user-approved)
- User: *"tests passed."* (⇧⌘Z opens the chooser at the mark, hints on and off), then *"tests passed. go ahead and mark the Task verified and the sprint closed (It was apple only)."*
- [T-0620] ✅ Verified → `Tasks/Verified/Task-verified-0620.md`. All of AC1–AC7 met.

## Retrospective
- **Went well:** the user's hint (the History Card already knows a branch's scene) turned a planned core addition into
  passing on two fields the tree already computed. A button over the margin gave the click, the tooltip and VoiceOver
  without custom drawing. Each test, with its mutation run in a copy of the repo, caught its mutation.
- **Went badly:** the first test passed with the mark on the scene's FIRST line, so it could not tell the right line from a
  wrong one. Tightening it exposed a real bug: no mark for a branch at the text's very end, which is the common case. The
  undo design's chooser refinements had been left stale since [I-0320] in [SP-188]. ⇧⌘Z's chooser position was found
  only by the live pass.
- **Adjust:** a position test puts its target where a wrong answer would differ (not on the first line or at an edge);
  include the document's END in any position mapping; when a behaviour is superseded, update the design doc's rule in
  the same change; when one gesture gets a new anchor, check every gesture that opens the same UI.
