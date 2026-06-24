# Sprint SP-049: Per-Window Model — Windowing & Restore-All-Windows

**Status:** ✅ Closed
**Epic:** EP-018
**Start Date:** 2026-06-24
**End Date:** 2026-06-24
**Goal:** Turn the SP-048 foundation into real per-window behavior — one window per project, with
the registry as the authoritative R3 guard — then restore all previously-open project windows on
relaunch (R4). No ScriviCore/C++ changes.

### Tasks

| ID | Title | Status |
| -- | ----- | ------ |
| T-0194 | Per-window project model — **AppKit NSWindow** per project (R1/R2/R3) + Welcome window; single-instance; File menu | ✅ Verified |
| T-0195 | Session manifest persistence + launch restore of all previously-open windows (R4) | ✅ Verified |

### Acceptance Criteria

- [x] Multiple distinct projects open simultaneously, one per window (R1/R2).
- [x] Re-opening an already-open project focuses its existing window rather than duplicating (R3), via the OpenProjectRegistry (authoritative guard).
- [x] On relaunch, **all** previously-open project windows are restored (R4); per-window scene/cursor/scroll restored by the existing backend open flow.
- [x] macOS build/codesign clean; no regression to open/save/close; deep links work.

### Outcome — final architecture (diverged from the WindowGroup(for:) plan)

The sprint's original plan was `WindowGroup(for: ProjectWindowID.self)`. Extensive iterative testing
against the running app (with os_log evidence) proved that approach unworkable and it was replaced:

- **`WindowGroup(for:)` ABANDONED** — it caches dismissed windows; reopening a closed project hung
  on "Loading…". Replaced with **AppKit `NSWindow`/`NSWindowController` per project**
  (`ProjectWindowManager`) — deterministic create/focus/close. Design doc §3.2 marked superseded.
- **URL delivery via `NSApplicationDelegate.application(_:open:)`** — per-view `.onOpenURL` was
  unreliable across the Window+WindowGroup split.
- **`LSMultipleInstancesProhibited`** — single instance (the "distinct window groups" the tester saw
  were separate processes).
- **`applicationShouldTerminateAfterLastWindowClosed = false`** — app returns to Welcome instead of
  quitting on last-window-close.
- **Menu state via `env.frontmostSession`** (AppKit windows don't feed SwiftUI `@FocusedValue`).
- **Welcome window** close-on-open / reopen-when-none.
- **File menu** New/Open/Close. Timeline import/export deferred.
- **T-0195 quit-time manifest freeze** — `applicationWillTerminate` freezes the open set before the
  quit-time close cascade can wipe it.

New files: `ProjectWindowManager.swift`, `OpenSessionManifest.swift`. Removed dead
`ProjectWindowID.swift`.

### Retrospective

**Completed:**
- Full per-window/per-project model working: R1–R4 verified by the user against the running app.
- Single-instance, Welcome lifecycle, focused menus, File menu, restore-all-windows.

**What went well:**
- Evidence-driven debugging (os_log + container plist inspection) repeatedly turned vague symptoms
  ("two window groups", "Loading…", "Welcome doesn't reappear") into exact root causes, each with a
  targeted fix. The decision to abandon `WindowGroup(for:)` for AppKit was made on evidence, not
  guesswork.

**What was harder than expected:**
- SwiftUI `WindowGroup(for:)` multi-window behavior (window caching, unreliable lifecycle callbacks,
  per-view URL delivery) cost several iterations before the AppKit pivot. The V1 spike (T-0191)
  caught the de-dup race but not the caching/lifecycle issues — those only surfaced in full
  multi-window runtime testing.

**Carried into SP-050:**
- T-0196 — rewrite the EP-017 deep-link handler on this model (largely already done as part of
  T-0194's handleDeepLink); scene-ID fix; open-flow doc cross-ref; EP-018 verification & close.

---

*Closed 2026-06-24 with user approval. EP-018 continues with SP-050.*
