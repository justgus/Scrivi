# EP-013: Scene Navigator Swipe Gesture

**Status:** ✅ Closed
**Goal:** Let the writer show and hide the Scene Navigator sidebar using a two-finger horizontal swipe on the manuscript surface, as a faster alternative to the NavigationSplitView collapse control.
**Date Created:** 2026-06-09
**Date Closed:** 2026-06-10
**Actual Close Date:** 2026-06-10

### Acceptance Criteria

- [ ] A two-finger swipe **left** on the manuscript surface hides the Scene Navigator (collapses the `NavigationSplitView` sidebar)
- [ ] A two-finger swipe **right** on the manuscript surface shows the Scene Navigator (expands the sidebar)
- [ ] The gesture does not interfere with text selection, scrolling, or cursor placement in `ManuscriptTextView`
- [ ] The gesture works on macOS (trackpad); iPadOS support deferred to T-0126
- [ ] The swipe threshold is 60pt horizontal displacement with horizontal velocity ≥ 2× vertical velocity to avoid accidental triggers
- [ ] Sidebar state (shown/hidden) persists across scene navigation within the same project session (not required to persist across app launches)
- [ ] No regression in `NavigationSplitView` collapse/expand behavior via the standard sidebar toggle button

*None of the acceptance criteria were delivered.*

### Tasks

| ID | Title | Status |
| -- | ----- | ------ |
| T-0125 | Two-finger swipe gesture on ManuscriptTextView — macOS (trackpad) | 🔴 Closed |
| T-0126 | Two-finger swipe gesture on ManuscriptTextView — iPadOS (touch) | 🔴 Closed (with EP-013) |
| T-0127 | EP-013 verification | 🔴 Closed |

### Sprints

| Sprint | Title | Status |
| ------ | ----- | ------ |
| SP-036 | Scene Navigator Swipe Gesture — macOS | ✅ Closed (no delivery) |

### Completion Summary

EP-013 is closed with no delivery. SP-036 attempted two implementation approaches for the macOS swipe gesture; both failed:

1. **NSPanGestureRecognizer on NSScrollView** — competed with `NSScrollView`'s built-in touch handling, blocking vertical scrolling.
2. **scrollWheel override on ManuscriptNSTextView** — horizontal scroll events did not route through the document view as expected; likely intercepted by `NSScrollView` or `NavigationSplitView` earlier in the responder chain.

All code changes were reverted. The codebase is in the same state as before SP-036 began.

**Future investigation:** See `docs/Tasks/Closed/Task-closed-0125.md` for four untried candidate approaches. The core unknown is where in the AppKit/SwiftUI responder chain a two-finger horizontal swipe lands when the text view is hosted inside an `NSScrollView` inside a SwiftUI `NavigationSplitView`.
