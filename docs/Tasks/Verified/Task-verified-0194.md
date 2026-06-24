# T-0194: Per-window project model — AppKit NSWindow per project

**Status:** ✅ Implemented - Verified
**Component:** `Scrivi/App/ProjectWindowManager.swift` (new); `ScriviApp.swift`, `AppEnvironment.swift`, `Info.plist`
**Epic:** EP-018
**Sprint:** SP-049
**Date Requested:** 2026-06-24
**Date Implemented:** 2026-06-24
**Date Verified:** 2026-06-24
**Design Reference:** `Scrivi_PerWindow_Project_Model_Design_v0_1.md` §3.2 (superseded — AppKit replaces WindowGroup(for:))

**Goal:**
Multiple distinct projects open at once, one per window (R1/R2); re-opening an open project focuses
its window rather than duplicating (R3); a Welcome window for the no-project state.

**Resolution (final architecture — diverged from the original WindowGroup(for:) plan):**
- **`WindowGroup(for:)` ABANDONED.** Evidence (live testing + os_log): it caches dismissed windows,
  so reopening a closed project rebound to a dead view whose `.task`/`.onAppear` never fired →
  permanent "Loading…". Replaced with **AppKit `NSWindow`/`NSWindowController` per project**
  (`ProjectWindowManager`): create on open, focus for R3, fully close+release on close. The
  `OpenProjectRegistry` (T-0193) remains the authoritative R3 guard.
- **URL delivery via `NSApplicationDelegate.application(_:open:)`** — per-view `.onOpenURL` was
  unreliable across the Window+WindowGroup split (deep links stopped arriving; zero `handle` log
  lines confirmed it).
- **`LSMultipleInstancesProhibited` = true** — single instance; a 2nd launch / deep link routes to
  the running process. (The earlier "distinct window groups" were separate processes.)
- **`applicationShouldTerminateAfterLastWindowClosed` = false** — app returns to Welcome instead of
  quitting when the last project window closes; quit via ⌘Q.
- **Menu state via `env.frontmostSession`** (set on `windowDidBecomeKey`) — AppKit windows don't
  feed SwiftUI `@FocusedValue`. Drives Close Project / Project Settings / View toggles on the
  frontmost window.
- **Welcome window:** closes when the first project opens; reopens (deferred + `NSApp.activate`)
  when the last project closes.
- **File menu:** New / Open / Close added; Project menu holds Project Settings. Timeline
  import/export deferred to a future task.
- New file: `ProjectWindowManager.swift`. Removed dead `ProjectWindowID.swift`.

**Verification (user, 2026-06-24, against the running app — extensive iterative testing):**
- R1/R2: two distinct projects open in two windows; no B-replaces-A. ✅
- R3: re-open an open project focuses its window, no duplicate. ✅
- Focused menus act on the frontmost window only. ✅
- Close (⌘W and red button) tears down cleanly; Welcome reappears when last closes; app does not
  quit. ✅
- Reopen-after-close loads cleanly (no "Loading…" hang). ✅
- Deep links open/focus the target window; single-instance enforced. ✅
- macOS build/codesign clean.

**Acceptance Criteria:**
- [x] R1/R2 — multiple distinct projects, one window each.
- [x] R3 — re-open focuses existing window (registry-authoritative).
- [x] Welcome window close-on-open / reopen-when-none.
- [x] Single instance; app survives last-window-close.
- [x] No regression to open/save/close; deep links work.

*Verified by the user 2026-06-24 via extensive manual testing.*
