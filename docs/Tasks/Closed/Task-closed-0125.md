# Task-closed-0125: Two-Finger Swipe Gesture on ManuscriptTextView — macOS (Trackpad)

**Status:** 🔴 Closed
**Component:** `ManuscriptTextView.swift`, `EditorView.swift`
**Priority:** High
**Date Requested:** 2026-06-10
**Date Closed:** 2026-06-10
**Sprint Assigned:** SP-036
**Epic:** EP-013

**Reason for Closure:** Two implementation approaches attempted; both failed to produce working behavior on macOS. Feature is closed pending a future investigation sprint that can more deeply characterize how `NSScrollView` and `NavigationSplitView` interact with gesture and scroll event routing on macOS 26.

---

## Rationale (Original)

The Scene Navigator sidebar can only be shown or hidden via the `NavigationSplitView` collapse button. A two-finger horizontal swipe on the manuscript surface would provide a faster, more fluid gesture-driven alternative consistent with macOS trackpad conventions.

---

## Approach 1: NSPanGestureRecognizer on NSScrollView

**Description:**
Added an `NSPanGestureRecognizer` (with `buttonMask = 0` for trackpad-only) to the `NSScrollView` returned from `ManuscriptTextView.makeNSView`. The gesture fired at `.ended` when `abs(translation.x) > 60` and `abs(velocity.x) >= 2 * abs(velocity.y)`. Sidebar state was managed via `@State private var columnVisibility: NavigationSplitViewVisibility` in `ManuscriptEditorView`, toggled by `onSwipeLeft`/`onSwipeRight` callbacks passed down to the gesture handler.

**Why It Failed:**
The `NSPanGestureRecognizer` competed directly with `NSScrollView`'s built-in touch handling. When the recognizer was active, it consumed the two-finger touch stream, preventing the scroll view from receiving the events needed for normal vertical scrolling. The gesture and the scroll view could not coexist on the same touch sequence.

**Files Modified (reverted):**
- `Scrivi/Views/ManuscriptTextView.swift` — `onSwipeLeft`/`onSwipeRight` properties on struct and `Coordinator`; `NSPanGestureRecognizer` in `makeNSView`; `handleSwipeGesture(_:)` on `Coordinator`
- `Scrivi/Views/EditorView.swift` — `@State columnVisibility`; `NavigationSplitView(columnVisibility:)`; swipe callbacks

---

## Approach 2: scrollWheel Override on ManuscriptNSTextView

**Description:**
Removed the gesture recognizer entirely. Instead, overrode `scrollWheel(with:)` on `ManuscriptNSTextView`. The override always called `super` first (to preserve vertical scrolling), then accumulated `scrollingDeltaX`/`Y` across `.began`/`.changed` phases of the trackpad gesture. On `.ended`, if `abs(accumX) > 60` and `abs(accumX) > abs(accumY)`, the sidebar callback fired. Mouse wheel events (`event.phase == []`) were ignored. Callbacks (`onSwipeLeft`/`onSwipeRight`) lived on `ManuscriptNSTextView` and were refreshed each SwiftUI cycle via `updateNSView`.

**Why It Failed:**
The override did not produce working sidebar toggling in practice. The `scrollingDeltaX` values during a horizontal trackpad swipe on the manuscript surface were not reliably routing through `ManuscriptNSTextView.scrollWheel` — likely because `NSScrollView` handles the scroll event before it reaches the document view's `scrollWheel` method, or because `NavigationSplitView`'s gesture handling intercepts horizontal scroll events at a higher level in the responder chain before they reach the text view.

**Files Modified (reverted):**
- `Scrivi/Views/ManuscriptTextView.swift` — `onSwipeLeft`/`onSwipeRight` on `ManuscriptNSTextView`; `swipeAccumX`/`Y` accumulators; `scrollWheel` override
- `Scrivi/Views/EditorView.swift` — same as Approach 1

---

## Known Unknowns / Future Investigation Notes

For a future attempt, the key question is: **where in the responder chain does a two-finger horizontal swipe event land when the cursor is in an NSScrollView hosted inside a NavigationSplitView?**

Candidate approaches not yet tried:
1. **Override `scrollWheel` on the `NSScrollView` subclass** (not the document view) — the scroll view may see the event before forwarding it to the document view.
2. **`NSGestureRecognizer` on the `NavigationSplitView`'s SwiftUI container** via a SwiftUI `.simultaneousGesture` modifier — avoids AppKit event routing entirely.
3. **`NSEvent` monitor** (`NSEvent.addLocalMonitorForEvents`) filtering for `scrollWheel` events with a dominant horizontal component — processes events before they enter the normal responder chain.
4. **SwiftUI `DragGesture` with `.simultaneously(with:)`** on the `ManuscriptTextView` wrapper — may work if `NavigationSplitView` does not consume the drag first.

---

*Closed: 2026-06-10 — both approaches reverted; codebase restored to pre-SP-036 state*
