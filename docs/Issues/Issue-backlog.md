# Issue Backlog

Issues listed here are open and documented but not currently assigned to a Sprint.

---

## I-0017: Window maximized state not restored on app relaunch

**Status:** 🔴 Open
**Platform:** macOS
**Component:** `WindowFrameAutosave.swift`
**Severity:** Medium
**Sprint:** Not Assigned

**Description:**
Window position and size are correctly restored between app launches, but the maximized (zoomed) state is not. If the user maximizes the project window and quits, the window reopens at the previous un-maximized size instead of maximized.

**Expected Behavior:**
If the window was maximized at quit, it reopens maximized. The Landing View also respects the saved window state.

**Actual Behavior:**
Window reopens at un-maximized size. Landing View shows at a default size regardless of saved state.

**Steps to Reproduce:**
1. Open a project, maximize the window.
2. Quit the app.
3. Relaunch — window opens un-maximized.

**Impact:**
- Minor friction for writers who prefer working maximized.

**Date Identified:** 2026-06-08

**Root Cause Analysis:**
`window.zoom(nil)` is deferred via `DispatchQueue.main.async` inside `makeNSView`, but SwiftUI's `WindowGroup` continues to apply its own sizing after that point, overriding the zoom. The Landing View is shown before the project opens, so `WindowFrameAutosave` is not in the view hierarchy at that point.

**Resolution:**
TBD — likely requires hooking into `NSApplicationDelegate.applicationDidFinishLaunching` after SwiftUI has completed all initial layout passes, or using `windowStyle(.hiddenTitleBar)` + manual frame management.

---

## I-0018: Scene Navigator shows no selection on app load

**Status:** 🔴 Open
**Platform:** macOS
**Component:** `SceneNavigatorView.swift`, `ViewportSceneLoader.swift`
**Severity:** Low
**Sprint:** Not Assigned

**Description:**
When the app loads a project, no scene is selected/highlighted in the Scene Navigator. The correct behavior is no selection (preferred over a wrong selection), but ideally the Navigator should highlight whichever scene is visible at the top of the viewport after load.

**Expected Behavior:**
On load, the Navigator highlights the scene visible at the top of the manuscript viewport — or shows no selection if that is not determinable before the first scroll event.

**Actual Behavior:**
No scene is highlighted. The Navigator becomes correct after the first scroll.

**Steps to Reproduce:**
1. Open a project with 2+ scenes.
2. Observe Scene Navigator immediately on load — no scene is highlighted.
3. Scroll slightly — the correct scene highlights.

**Impact:**
- Low — the Navigator self-corrects on first scroll. No functional breakage.

**Date Identified:** 2026-06-08

**Root Cause Analysis:**
`viewportSceneID` is intentionally left nil during `loadAll()` to avoid the previous bug where a stale value caused SwiftUI's `List` to select the last row. The scroll observer sets `viewportSceneID` on first scroll, but this hasn't fired yet at load time.

**Resolution:**
TBD — needs a mechanism to determine which scene is at the top of the viewport after `NSTextView` completes its initial layout, without triggering a scroll notification.

---

*Last Updated: 2026-06-08 (I-0017, I-0018 added to backlog)*
