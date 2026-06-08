## T-0117: Reliable first-responder transfer — replace `focusManuscriptView` workaround

**Status:** ✅ Implemented - Verified
**Date Implemented:** 2026-06-08
**Component:** `ManuscriptTextView`, `ViewportSceneLoader`, `AppEnvironment`
**Priority:** High
**Date Requested:** 2026-06-08
**Sprint Assigned:** SP-033

**Rationale:**
The `focusManuscriptView` binding added in SP-032 is an acknowledged workaround (EP-011 Known Defect 3). A clean, direct first-responder transfer model is needed.

**Current Behavior:**
Focus transfer uses a `@Binding` flag (`focusManuscriptView`) that SwiftUI observes and reacts to by triggering AppKit focus. This is a round-trip through the SwiftUI layer for a purely AppKit operation.

**Desired Behavior:**
- First-responder transfer is performed directly in AppKit: `window?.makeFirstResponder(textView)`.
- No SwiftUI binding is involved in the transfer.
- The `focusManuscriptView` binding and its observation machinery are removed.
- Focus is transferred after: scene delete (see T-0116), Navigator tap (see T-0116), and any other structural operation that moves the cursor.

**Requirements:**
1. `focusManuscriptView` binding removed from `ManuscriptTextView` and all call sites
2. `ManuscriptNSTextView` (or its coordinator) exposes a method `takeFocus()` that calls `window?.makeFirstResponder(self)`
3. All callers that previously set `focusManuscriptView = true` now call `takeFocus()` directly
4. Focus transfer works correctly on macOS 26

**Design Approach:**
Expose `takeFocus()` on the `Coordinator` or on a shared reference held by `ViewportSceneLoader`. Remove the binding. Update all callers.

**Components Affected:**
- `ManuscriptTextView.swift`: remove binding, add `takeFocus()` method
- `ViewportSceneLoader.swift` (or equivalent): update callers
- `AppEnvironment.swift`: remove `focusManuscriptView` if present there

**Dependencies:**
- No hard dependency, but should be implemented before T-0116 so T-0116 can use the clean API

**Test Steps:**
1. Delete a scene — confirm cursor appears in text view and writer can type without clicking
2. Tap a scene in Navigator — confirm text view is focused and cursor is placed
3. Build with zero warnings related to removed binding

**Notes:**
`focusManuscriptView` was introduced as a temporary fix in SP-032. Remove it entirely — do not leave a deprecated stub.
