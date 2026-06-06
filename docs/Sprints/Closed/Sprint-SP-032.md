# Sprint SP-032: Chapter Title Toggle, Navigator Fallback, and EP-010 Close

**Status:** ✅ Closed
**Epic:** EP-010: Manuscript Structure Editing — Delete, Reorder, and Title
**Goal:** Implement the two remaining EP-010 features — Project Settings with chapter title toggle, and the navigator title fallback chain with correct cursor placement after scene delete — then verify all 14 EP-010 acceptance criteria and close the epic.
**Start Date:** 2026-06-06
**End Date:** 2026-06-06
**Capacity:** —

### Assigned Tasks

| ID | Title | Priority | Status |
| -- | ----- | -------- | ------ |
| T-0109 | Global chapter title toggle + headings in writing surface | High | ✅ Verified |
| T-0110 | Navigator title fallback chain + delete-of-open-scene edge case + cursor placement | High | ✅ Verified |
| T-0111 | EP-010 acceptance criteria verification + macOS smoke test | Critical | 🚫 Retired (OBE) |

### Assigned Issues

*None.*

### Sprint Notes

T-0109 and T-0110 were implemented and verified. T-0111 (acceptance criteria walk) was retired as Overtaken By Events: during smoke testing the scroll-driven scene promotion mechanism was found to have a fundamental UX defect — scrolling up past a scene boundary instantly jumps the viewport position rather than preserving the author's scroll position. This defect predates EP-010 and affects the underlying `ManuscriptTextView` / `ViewportSceneLoader` design. Rather than verify partial ACs against a broken scroll surface, the decision was made to close EP-010 on the implemented structural editing work and open EP-011 to redesign the writing surface behavior from scratch.

### Retrospective

**Completed (Implemented and Verified):**
- T-0109 — Project Settings sheet with chapter title toggle; headings injected at chapter boundaries in `ManuscriptTextView.rebuildStorage` when toggle is on
- T-0110 — Delete-of-open-scene now navigates to the nearest remaining scene and transfers keyboard focus to `ManuscriptNSTextView` via `focusManuscriptView` binding

**Retired (OBE):**
- T-0111 — Full acceptance criteria walk deferred; scroll surface defect makes a meaningful smoke test impossible without first fixing the underlying behavior

**What went well:**
- Structural editing features (delete, rename, reorder) all implemented correctly across SP-027–SP-031
- Chapter title toggle and delete-navigate focus transfer implemented cleanly in SP-032

**What to improve:**
- The scroll-driven scene promotion design (viewport loader + NSTextView boundaries) was never formally specified to handle the author's scroll position correctly; this will be the focus of EP-011

**Carry-forward notes:**
- EP-011: Writing Surface Behavior — full redesign of scroll, cursor, and focus behavior for `ManuscriptTextView` and `ViewportSceneLoader`
- T-0096 (scroll bar fidelity) and T-0112 (cursor position tracking) remain in the Task backlog and are natural candidates for EP-011 scope
