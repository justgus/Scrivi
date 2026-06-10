# SP-036: Scene Navigator Swipe Gesture — macOS

**Status:** ✅ Closed
**Epic:** EP-013: Scene Navigator Swipe Gesture
**Goal:** Wire a two-finger horizontal swipe gesture on the macOS manuscript surface to collapse and expand the Scene Navigator sidebar via `NavigationSplitViewVisibility`.
**Start Date:** 2026-06-10
**End Date:** 2026-06-10
**Capacity:** Single session

### Assigned Tasks

| ID | Title | Priority | Status |
| -- | ----- | -------- | ------ |
| T-0125 | Two-finger swipe gesture on ManuscriptTextView — macOS (trackpad) | High | 🔴 Closed |
| T-0127 | EP-013 verification | High | 🔴 Closed |

### Assigned Issues

*None.*

### Sprint Notes

- T-0126 (iPadOS swipe) was explicitly deferred to a later sprint before this sprint began.
- Two implementation approaches were attempted and both failed. See `Task-closed-0125.md` for full failure analysis.
- All code changes from both approaches were reverted. The codebase is in the same state as before SP-036 began.

### Retrospective

**Completed:**
- Nothing delivered. Both implementation attempts failed.

**Returned to Backlog:**
- Nothing. T-0125 and T-0127 were closed (not returned to backlog). EP-013 closed.

**What went wrong:**
- **Approach 1 (NSPanGestureRecognizer):** Competed with `NSScrollView`'s built-in touch handling and blocked vertical scrolling.
- **Approach 2 (scrollWheel override on ManuscriptNSTextView):** `scrollingDeltaX` events from a horizontal trackpad swipe did not route through the document view's `scrollWheel` method as expected — likely intercepted earlier in the responder chain by `NSScrollView` or `NavigationSplitView`.

**What to improve:**
- Before implementing gesture features on AppKit views hosted inside SwiftUI containers, invest time in characterizing how events actually flow through the responder chain in that specific context — don't assume standard AppKit patterns hold.

**Carry-forward notes:**
- EP-013 is closed. See `Task-closed-0125.md` for four candidate approaches that were not tried — any future attempt at this feature should start there.
- EP-014 is next.
