# Issue Backlog

Issues listed here are open and documented but not currently assigned to a Sprint.

> **I-0051 moved to Issue-active.md (2026-06-25)** — assigned to **SP-046**. See `Issue-active.md`.

---

## I-0017: Window maximized state not restored on app relaunch

**Status:** 🔴 Open
**Platform:** macOS
**Component:** `WindowFrameAutosave.swift`
**Severity:** Medium
**Sprint:** Not Assigned
**Related:** I-0051 (multi-window per-project frame/position restore — predates and largely subsumes this; the zoom-timing problem here should be folded into that fix)

**Description:**
Window position, size, and maximized state are not fully restored between app launches. Frame and position restore correctly. Maximized state does not — the window always relaunches un-maximized regardless of saved zoom state.

**Expected Behavior:**
On relaunch, the window appears at the same size, position, and maximized state as when the user last quit. The Landing View and Editor share the same window — no resize occurs when transitioning between them.

**Actual Behavior:**
Frame and position restore correctly. Maximized state does not restore.

**Steps to Reproduce:**
1. Maximize the window.
2. Quit the app.
3. Relaunch — window opens un-maximized.

**Date Identified:** 2026-06-08

**Root Cause Analysis:**
`window.zoom(nil)` fires too early — SwiftUI's `WindowGroup` continues async layout passes after the call and overrides it. Current approach uses `NSApplication.didFinishLaunchingNotification` as the trigger, but this has not resolved the issue. Requires deeper investigation.

**Resolution:** TBD

---

## I-0018: Scene Navigator shows no selection on app load

**Status:** 🔴 Open
**Platform:** macOS
**Component:** `SceneNavigatorView.swift`, `ViewportSceneLoader.swift`
**Severity:** Low
**Sprint:** Not Assigned

**Description:**
When the app loads a project, no scene is selected/highlighted in the Scene Navigator. The Navigator self-corrects on first scroll.

**Expected Behavior:**
On load, the Navigator highlights the scene visible at the top of the manuscript viewport.

**Actual Behavior:**
No scene is highlighted until the first scroll event.

**Date Identified:** 2026-06-08

**Root Cause Analysis:**
`viewportSceneID` is intentionally left nil during `loadAll()`. The scroll observer sets it on first scroll, but this hasn't fired at load time.

**Resolution:**
TBD — needs a mechanism to determine the top-of-viewport scene after `NSTextView` completes initial layout without triggering a spurious scroll notification.

---

## I-0019: Undo and Redo have no effect in the manuscript editor

**Status:** 🔴 Open
**Platform:** macOS, iPadOS
**Component:** `ManuscriptTextView.swift`
**Severity:** High
**Sprint:** Not Assigned

**Description:**
Pressing `⌘Z` in the manuscript editor produces the system "nothing to undo" flash — the undo manager is empty even after typing. Redo has no effect either.

**Expected Behavior:**
Undo reverses the most recent text edit. Redo re-applies it. Standard AppKit per-keystroke undo via `NSTextView`'s built-in undo manager.

**Actual Behavior:**
`⌘Z` produces a screen flash (empty undo stack). Nothing is undone.

**Date Identified:** 2026-06-09

**Root Cause Analysis:**
Requires deeper investigation. Previous attempt (wrapping `rebuildStorage` with `disableUndoRegistration`) did not resolve the issue.

**Resolution:** TBD

---

*Last Updated: 2026-06-09 (I-0017/I-0019 returned to backlog; I-0018 unchanged)*
