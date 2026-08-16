# Verified Issues — I-0051 to I-0060


---

## I-0051: Restored project windows don't remember per-window size/position (stack at default)

**Status:** ✅ Resolved - Verified (2026-06-29, user-confirmed on macOS — windows restore size/position perfectly across relaunch)
**Platform:** macOS
**Component:** `ProjectWindowFrameStore.swift` (new), `ProjectWindowManager.swift` (`ProjectWindowController`)
**Severity:** Medium
**Sprint:** SP-046
**Epic:** EP-018 follow-up (per-window model polish), scheduled under EP-017 / SP-046
**Related:** I-0017 (single-window maximized-state restore — this is its multi-window successor); I-0055 (restored-maximized state defect — carved out below)
**Date Implemented:** 2026-06-25
**Date Verified:** 2026-06-29

> **Verified scope:** un-zoomed size/position restore (incl. two side-by-side projects returning
> side-by-side, reopen-of-closed-project, and off-screen re-anchor) is user-confirmed working. The
> one remaining defect — a window quit while **maximized** reopens *filling the screen but not in the
> true zoomed state* — is split out to **I-0055** and is NOT part of this Issue's verified scope.

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

---

## I-0052: iOS target fails to build — macOS-only scene/commands in `ScriviApp`

**Status:** ✅ Resolved - Verified · ⚪ **OBE (2026-07-07)** — the "macOS deployment target left at
26.6" detail below is superseded: **all app targets (macOS/iOS/visionOS) and `ScriviInteropTests` now
deploy to 27.0** (raised during SP-052 / T-0203, user-directed). CLAUDE.md updated to 27.0. The
scene-split fix itself is unchanged and remains Verified; only the 26.6 deployment note is obsolete.
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
macOS deployment target left at 26.6 (unaffected). **[OBE 2026-07-07: macOS raised to 27.0 too;
all targets incl. `ScriviInteropTests` now 27.0 — see the OBE note in this Issue's status.]**

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

---

## I-0053: iOS `ScriviEngine` is stubbed — ScriviCore not built/linked for iOS; all backend calls throw

**Status:** ✅ Resolved - Verified (2026-06-29, user-confirmed on iPhone 17 Pro + iPad Pro / iOS 27.0)
**Platform:** iOS / iPadOS (visionOS still stubbed — separate future item)
**Component:** `ScriviEngine.swift`, `Scrivi.xcodeproj/project.pbxproj` (build graph), `ScriviCore/src/util/Process.cpp`
**Severity:** High (no project can be created or opened on iOS; the app is non-functional past Landing)
**Sprint:** SP-046
**Epic:** Successor to I-0052; blocker for EP-012 / T-0123 (iPad/iPhone button-bar verification) and relevant to EP-017 / T-0190 (iOS assessment)
**Date Identified:** 2026-06-26
**Date Implemented:** 2026-06-26
**Date Verified:** 2026-06-29

> **Verified (2026-06-29):** Confirmed via this session's live iOS runs — the console logged
> `[Scrivi] Identity: identity_019f13…` (the real ScriviCore backend bootstrapped, not the stub's
> "ScriviCore not available" throw), and new projects were created and opened on the iOS simulator
> ("The Majestic Horse of Time", "The Tattered Kingdom of Thieves") with the editor rendering. Met on
> **both** iPhone 17 Pro and iPad Pro / iOS 27.0 (exceeds the iPad-only bar). The whole T-0123 / I-0054
> iOS UI verification was only possible because the backend is linked. visionOS remains stubbed
> (separate future item); the in-memory `PrototypeSecureStore` (identity not persisted across launches)
> remains a tracked-separately limitation.

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

---

## I-0054: iPad has no button bar and no iOS menu bar — Project Settings / Close Project unreachable on iPad

**Status:** ✅ Resolved - Verified (2026-06-29, user-confirmed on iPad Pro / iOS 27.0)
**Platform:** iPadOS
**Component:** `Scrivi/Views/EditorView.swift`, `Scrivi/App/ScriviApp.swift` (`iosCommands`), `AppEnvironment.swift`, `LandingView.swift`
**Severity:** High (no way to reach Project Settings or close a project on iPad)
**Sprint:** SP-046
**Epic:** EP-012 (toolbar/menu-bar split) — surfaced while verifying T-0123
**Related:** T-0123 (phone-idiom toolbar restore), I-0052 (iOS scene split — no menu bar on iOS)
**Date Identified:** 2026-06-28
**Date Verified:** 2026-06-29

**Resolution (2026-06-29 — Verified):** Fixed as part of the T-0123 iOS Master/Detail rework (see
that Task's 2026-06-29 Resolution addendum). iPad now has two reachable surfaces:
1. **Nav-bar `•••` menu** on the editor detail (Project Settings, Show Inspector, Show Timeline,
   Close Project) — present on both iPhone and iPad.
2. **iPad hardware-keyboard menu bar** (`ScriviApp.iosCommands`) — File (New/Open/Close) and Project
   (Settings, Show Inspector ⇧⌘I, Show Timeline ⇧⌘T), deconflicted against the iOS-synthesized
   menus (no ⌘, no duplicate "View" menu, ⇧⌘ toggle shortcuts).
User-confirmed on iPad Pro / iOS 27.0: menus populated and functional, no menu-conflict console spam.

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

---

## I-0055: Restored-maximized project window fills the screen but is not truly zoomed

**Status:** ✅ Resolved - Verified (2026-06-29, user-confirmed on macOS — a project quit/closed in Full Screen reopens in Full Screen)
**Platform:** macOS
**Component:** `ProjectWindowManager.swift` (`ProjectWindowController` — full-screen restore + delegate callbacks), `ProjectWindowFrameStore.swift`
**Severity:** Low
**Sprint:** SP-046
**Epic:** EP-018 follow-up (per-window model polish)
**Related:** I-0051 (parent — windowed frame restore, Verified 2026-06-29); I-0017 (single-window maximized-state restore)
**Date Identified:** 2026-06-29
**Date Implemented:** 2026-06-29
**Date Verified:** 2026-06-29

**Description:**
When a project window is put into **macOS Full Screen** (the green traffic-light button, clicked with
no tiling option selected from its hover menu — the menu bar auto-hides and the window moves to its
own Space) and the app is then quit, on relaunch the window did **not** return to Full Screen — it
reopened windowed (and an earlier attempt left it merely *filling the screen* without truly being
maximized). (Toggling full screen *within* a single session worked; the defect was across quit/relaunch.)

**Expected Behavior:**
A window quit in Full Screen reopens **genuinely in Full Screen**. Exiting Full Screen returns the
window to the size/position it had **before** entering Full Screen. "Full Screen" is treated as a
dimensionless binary: the window's dimensions are not recorded while it is full screen; only the
binary flag is persisted.

**Actual Behavior:**
The restored window did not re-enter Full Screen (or filled the screen without being truly full
screen); the saved windowed dimensions were also at risk of being overwritten by the full-screen
geometry.

**Steps to Reproduce:**
1. Open a project; click the green button so it enters Full Screen.
2. ⌘Q to quit.
3. Relaunch → the window does not return to Full Screen.

**Root Cause Analysis:**
The green button enters macOS **Full Screen**, not Zoom. The earlier implementation keyed on
`NSWindow.isZoomed` (which read false right after the action, so the state was never persisted) and
then on a frame-vs-`visibleFrame` geometry check (which would also misclassify a window the user
manually resized to fill the screen). The reliable signal is `styleMask.contains(.fullScreen)` plus
the full-screen delegate callbacks. See the Diagnosis history + Fix below.

**Diagnosis history (2026-06-29):**
1. *First attempt (zoom-timing):* deferred `window.zoom(nil)` from init to `showAndFocus()`. No
   change in behavior.
2. *Second attempt (NSWindow.zoom / isZoomed):* logging revealed the real fault was **save-side** —
   after maximizing, the relaunch logged `savedZoomed=false`. `NSWindow.isZoomed` read **false**
   right after a green-button "maximize", so the state was never persisted. Switched detection to a
   geometry check (frame ≈ `visibleFrame`).
3. *Correction (the actual macOS behavior):* the geometry check is **wrong** — it would flag a
   window the user manually resized to fill the screen as "maximized." More fundamentally, the
   green button (clicked with no tiling option from its hover menu) does **not** zoom; it enters
   **macOS Full Screen** — the menu bar auto-hides and the window moves to its own Space. That is a
   distinct, *deterministic* state: `NSWindow.styleMask.contains(.fullScreen)` plus the
   `windowWillEnter/DidEnter/WillExit/DidExitFullScreen` delegate callbacks. (The green-button hover
   menu's tiling options — quadrants/halves/thirds, "Maximize and center" — leave the menu bar/Space
   alone and are ordinary frame changes; only true Full Screen is the dimensionless binary.)

**Fix (2026-06-29):**
- `ProjectWindowFrameStore` now persists a **`fullScreen`** flag via
  `window.styleMask.contains(.fullScreen)` (key `scrivi.projectWindow.<id>.fullScreen`), and writes
  the windowed frame **only while not full screen** — so quitting in full screen preserves the
  pre-full-screen size/position.
- `ProjectWindowController` restores via `window.toggleFullScreen(nil)` in `showAndFocus()` (real
  full-screen transition), and uses the full-screen delegate callbacks
  (`windowDidEnter/DidExitFullScreen`) to persist the flag, with an `isTransitioningFullScreen`
  guard so the transient resize doesn't overwrite the windowed frame.
- Diagnostic logging added (`init … savedFullScreen=…`, `restore-fullscreen requested`,
  `didEnter/ExitFullScreen`, `windowWillClose … fullScreen=…`).

**Verification needed (USER, macOS):**
1. Open a project; click the green button so it enters Full Screen (menu bar hides, own Space). ⌘Q.
2. Relaunch → window returns **genuinely in Full Screen** (not just screen-filling). Log:
   `init … savedFullScreen=true`.
3. Exit Full Screen (Ctrl-⌘-F or the green button) → window returns to its **pre-full-screen** size/position.
4. **Regression guard:** manually resize a window to fill the screen (NOT full screen — menu bar
   stays), quit, relaunch → it restores at that manual size and is **not** treated as full screen.

**Impact:** Cosmetic/behavioral nit; does not affect windowed size/position restore (I-0051, Verified).

---

---

## I-0056: macOS File ▸ Open Project panel won't let you select the `.scrivi` package

**Status:** ✅ Resolved - Verified (2026-06-29, user-confirmed on macOS — File ▸ Open Project selects `.scrivi`, matching the Welcome screen)
**Platform:** macOS
**Component:** `AppEnvironment.swift` (`presentOpenProjectPanel`)
**Severity:** Medium (no way to open an existing project from the menu when no project is open / via File ▸ Open)
**Sprint:** SP-046
**Epic:** EP-012 follow-up (the menu File ▸ Open path) / per-window model
**Related:** Welcome screen open path (`LandingView` `.fileImporter`) — the inconsistent counterpart
**Date Identified:** 2026-06-29
**Date Implemented:** 2026-06-29
**Date Verified:** 2026-06-29

**Description:**
The macOS **File ▸ Open Project…** menu command opens an `NSOpenPanel` that **does not allow selecting
the `.scrivi` project folder itself** — the panel traverses *into* the `.scrivi` package instead of
treating it as a selectable item, so the user cannot actually pick a project. This is inconsistent
with the **Welcome screen's** open button, which uses a `.fileImporter` that correctly presents
`.scrivi` as one selectable item.

**Expected Behavior:**
File ▸ Open Project lets the user select a `.scrivi` project as a single item — the same selection
behavior as the Welcome screen.

**Actual Behavior:**
The panel set `canChooseFiles = false`, `canChooseDirectories = true`, and **no `allowedContentTypes`**.
Because `.scrivi` is registered as a package UTI (`com.caposoft.scrivi.project`), the panel showed it
as an opaque package and, with files not choosable, it could not be selected — only navigated into.

**Steps to Reproduce:**
1. With a project open (or from any state where the menu bar is available), choose File ▸ Open Project…
2. Navigate to a `.scrivi` project folder.
3. The panel won't let you select it (it drills into the package); only plain folders are selectable.

**Root Cause:**
`NSOpenPanel` defaulted to treating the registered `.scrivi` package as opaque, with `canChooseFiles`
off and no `allowedContentTypes`, so the package was neither selectable as a file nor as a folder.

**Resolution (2026-06-29 — Implemented, Not Verified):**
`presentOpenProjectPanel` now configures the panel to match the Welcome screen's `.fileImporter`:
- `allowedContentTypes = [UTType("com.caposoft.scrivi.project") ?? .package]`
- `canChooseFiles = true`, `canChooseDirectories = true` (folder fallback if the UTI registration is missing)
- `treatsFilePackagesAsDirectories = false` so the `.scrivi` package is chosen as a single unit.
Added `import UniformTypeIdentifiers` (macOS-guarded).

**Verification needed (macOS):**
1. File ▸ Open Project… → navigate to a `.scrivi` project → it is **selectable as one item**; choosing it opens the project.
2. The behavior matches the Welcome screen's open button.
3. A plain (non-`.scrivi`) folder is not offered as a valid project (or is handled gracefully).

---

*Last Updated: 2026-07-15 (I-0065 + I-0066 both Resolved - Verified on macOS — user confirmed "The Twisted Remains of Myself" opens on a rebuilt app, its history log self-heals after one open, and navigator scene/chapter deletes record a `sceneDelete` barrier. I-0066 (durable root cause): a navigator delete left orphaned history in the log; fix = `sceneDelete` barrier before the navigator delete (Apple) + load-time `pruneInconsistentNodes()` that drops any node whose diff can't match its scene and persists a `ctl:purge` so the log self-heals on next open (ScriviCore). I-0065 (immediate crash stop): clamp-safe `applyForward`/`applyReverse` + a `guarded()` C ABI wrapper on `scrivi_history_open`/`_undo`/`_redo`. Suite 268/268 green. Earlier: I-0055/I-0056 Resolved - Verified on macOS.)*

---

## I-0057: Spotlight on-disk importer (`CSImportExtension`) never runs on macOS — Layer 2 descoped to the in-app `CSSearchableIndex` donor

**Status:** ✅ Resolved - Verified (2026-07-01, user-approved: descope to Option B accepted; on-disk `CSImportExtension` removed and the macOS app builds/signs clean without it)
**Platform:** macOS (Apple platforms generally; `CSImportExtension` is non-functional on macOS)
**Component:** `ScriviSpotlightImporter/ImportExtension.swift` (extension, to be removed), `ScriviSpotlightImporter.appex` target, `Scrivi.xcodeproj/project.pbxproj`; unaffected survivor: `Scrivi/App/SpotlightDonor.swift` (Layer 1 in-app donor)
**Severity:** High (the entire SP-046 deliverable — on-disk indexing — cannot work as built; the feature must be re-architected or descoped)
**Sprint:** SP-046
**Epic:** EP-017 (Spotlight) — this is the Layer 2 on-disk importer
**Related:** SP-046 T-0185/T-0186/T-0187/T-0188 (all built against the non-functional API); Layer 1 in-app donor (`SpotlightDonor.swift`) is the adopted replacement
**Date Identified:** 2026-07-01
**Date Implemented (decision):** 2026-07-01

**Description:**
The SP-046 on-disk Spotlight importer is implemented as a modern `CSImportExtension`
(`class ImportExtension: CSImportExtension`). The extension builds, codesigns cleanly, and is embedded
in the app, but at invocation time the OS spawns the extension process and then immediately tears it
down before it vends its XPC service — so `.scrivi` packages are never indexed on disk. Extensive
diagnosis (below) shows this is **not** a bug in our code, our link, our signature, or our
entitlements: **`CSImportExtension` is non-functional on macOS**, confirmed by Apple DTS, and this has
not changed through macOS 26 Tahoe.

**Expected Behavior:**
The OS launches the importer extension to index a `.scrivi` package on disk even when Scrivi is not
running; project/scene/object content becomes findable in Spotlight (SP-046 acceptance criteria).

**Actual Behavior:**
The extension process launches (observed pid) then dies before vending its XPC service. Host-side log:
`apple-extension-service was invalidated: Connection init failed at lookup with error 3 - No such
process`; `Unable to setup extension context - error: Couldn't communicate with a helper application.
Code=4099`; `xpc_error=[3: No such process]`. Nothing is ever indexed. **No crash report is generated
and the extension emits no log lines under its own process name** — because it is not crashing; the
system never drives a `CSImportExtension` at all.

**Steps to Reproduce:**
1. Build & run `Scrivi.app` (macOS) with the embedded `ScriviSpotlightImporter.appex`.
2. Trigger indexing of a `.scrivi` package (e.g. `mdimport -d3` the package, or let Spotlight scan).
3. The extension process appears and dies; host log shows the XPC `error 3 / Code=4099` teardown; the
   package is not indexed.

**Impact:**
- The SP-046 deliverable (on-disk indexing while Scrivi is not running) **cannot work as built** on macOS.
- SP-046 tasks T-0185–T-0188 targeted a non-functional API; the extension target is dead weight.
- Layer 1 (in-app `CSSearchableIndex` donor, `SpotlightDonor.swift`) is **unaffected** and remains the
  supported path for making Scrivi content findable.

**Root Cause Analysis:**
The code uses the modern `CSImportExtension` API, delivered via ExtensionKit. Per Apple DTS engineer
**Kevin Elliott**: *"CSImportExtension does not function on macOS and never has. Multiple bugs have
been filed on both the extension point and the documentation, but until something changes in the
system, the only option is to use the old MDImport API."* The macOS ExtensionKit host looks up /
spawns the service but never actually drives a `CSImportExtension`, so the process is invalidated
(`error 3 - No such process`). This is still the case on **macOS 26 Tahoe** (confirmed 2026-07-01);
Tahoe's Spotlight overhaul did not restore the extension point. The (unreliable) lowercase-`CSSupportedContentTypes`
workaround reported for macOS 15 does not apply here — our UTI `com.caposoft.scrivi.project` is already
lowercase.

**Diagnosis evidence (2026-07-01 — this is not our bug):**
- **No crash report anywhere** (`~/Library/Logs/DiagnosticReports`, `/Library/…`, `Retired/`) — no
  `.ips` for `ScriviSpotlightImporter`; and `log show --predicate 'process == "ScriviSpotlightImporter"'`
  is empty. The process never emits a single line under its own identity → not a runtime crash, a
  service-activation teardown.
- **Binary is healthy:** `otool -L` on the embedded appex shows only system dylibs (`libc++`,
  Foundation, libobjc, libSystem, CoreSpotlight, Swift runtime) plus the cleanly *statically*-linked
  `libScriviCore.a` — no dyld/"Library not loaded" risk. Earlier hypothesis (unloadable static lib /
  C++/Swift runtime miss) is **falsified**.
- **Signature/entitlements are fine:** `codesign -vvv --deep --strict` → "valid on disk", satisfies its
  Designated Requirement; hardened runtime on. Entitlements = app-sandbox + user-selected.read-only +
  get-task-allow — the same `get-task-allow=true` the **host app** carries, and the host app runs; so
  that is not the killer.
- **Info.plist mismatch is secondary:** the built plist uses the legacy `NSExtension` /
  `NSExtensionPointIdentifier = com.apple.spotlight.import` / `NSExtensionPrincipalClass` block while
  the code is a modern ExtensionKit `CSImportExtension`. But per DTS this is moot — no plist shape
  (`NSExtension` or `EXAppExtensionAttributes`) makes `CSImportExtension` run on macOS.

**Resolution (2026-07-01 — Decision: Option B, adopted; implementation pending):**
Descope Layer 2's on-disk `CSImportExtension`. Rely on the supported **Layer 1 in-app
`CSSearchableIndex` donor** (`SpotlightDonor.swift`), which Scrivi already has and which uses a
100%-supported API. Trade-off accepted: content is findable once the app has opened/edited a project
(the donor runs in-app), but **not** while the app has never been opened. This is the pragmatic path
given DTS calls the on-disk extension point non-functional.

Rejected alternative (**Option A**): re-implement the on-disk importer as a legacy MDImporter
(`CFPlugIn` / `.mdimporter`, e.g. via Quinn's `QSpotlightPlugIn` scaffolding) — the only Apple-supported
on-disk path. Keeps the Option-A boundary (the `.mdimporter`'s `GetMetadataForFile` still calls
`scrivi_extract_searchable_text`). Not chosen this round; may be revisited if true not-yet-opened
on-disk indexing becomes a requirement.

**Implementation (2026-07-01 — done):**
1. **Removed the `ScriviSpotlightImporter.appex` target** and all 18 of its reference sites across every
   `Scrivi.xcodeproj/project.pbxproj` section — PBXBuildFile, PBXContainerItemProxy, the app's "Embed
   Foundation Extensions" PBXCopyFilesBuildPhase, PBXFileReference, both PBXFileSystemSynchronized*
   sections, the target's Frameworks/Resources/Sources/CMake build phases, both PBXGroup children,
   the PBXNativeTarget, the project `targets` list + `TargetAttributes`, the app target's embed-phase
   and dependency refs, the PBXTargetDependency, both XCBuildConfigurations, and the XCConfigurationList.
   `plutil -lint` OK; zero remaining `861474…`/appex/importer references.
2. **Deleted `ScriviSpotlightImporter/`** (`ImportExtension.swift`, `Info.plist`) and the stale
   `ScriviSpotlightImporter.xcscheme`.
3. **In-app donor (`SpotlightDonor.swift`) survives** as the sole Spotlight path (Layer 1).
4. SP-046 re-scoped and EP-017 AC6–AC8 struck (done in the same pass this Issue was logged).

**Files Affected:**
- `ScriviSpotlightImporter/ImportExtension.swift` + `ScriviSpotlightImporter/Info.plist` — deleted
- `Scrivi.xcodeproj/project.pbxproj` — appex target + all membership removed
- `Scrivi.xcodeproj/xcshareddata/xcschemes/ScriviSpotlightImporter.xcscheme` — deleted
- `Scrivi/App/SpotlightDonor.swift` — unchanged; adopted Layer 1 path

**Verification (2026-07-01 — Verified):**
- `xcodebuild -scheme ScriviApp -destination 'platform=macOS' build` → **BUILD SUCCEEDED**; app
  codesigns cleanly.
- Built `Scrivi.app` has **no `Contents/PlugIns`** (appex gone); `codesign --deep --strict` → valid,
  satisfies its Designated Requirement.
- Target list reduced to ScriviApp / -iOS / -visionOS / ScriviInteropTests.
- **User-approved (2026-07-01):** Option-B descope accepted; proceed. (Live donor-indexed Spotlight
  search remains exercisable via Layer 1 / T-0189 in SP-047 — not a gate on this descope Issue.)

**References:**
- Apple DTS thread — [macOS] CoreSpotlight importer using CSImportExtension: https://developer.apple.com/forums/thread/713953
- CSImportExtension docs: https://developer.apple.com/documentation/corespotlight/csimportextension

---

## I-0058: App does not resume at the last-edited scene on launch — always opens at the first scene

**Status:** ✅ Resolved - Verified (2026-07-09, user-approved: full-fidelity fix confirmed — resumes at last-edited scene + cursor + scroll)
**Platform:** macOS (Apple platforms generally; the Swift open path is shared)
**Component:** `Scrivi/App/ProjectSession.swift` (`load(at:)`), `Scrivi/Views/ViewportSceneLoader.swift` (`loadAll()`), `Scrivi/Views/EditorView.swift` (`selectDefaultSceneIfNeeded()`); backend serialization gap in `ScriviCore/src/public_api/scrivi_c_api.cpp` (`scrivi_open_project`) and `scrivi_save_scene`
**Severity:** High (documented core behavior — "Resume Writing" — is silently non-functional; regressed against SP-049 R4 / T-0007 / T-0062 acceptance criteria)
**Regression:** ✅ Yes — regression against **SP-049 R4** ("On relaunch, all previously-open project windows are restored; per-window scene/cursor/scroll restored by the existing backend open flow"). The backend open flow does return the restored scene/selection/scroll, but the Swift open path was never wired to consume it, so the R4 "scene/cursor/scroll restored" acceptance was not actually met on the Apple UI even though the sprint was closed.
**Sprint:** Not Assigned
**Epic:** EP-018 (per-window/per-project model — R4 restore-within-project) / touches EP-019 area only incidentally
**Related:** `Scrivi_Project_Creation_and_Open_Flow_v0_2.md` (Resume Writing Flow §, restores active scene / `restoredSelection` / `restoredScroll`), `Scrivi_PerWindow_Project_Model_Design_v0_1.md` R4, `Scrivi_Backend_Behavior_Spec_v0_2.md` step 12 ("Scrivi restores last scene and cursor"); Task-verified-0007, Task-verified-0062 (backend-side workspace-state restore, verified in isolation); I-0018 (Navigator shows no selection on load — same "nothing is selected on open" surface)
**Date Identified:** 2026-07-09
**Reported during:** SP-054 testing (T-0207/T-0208/T-0209 verified working; this is a separate, pre-existing regression noticed in passing)

**Description:**
Every launch opens the manuscript at the very first scene (chapter one), regardless of where the
writer was editing at the end of the previous session. The user left off in Chapter 15; the app
reopens at the beginning each time.

**Expected Behavior:**
On relaunch, within each restored project window, Scrivi returns the writer to the scene they last
edited (and, ideally, the cursor/scroll position within it) — the "Resume Writing Flow" specified in
`Scrivi_Project_Creation_and_Open_Flow_v0_2.md` and asserted by SP-049 R4.

**Actual Behavior:**
The editor always shows the first scene. The Navigator shows no active highlight until the user
scrolls.

**Steps to Reproduce:**
1. Open a project, edit a scene deep in the manuscript (e.g. Chapter 15), let it auto-save.
2. Quit the app.
3. Relaunch — the editor opens at the first scene, not Chapter 15.

**Root-Cause Analysis (no code changed):**

The backend *does* the right thing; the Swift layer discards it, and the boundary drops the fine-grained parts.

1. **Backend persists & restores the active scene correctly.**
   - On save, `SceneWriter.cpp` (~L92–110) writes `WorkspaceState.lastWritingSurface` with the
     saved `sceneID` (via `WorkspaceStateService`). So the last-edited **scene** is durably recorded
     each save.
   - On open, `ScriviCore.cpp` (~L99–145) loads that workspace state, computes `activeScene`,
     `restoredSelection`, and `restoredScroll`, and returns them in `OpenProjectResult`.

2. **The C ABI boundary drops selection/scroll (and never receives them on save).**
   - `scrivi_open_project` (scrivi_c_api.cpp ~L354–367) serializes only
     `activeScene.{sceneID,metadataPath,contentPath,markdown}` — it **omits `restoredSelection` and
     `restoredScroll`** that the C++ result carries. So cursor/scroll can never reach Swift even
     though the backend computed them.
   - `scrivi_save_scene` has **no selection/scroll parameters**, and Swift's `saveScene` does not
     send them. Thus `WorkspaceState.lastWritingSurface.selection/scroll` are always default. (The
     **sceneID** is still written correctly, so scene-level restore is fully available.)

3. **The Swift open path ignores the restored scene entirely — the decisive defect.**
   - `ProjectSession.load(at:)` (ProjectSession.swift ~L77–94) decodes `result.activeScene` into
     `OpenProjectResult` (it *is* decoded — ScriviEngine.swift ~L1093/1101) but passes **only
     `result.scenes`** to `ViewportSceneLoader` and calls `loadAll()`. `result.activeScene` is never
     used.
   - `ViewportSceneLoader.loadAll()` (ViewportSceneLoader.swift ~L87–99) hard-codes
     `currentIndex = 0`, sets `cursorSceneID = segments.first`, and **intentionally leaves
     `viewportSceneID = nil`**.
   - `EditorView.selectDefaultSceneIfNeeded()` (iOS, EditorView.swift ~L115–118) and its macOS
     analogue then read `loader.viewportSceneID ?? allScenes.first` → always falls back to the first
     scene. The in-code comments ("we already persist/restore it") are **stale/aspirational** — no
     Swift persistence or restore of `viewportSceneID` exists; there is no UserDefaults/disk key for
     it, and none was ever committed (git history shows no `viewportSceneID`/`scrivi.viewport`
     persistence).

**Net:** the backend hands Swift the correct last-active scene on every open; Swift throws it away and
defaults to scene 0.

**Candidate Fix Directions (for discussion — not yet chosen):**
- **Minimum viable (scene-level resume):** In `ProjectSession.load(at:)`, after `loadAll()`, seed the
  loader's `viewportSceneID` (and `currentIndex`/`cursorSceneID`) from `result.activeScene?.sceneID`
  when present. This alone fixes "returns to Chapter 15" with no ABI change, because the backend
  already returns the correct `activeScene`. Add a `ViewportSceneLoader` entry point (e.g.
  `seedActiveScene(_:)`) rather than overloading `loadAll()`.
- **Full fidelity (cursor + scroll):** Extend the C ABI to (a) serialize `restoredSelection` /
  `restoredScroll` in `scrivi_open_project`, and (b) accept selection/scroll on `scrivi_save_scene`
  so Swift can report them at save time. Then thread scroll/selection through `EditorView` /
  `ManuscriptTextView` to scroll-to and place the cursor on open.
- **Verify the write path first:** Confirm auto-save actually calls `saveScene` for the deep scene
  before quit (the last-active `sceneID` is only recorded on a real save). If auto-save on a scene the
  user merely scrolled to — but didn't edit — doesn't fire, the recorded active scene may lag; may
  need `openProject`/close to also stamp the current `viewportSceneID`.

**Recommended first step:** the minimum-viable scene-level seed (backend already supplies the data),
then decide whether cursor/scroll fidelity is worth the ABI change.

**Implementation — Full-fidelity fix (2026-07-09, user-approved direction: full fidelity):**

Scene + cursor + scroll all restored end-to-end. The backend already computed and persisted the data;
the fix widens the C ABI to carry it and wires the Swift layer to consume/produce it.

1. **C ABI — emit restored surface on open, accept it on save**
   (`ScriviCore/src/public_api/scrivi_c_api.cpp`, `ScriviCore/include/scrivi/scrivi.h`):
   - `scrivi_open_project` now serializes a `restored` sub-object `{anchor, focus, scroll}` from the
     C++ result's `restoredSelection`/`restoredScroll` (previously dropped at the boundary).
   - `scrivi_save_scene` gains `long long selectionAnchor, long long selectionFocus, double scroll`
     params, populating `SaveSceneRequest.selection`/`.scroll` (the C++ struct already had these
     fields; only the C ABI lacked them). Negative offsets are clamped to 0.
2. **Swift engine** (`Scrivi/Engine/ScriviEngine.swift`):
   - New `RestoredSurfaceResult {anchor, focus, scroll}`; `OpenProjectResult.restored` decodes it
     (all `decodeIfPresent`, defaults 0 — backward compatible).
   - `saveScene(...)` gains `selectionAnchor/selectionFocus/scroll` params (defaulted to 0, so
     existing call sites are unaffected) and forwards them to the widened C ABI. Unavailable-platform
     stub signature updated to match.
3. **Loader** (`Scrivi/Views/ViewportSceneLoader.swift`):
   - `loadAll(activeSceneID:restoredSelection:restoredScroll:)` — when the backend supplies an active
     scene that still exists, seeds `currentIndex`/`cursorSceneID`/`viewportSceneID` to it and stashes
     the one-shot `restoredSelectionOffset`/`restoredScrollFraction`; otherwise unchanged (scene 0, no
     pre-selection).
   - Tracks live `currentSceneCursorOffset` (scene-local) and `scrollFraction`; `saveScene(at:)` sends
     them **only for the current scene**. `saveAllDirty` now saves the current scene **last** so its
     writing-surface state is the one the backend records as `lastWritingSurface`.
4. **Session** (`Scrivi/App/ProjectSession.swift`): `load(at:)` passes
   `result.activeScene?.sceneID` + `result.restored?.anchor/scroll` into `loadAll(...)`.
5. **Editor** (`Scrivi/Views/ManuscriptTextView.swift`):
   - `textViewDidChangeSelection` records the scene-local cursor offset into the loader.
   - `scrollDidChange` records the document scroll fraction into the loader.
   - New `restoreWritingSurface(in:)` runs once (guarded by `didRestoreSurface`) on first
     `updateNSView`: places the cursor at the restored scene's storage offset + clamped scene-local
     offset and scrolls it into view, then consumes the one-shot state.
   - iOS resumes automatically: `EditorView.selectDefaultSceneIfNeeded()` already reads
     `loader.viewportSceneID`, which is now correctly seeded.

**Cursor rules honoured** (`Scrivi_Project_Creation_and_Open_Flow_v0_2.md` §9.3): the scene-local
offset is clamped to the restored scene's current text length, so an externally-shrunken scene places
the cursor safely rather than out of bounds.

**Files changed:** `scrivi_c_api.cpp`, `scrivi.h`, `ScriviEngine.swift`, `ViewportSceneLoader.swift`,
`ProjectSession.swift`, `ManuscriptTextView.swift`. No new files → no `project.pbxproj` change.

**Build/test:** ScriviCore builds clean; all **249** C++ tests pass (incl. `OpenProjectTests` /
`MvpLoopTests` that assert `restoredSelection`/`restoredScroll` round-trip). `xcodebuild -scheme
ScriviApp -destination 'platform=macOS' build` → **BUILD SUCCEEDED**.

**Verification:** ✅ Confirmed on-device (2026-07-09, user-approved) — editing deep in the manuscript,
quitting, and relaunching returns to that scene with cursor/scroll restored.

**Follow-up — stamp the scrolled-to scene on flush (2026-07-09, user-approved):**

Addresses verification note (b): a scene the writer *scrolled to but never edited* is now also a valid
resume point. Previously the last-active `sceneID` was only stamped by a real `saveScene`, so a
scroll-only visit didn't move the resume point.

- `ViewportSceneLoader.stampWritingSurface(engine:ref:)` — forces one final `saveScene` of the
  **viewport** (scrolled-to) scene with the current scroll fraction and cursor offset 0, so the
  backend records it as `lastWritingSurface`. No-op when the viewport scene equals the cursor scene
  (already stamped) or isn't loaded. Cursor offset 0 follows §9.3 ("cursor not in this scene ⇒ restore
  scene, place cursor safely").
- `saveAllDirty(...)` calls `stampWritingSurface` after the current-scene save.
- Hook: this runs on the existing `willResignActive` flush (`AppEnvironment.onAppResign` →
  `session.saveAllDirty()`), which fires on quit and on background — the paths that matter for resume.
- **Deliberately not** wired into the synchronous window-close path (`closeProject` → `session.close()`
  is a synchronous NSWindowDelegate teardown; injecting an async save there risks the loader being
  nil'd mid-write). Narrow uncovered case: ⌘W one window while the app stays active and never resigns
  before quit — edits are already persisted by the per-scene autosave; only the scroll-only resume
  point could be slightly stale.

**Follow-up files changed:** `ViewportSceneLoader.swift` only. macOS app builds; 249 C++ tests pass.


> *Archived from `Issue-backlog.md` on 2026-08-16. It was Verified 2026-07-09 but never moved out of
> the backlog — the 2026-08-15 cleanup archived `Issue-active.md` only, and the 2026-08-16 consistency
> audit did not open `Issue-backlog.md` either. Content is unchanged from the backlog entry.*

---
