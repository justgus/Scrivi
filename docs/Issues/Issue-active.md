# Active Issues

## SP-046 — EP-017 Spotlight Layer 2 + Windowing Polish

| ID | Title | Status |
| -- | ----- | ------ |
| I-0051 | Restored project windows don't remember per-window size/position (stack at default) | 🟢 Resolved - Not Verified |
| I-0052 | iOS target fails to build — `Window`/menu commands are macOS-only in `ScriviApp` | 🟢 Resolved - Not Verified |

---

## I-0052: iOS target fails to build — macOS-only scene/commands in `ScriviApp`

**Status:** 🟢 Resolved - Not Verified
**Platform:** iOS / iPadOS / visionOS
**Component:** `ScriviApp.swift`
**Severity:** High (target does not compile)
**Sprint:** SP-046
**Epic:** EP-018 follow-up (iOS window model was a deferred non-goal of EP-018)
**Date Identified:** 2026-06-25
**Date Implemented:** 2026-06-25
**Date Verified:** —

**Description:**
Building the `ScriviApp-iOS` target fails: `ScriviApp.swift:62 'Window' is unavailable in iOS`. The
EP-018 per-window rework built the entire scene model around macOS-only constructs — the SwiftUI
`Window` scene, menu `.commands` (`CommandGroup`/`CommandMenu`), and the AppKit `ProjectWindowManager`
— with no iOS scene path. iOS was an explicit EP-018 non-goal, but the shared `ScriviApp.swift` still
compiles on the iOS target and hits the unavailable APIs.

**Expected Behavior:**
`ScriviApp-iOS` (and `-visionOS`) compile and launch. iOS is single-window: one `WindowGroup` hosting
the editor for the active project, or the Landing view when none is open. No menu bar (iOS has none).

**Actual Behavior:**
iOS target does not compile (`'Window' is unavailable in iOS`, plus `.commands` and other macOS-only
references would follow).

**Date Identified:** 2026-06-25

**Root Cause Analysis:**
`ScriviApp.body` used `Window(... id: "welcome")` + `.commands { appCommands }` unconditionally;
`focusedSession`, `appCommands`, and `WelcomeWindowRoot` reference macOS-only APIs
(`env.windows` / `ProjectWindowManager`, `CommandGroup`, `dismissWindow(id:)`). None were
platform-guarded, so the iOS target compiled them and failed on the first unavailable symbol.

**Resolution (2026-06-25 — Implemented, Not Verified):**
Minimal "iOS compiles + runs" scope (full iOS multi-project UX still deferred):
1. **Scene split** in `ScriviApp.body` — `#if os(macOS)` keeps the `Window` + AppKit + `.commands`
   model unchanged; `#else` provides a single `WindowGroup { iOSRootView() }`.
2. **`iOSRootView`** (new, `#if !os(macOS)`) — shows `EditorView` (injecting `env.activeSession`) when a
   project is open, else `LandingView`.
3. **Shared `launchSetup()`** extracted (bootstrap + restore), called from both scenes' `.task`.
4. **macOS-only declarations guarded** — `focusedSession`, `appCommands`, and `WelcomeWindowRoot` are
   now `#if os(macOS)`. `AppEventsModifier` is iOS-safe (its `.onOpenURL`, `WindowFrameAutosave`
   already guarded; `openWindow(id:"welcome")` is a harmless no-op on iOS).

`requestOpenWindow` is already `#if os(macOS)`-internal (no-op on iOS); the iOS root view shows the
editor by observing `activeSession`.

**Follow-up (2026-06-25) — iOS open/create UX wired up.** First iOS launch reached the Welcome screen
but "Open Project" and the New Project "Choose…" did nothing (they called macOS-only NSOpenPanel/
NSSavePanel paths). Added the iOS-native pickers:
- **Open** (`LandingView`): `.fileImporter` for the `com.caposoft.scrivi.project` package type →
  new `AppEnvironment.openProjectFromPickedURL(_:)` (starts security-scoped access, `loadProject`,
  handles repairRequired). The editor appears via `activeSession` (no window needed).
- **New** (`NewProjectSheet`): `.fileImporter` for `.folder` picks a parent directory; Create composes
  `<dir>/<slug>.scrivi` via new `AppEnvironment.createProjectInPickedDirectory(_:title:slug:)`. macOS
  path (NSSavePanel) unchanged; sheet branches on `chosenPath` (macOS) vs `chosenDirectory` (iOS).
- `choosePath()` (NSSavePanel) now `#if os(macOS)`-guarded; `hasLocation` drives the Create button.
- **Known iOS limitation:** `ProjectBookmarkStore` is macOS-only (`.withSecurityScope` bookmarks), so
  iOS does not yet persist a reopen-from-Recent bookmark — the open works within the session (scope
  held live). iOS reopen/restore is a separate future concern, consistent with the deferred iOS
  window model.

**Deployment target raised (2026-06-25):** `IPHONEOS_DEPLOYMENT_TARGET` and `XROS_DEPLOYMENT_TARGET`
moved 26.6 → **27.0** (Debug + Release). The original 26.6 minimum exceeded the only installed SDK
(26.5), which itself blocked the build; 27.0 aligns with the installed iOS/visionOS 27.0 **runtimes**.
macOS deployment target left at 26.6 (unaffected).

**Verification status:**
- macOS `ScriviApp` still builds clean (no regression); pbxproj `plutil -lint` OK.
- **iOS/visionOS still cannot be built in THIS environment — one step remains:** the **27.0 SDK is not
  installed** (only the 27.0 *runtime* is; the installed SDK is 26.5). A 27.0 deployment target needs the
  27.0 SDK, so the build won't link until the **iOS/visionOS 27.0 platform/SDK** is installed via
  **Xcode ▸ Settings ▸ Components**.
- Strongest proxy run here: **`swiftc -typecheck` of the full app Swift source set against the iOS SDK
  passes with zero errors/warnings** — the `'Window' is unavailable` error and all other iOS-availability
  issues are gone. The remaining failure is purely the missing SDK, not the code.
- **Needs (user):** install the iOS/visionOS 27.0 SDK, then build `ScriviApp-iOS` and launch a 27.0
  simulator. Confirm: app launches to Landing; the active-session path shows the editor. (Same install
  also unblocks T-0123 / EP-012 iPhone verification.)

**Resolution:** Implemented (scene split + 27.0 deployment target); awaiting the 27.0 SDK install + iOS run.

---

## I-0051: Restored project windows don't remember per-window size/position (stack at default)

**Status:** 🟢 Resolved - Not Verified
**Platform:** macOS
**Component:** `ProjectWindowFrameStore.swift` (new), `ProjectWindowManager.swift` (`ProjectWindowController`)
**Severity:** Medium
**Sprint:** SP-046
**Epic:** EP-018 follow-up (per-window model polish), scheduled under EP-017 / SP-046
**Related:** I-0017 (single-window maximized-state restore — this is its multi-window successor)
**Date Implemented:** 2026-06-25
**Date Verified:** —

**Description:**
With multiple projects open (EP-018), each project window's size and on-screen position are not
persisted or restored across an app quit/relaunch. On relaunch the restore flow (R4) reopens every
previously-open project, but each new `NSWindow` is created at a hardcoded default rect and centered,
so all restored windows appear at the same default size stacked directly on top of one another —
losing the layout the user had arranged.

**Expected Behavior:**
On relaunch (and on reopening a previously-closed project), each project window reappears at the
**same size and screen position** — ideally also the same zoom/maximized state — it had when the app
was last quit / when that window was last closed. Two projects the user placed side-by-side should
return side-by-side, not stacked.

**Actual Behavior:**
Both (all) restored project windows open at the default `1100×700` size, centered, on top of each
other. The user must re-arrange them every launch.

**Steps to Reproduce:**
1. Open two projects.
2. Move and resize each window to a distinct location/size.
3. ⌘Q to quit.
4. Relaunch — both windows reopen at the default size, centered and overlapping.

**Impact:**
- Multi-project users lose their window arrangement on every launch — friction that undercuts the
  multi-window capability EP-018 just delivered.
- Single-project users also don't get size/position restore for the editor window.

**Date Identified:** 2026-06-25

**Root Cause Analysis:**
`ProjectWindowController.init` (`ProjectWindowManager.swift:80-88`) always creates the window with a
fixed `NSRect(0,0,1100,700)` and calls `window.center()`; there is no per-project frame persistence
or restore. The existing `WindowFrameAutosave` (`WindowFrameAutosave.swift`) does persist frame +
zoom, but (a) it stores a **single** frame under one key (`scrivi.mainWindow.frame`), not one per
project, and (b) it is attached only to the **SwiftUI Welcome window** (`.background(WindowFrameAutosave())`
in `ScriviApp.swift`) — the AppKit project windows never use it.

**Proposed Direction (for when scheduled):**
- Persist a **per-projectID frame** (and zoom/maximized flag) — e.g. keyed
  `scrivi.projectWindow.<projectID>.frame` in `UserDefaults`, written on
  `windowDidEndLiveResize` / `windowDidMove` / `windowWillClose`.
- In `ProjectWindowController`, if a saved frame exists for the projectID, apply it with
  `window.setFrame(_:display:)` instead of `window.center()`; fall back to the current default +
  light cascade for first-ever-open or off-screen frames.
- Consider folding I-0017's zoom-restore timing problem into the same fix (the zoom-applies-too-early
  issue may be moot for AppKit-owned windows since SwiftUI no longer drives their layout).
- Validate against multiple displays and the case where a saved frame is now off every screen
  (clamp back on-screen).

**Resolution (2026-06-25 — Implemented, Not Verified):**

1. **New `ProjectWindowFrameStore`** (`Scrivi/App/ProjectWindowFrameStore.swift`) — persists each
   project window's frame + zoom state in `UserDefaults`, keyed **per projectID**
   (`scrivi.projectWindow.<projectID>.frame` / `.zoomed`). Frame is recorded only while un-zoomed
   (preserves the un-zoomed size across zoomed sessions). `@MainActor` (touches `NSWindow`/`NSScreen`).
   Includes `clampedOnscreen(_:)` to re-anchor a frame whose display is no longer attached.
2. **`ProjectWindowController.init`** — applies the saved frame via `setFrame(_:display:)` instead of
   `window.center()`. For a first-ever open (no saved frame) it centers, then **cascades** off a shared
   anchor so multiple concurrently-opened windows step down/right rather than stacking exactly. Re-applies
   saved zoom after the window exists.
3. **Save hooks** — `windowDidEndLiveResize`, `windowDidMove`, `windowDidChangeOcclusionState`, and
   `windowWillClose` all persist the current frame/zoom. Quit-time routes through `windowWillClose` as
   each window closes, so the still-open layout is captured before teardown.

Swift-layer only; no ScriviCore change. New file wired into `project.pbxproj` (file ref `C055`, build
files `C055_BF`/`D055_BF`/`E055_BF`, `GRP_SOURCES` group, all three Sources phases). macOS build clean
(no warnings); interop 26/26; backend ctest 224/224.

**Also fixed in this pass (pbxproj integrity):** Xcode's "file format integrity" warning on
`Scrivi.xcodeproj/project.pbxproj` was traced to a leftover **"Recovered References"** group carrying a
foreign 24-hex UUID (`865E8CFE…`) injected by a past Xcode recovery — out of step with the file's
hand-authored short-ID style. Renamed the group to `GRP_SOURCES /* Sources */`. `plutil -lint` OK; build
clean. (No functional change — same children/refs.)

**Verification needed (manual macOS relaunch):**
1. Open two projects; move + resize each to distinct locations.
2. ⌘Q; relaunch → both windows return to their prior size/position (side-by-side, **not** stacked at default).
3. Maximize one, quit, relaunch → it returns maximized.
4. Reopen a previously-closed single project → it returns to its last frame.
5. (Edge) Save a frame on an external display, detach it, relaunch → window re-anchors on-screen.

---

*Last Updated: 2026-06-25 (I-0051 implemented — per-project window frame/zoom persistence; awaiting manual relaunch verification. pbxproj integrity warning also fixed.)*
