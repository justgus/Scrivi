# Active Issues

## SP-046 — EP-017 Spotlight Layer 2 + Windowing Polish

| ID | Title | Status |
| -- | ----- | ------ |
| I-0051 | Restored project windows don't remember per-window size/position (stack at default) | 🟢 Resolved - Not Verified |
| I-0052 | iOS target fails to build — `Window`/menu commands are macOS-only in `ScriviApp` | ✅ Resolved - Verified (2026-06-26) |
| I-0053 | iOS `ScriviEngine` is stubbed — ScriviCore not built/linked for iOS; all backend calls throw | 🟢 Resolved - Not Verified |
| I-0054 | iPad has no button bar (by design) and no iOS menu bar — Project Settings / Close Project unreachable on iPad | 🔴 Open |

---

## I-0054: iPad has no button bar and no iOS menu bar — Project Settings / Close Project unreachable on iPad

**Status:** 🔴 Open
**Platform:** iPadOS
**Component:** `Scrivi/Views/EditorView.swift` (`ManuscriptEditorView`), iOS scene/command model in `ScriviApp.swift`
**Severity:** High (no way to reach Project Settings or close a project on iPad)
**Sprint:** SP-046
**Epic:** EP-012 (toolbar/menu-bar split) — surfaced while verifying T-0123
**Related:** T-0123 (phone-idiom toolbar restore), I-0052 (iOS scene split — no menu bar on iOS)
**Date Identified:** 2026-06-28

**Description:**
EP-012 split project-chrome actions (Project Settings, Close Project) by idiom: macOS and **iPadOS**
were intended to use the **menu bar**, while iPhone keeps an in-view toolbar (T-0123, gated to
`userInterfaceIdiom == .phone`). But the iOS scene split (I-0052) established that **iOS/iPadOS have
no menu bar** — the macOS `.commands`/`CommandMenu` model is compiled out under `#if os(macOS)`. The
result is a coverage gap on **iPad specifically**:

- iPad is **not** `.phone`, so T-0123's `phoneToolbar` is hidden by its runtime idiom check.
- iPad has **no menu bar**, so the menu-bar route EP-012 assumed for iPad does not exist.

So on iPad there is currently **no UI affordance** to open Project Settings or Close Project once a
project is open.

**Expected Behavior:**
On iPad, Project Settings and Close Project are reachable through some always-available affordance
(e.g. the same in-view toolbar used on iPhone, a navigation-bar item, or a real iPad menu surface).

**Actual Behavior:**
On iPad, neither the phone toolbar (idiom-gated to `.phone`) nor a menu bar (doesn't exist on iOS)
is present — Project Settings and Close Project are unreachable.

**Root Cause Analysis:**
- T-0123 gates its toolbar to `UIDevice.current.userInterfaceIdiom == .phone`, deliberately excluding
  iPad on the assumption iPad would use the menu bar (`EditorView.swift`).
- I-0052's iOS scene split compiles the entire macOS menu model (`.commands`, `CommandGroup`,
  `CommandMenu`) out under `#if os(macOS)`; iOS/iPadOS have no menu bar to host those actions.
- The two changes individually are correct but jointly leave iPad with no host for these actions.

**Proposed Direction (for when scheduled):**
Decide the iPad affordance and implement it. Options:
1. **Broaden T-0123's toolbar to iPad** — drop the `.phone`-only gate so the in-view toolbar shows on
   any iOS idiom (simplest; one-line change to the runtime check). Re-evaluate visual placement on the
   larger iPad canvas.
2. **iPad navigation-bar items** — surface Project Settings / Close Project as `ToolbarItem`s in the
   editor's `.toolbar`, idiom-appropriate for iPad.
3. **iPad menu surface** — a dedicated overflow/ellipsis menu in the nav bar.
Recommendation: option 1 for the minimum-to-reachable scope, since the toolbar already exists and is
verified-pending on iPhone (T-0123).

**Impact:**
- Blocks the iPad arm of the EP-012 close-out (a minimal working iPad version requires reachable
  Project Settings / Close Project).
- Does not affect iPhone (toolbar present) or macOS (menu bar present).

---

## I-0052: iOS target fails to build — macOS-only scene/commands in `ScriviApp`

**Status:** ✅ Resolved - Verified
**Platform:** iOS / iPadOS / visionOS
**Component:** `ScriviApp.swift`
**Severity:** High (target does not compile)
**Sprint:** SP-046
**Epic:** EP-018 follow-up (iOS window model was a deferred non-goal of EP-018)
**Date Identified:** 2026-06-25
**Date Implemented:** 2026-06-25
**Date Verified:** 2026-06-26 (user-confirmed: iOS 27.0 SDK installed, `ScriviApp-iOS` builds clean and launches to the Welcome/Landing screen on a 27.0 simulator — this Issue's acceptance bar)

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
- **CORRECTION (2026-06-26):** The earlier claim that the **iOS/visionOS 27.0 SDK was not installed**
  (and that the installed SDK was 26.5) was **wrong** — a stale environment observation. The user
  confirms the **iOS 27.0 SDK IS installed**, `ScriviApp-iOS` **builds clean with no errors**, and the
  app **launches to the Welcome/Landing screen** in a 27.0 simulator. App launch succeeds. The scene
  split (this Issue's actual fix) therefore works as intended — strike the SDK-blocker note.
- **What this Issue's fix did NOT cover:** linking the ScriviCore backend into the iOS target. Booting
  to Landing needs no backend; the first engine call (`bootstrap → ensureLocalIdentity`) throws
  `"ScriviCore not available on this platform"` because `ScriviEngine` compiles as its iOS stub
  (`#else` block) — ScriviCore isn't built/linked for iOS. That is a **separate fault tracked as I-0053**,
  not a regression in this scene-split fix.

**Resolution:** Implemented (macOS/iOS scene split + iOS pickers + deployment target). Builds clean and
launches to Landing on the iOS 27.0 simulator (user-observed). Awaiting user's **Verified** sign-off. The
backend-not-linked runtime fault is split out to **I-0053**.

---

## I-0053: iOS `ScriviEngine` is stubbed — ScriviCore not built/linked for iOS; all backend calls throw

**Status:** 🟢 Resolved - Not Verified
**Platform:** iOS / iPadOS (visionOS still stubbed — separate future item)
**Component:** `ScriviEngine.swift`, `Scrivi.xcodeproj/project.pbxproj` (build graph), `ScriviCore/src/util/Process.cpp`
**Severity:** High (no project can be created or opened on iOS; the app is non-functional past Landing)
**Sprint:** SP-046
**Epic:** Successor to I-0052; blocker for EP-012 / T-0123 (iPad/iPhone button-bar verification) and relevant to EP-017 / T-0190 (iOS assessment)
**Date Identified:** 2026-06-26
**Date Implemented:** 2026-06-26
**Date Verified:** —

**Description:**
On the iOS target, every `ScriviEngine` method throws `"ScriviCore not available on this platform"`.
The app builds and launches to the Welcome/Landing screen (no backend needed there), but the first
call into the engine fails, so identity bootstrap and project create/open are impossible.

**Expected Behavior:**
`ScriviApp-iOS` reaches the editor: identity bootstraps, and creating/opening a `.scrivi` project
succeeds via the real ScriviCore backend — same as macOS.

**Actual Behavior:**
- Console at launch: `[Scrivi] Bootstrap failed: ScriviCore not available on this platform`.
- Tapping **Create** in the New Project sheet reports **"Identity not bootstrapped"** (the
  `authorshipRef == nil` guard in `AppEnvironment.createProject`, because `bootstrap()` threw).

**Steps to Reproduce:**
1. Build & run `ScriviApp-iOS` on an iOS 27.0 simulator. App launches to Landing/Welcome.
2. New Project → enter a title → Choose… a folder → tap **Create**.
3. Error: "Identity not bootstrapped". (Console shows the bootstrap failure at launch.)

**Impact:**
- iOS/iPadOS app is unusable past the Landing screen.
- Blocks on-iPad UI verification (button bar / editor), which gates **EP-012 / T-0123** and, in turn,
  the close of **EP-012**.

**Root Cause Analysis:**
- `ScriviEngine.swift` gates the real engine behind `#if os(macOS)` (lines 2–4, 9); the iOS build
  compiles the `#else` **stub** (lines 824–890), whose `unavailable()` throws the observed string
  (line 834). This is by design "so the codebase compiles without ScriviCore on iOS" (file comment).
- **ScriviCore is not built or linked for iOS.** The macOS app links a CMake-built
  `build/ScriviCore/libScriviCore.a` (`SCRIVI_CORE_LIB`, produced by the `Build ScriviCore (CMake)`
  shell phase running `cmake -S . -B build`) and imports the C ABI via
  `ScriviCore/include/scrivi/module.modulemap`. That archive is a **macOS-host** build — wrong
  architecture for iOS/iOS-simulator.
- The iOS target's Frameworks phase (`D_FW`) is **empty** (no `libScriviCore.a`), though its
  `LIBRARY_SEARCH_PATHS`/`SWIFT_INCLUDE_PATHS` were pre-pointed at the macOS build dir — wiring was
  started and left incomplete.

**Proposed Direction (in-project static lib; minimum-to-button-bar scope):**
1. Add CMake invocation(s) producing iOS-device (arm64) and iOS-simulator slices of
   `libScriviCore.a` into per-SDK build dirs, mirroring the existing `Build ScriviCore (CMake)` phase.
2. Wire the correct slice into the iOS target's `D_FW` Frameworks phase, with
   `LIBRARY_SEARCH_PATHS` resolved per-SDK (`[sdk=iphoneos*]` / `[sdk=iphonesimulator*]`).
   Update `project.pbxproj` in the same step (CLAUDE.md).
3. Broaden the engine gate: `#if os(macOS)` → `#if os(macOS) || os(iOS)` (incl. `import ScriviCore`)
   so the real engine compiles for iOS. Required C ABI symbols
   (`scrivi_ensure_local_identity`, `scrivi_create_project`, `scrivi_open_project`,
   `scrivi_open_scene`) are present in `scrivi.h`.
4. Verify on the iOS 27.0 simulator: `[Scrivi] Identity:` prints; create-into-picked-folder succeeds;
   editor + button bar render.

**Known limitations / out of scope for this round:**
- Git is unavailable on iOS — already handled (`SystemGitProvider.available()` returns false; create
  only runs git when `enableGitSnapshots` is set).
- The runtime `PrototypeSecureStore` (in `scrivi_c_api.cpp`) is **in-memory only**, so identity won't
  persist across launches on iOS. Acceptable for button-bar verification; track a real Keychain/file
  store as a separate follow-up.
- visionOS (`E_FW`) left untouched this round (scoped to iPad button-bar verification).

**Resolution (2026-06-26 — Implemented, Not Verified):**

1. **`ScriviCore/src/util/Process.cpp`** — guarded the subprocess path for embedded Apple
   platforms. `std::system`/`popen`/`pclose` are marked **unavailable** in the iOS SDK; added a
   `SCRIVI_NO_SUBPROCESS` macro (`__APPLE__ && TARGET_OS_IPHONE`) under which `executableInPath`
   returns `false` and `runProcess` returns a graceful "not available" failure. No behavior change on
   macOS/Linux/Windows. This is the **only** source-level iOS incompatibility in ScriviCore; git is
   already a no-op on iOS (`SystemGitProvider::available()` → false; create only shells out when
   `enableGitSnapshots` is set).

2. **`Scrivi.xcodeproj/project.pbxproj`** — new build phase **`D_CMAKE_PHASE` ("Build ScriviCore for
   iOS (CMake)")** added as the iOS target's first phase. It cross-builds ScriviCore against the
   SDK Xcode selected (`$SDKROOT`/`$ARCHS`/`$IPHONEOS_DEPLOYMENT_TARGET`) into a per-platform dir
   `build-$(PLATFORM_NAME)/` (so simulator and device slices don't collide). The iOS Debug/Release
   configs (`D_BC_DEBUG`/`D_BC_RELEASE`) gained `LIBRARY_SEARCH_PATHS =
   $(SRCROOT)/build-$(PLATFORM_NAME)/ScriviCore`, `SWIFT_INCLUDE_PATHS` to the modulemap, and
   `OTHER_LDFLAGS = -lScriviCore -lc++`. Mirrors the macOS `C_CMAKE_PHASE` mechanism (no `.xcframework`).

3. **`Scrivi/Engine/ScriviEngine.swift`** — broadened the gate `#if os(macOS)` → `#if os(macOS) ||
   os(iOS)` (incl. `import ScriviCore`), so iOS compiles the real engine instead of the `#else` stub.
   visionOS still falls through to the stub (future item).

4. **`.gitignore`** — added `/build-*/` so the per-platform iOS build dirs aren't tracked.

**Files Affected:**
- `ScriviCore/src/util/Process.cpp`
- `Scrivi.xcodeproj/project.pbxproj`
- `Scrivi/Engine/ScriviEngine.swift`
- `.gitignore`

**Verification performed (in the dev environment, Xcode-beta / iOS 27.0 SDK):**
- ScriviCore cross-builds clean for both **iOS simulator** (arm64 + x86_64) and **iOS device**
  (arm64) against the 27.0 SDK.
- `xcodebuild ScriviApp-iOS -sdk iphonesimulator` → **BUILD SUCCEEDED**; app links
  `-lScriviCore` from `build-iphonesimulator/ScriviCore`.
- Built `Scrivi.app` (iOS-sim, minos/sdk 27.0): the stub string `"ScriviCore not available on this
  platform"` is **absent** from the binary (`strings | grep -c` → 0) — proves the real engine, not
  the stub, is compiled in.
- No regression: backend **ctest 224/224 pass**; macOS `ScriviApp` still **BUILD SUCCEEDED**.

**Verification still needed (USER — requires a live iOS 27.0 simulator/device run):**
- This dev environment has the iOS **27.0 SDK** (builds) but only **26.3.1 simulator runtimes**
  (cannot boot the 27.0 app). The user's machine has a 27.0 sim runtime (their console showed one).
- On an iPad 27.0 simulator: launch `ScriviApp-iOS`; confirm console prints **`[Scrivi] Identity:`**
  (not "Bootstrap failed"); New Project → choose a folder → **Create** succeeds (no "Identity not
  bootstrapped"); the **editor and its button bar render**. That observation unblocks **EP-012 / T-0123**.

**Known limitation (out of scope, track separately):** the runtime `PrototypeSecureStore`
(`scrivi_c_api.cpp`) is in-memory only, so the iOS author identity regenerates each launch and does
not persist (no Keychain yet). Acceptable for button-bar verification.

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

*Last Updated: 2026-06-28 (I-0054 filed — iPad has no button bar and no iOS menu bar, so Project Settings / Close Project are unreachable on iPad; surfaced while verifying T-0123. EP-012 iPad arm blocked until an iPad affordance is chosen.)*
