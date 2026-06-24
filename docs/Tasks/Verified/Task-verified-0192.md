# T-0192: Extract `ProjectSession`; move per-project state off `AppEnvironment`

**Status:** ✅ Implemented - Verified
**Component:** `Scrivi/App/ProjectSession.swift` (new); `Scrivi/App/AppEnvironment.swift`; views (`EditorView`, `ManuscriptTextView`, `SceneNavigatorView`, `ScriviApp`)
**Epic:** EP-018
**Sprint:** SP-048
**Date Requested:** 2026-06-24
**Date Implemented:** 2026-06-24
**Date Verified:** 2026-06-24
**Design Reference:** `Scrivi_PerWindow_Project_Model_Design_v0_1.md` §3.1 (two-tier state), §8 step 1

**Goal:**
Move all per-project state and lifecycle off the single shared `AppEnvironment` onto a new
per-project `ProjectSession`, behavior-preserving (app stays single-window this sprint). This is the
foundation that lets T-0194 make sessions per-window.

**Resolution:**
- New `@Observable @MainActor ProjectSession` owns `openProjectResult`, `projectRootPath`,
  `viewportLoader`, `projectPreferences`, `timelineModel`, `pendingNavigationSceneID`,
  `deepLinkAccessURL`, the per-window UI toggles (`inspectorVisible`, `timelineVisible`,
  `showProjectSettings`), and the Spotlight domain. Methods `load(at:)`, `close()`,
  `saveAllDirty()`, `donateSpotlight()`. App-global dependencies (engine, authorshipRef,
  appSupportRoot, identityID) **injected at construction** — the session never reaches back into
  `AppEnvironment`.
- `AppEnvironment` slimmed to app-global orchestration: engine, identity, `projectError`
  (open/create), and a single `activeSession`. It builds and installs sessions; close/onAppResign/
  deep-link/Spotlight delegate to the session.
- Views migrated to `.environment(ProjectSession.self)` (the target multi-window shape, done once):
  `ContentRootView` injects the session and hosts the Project Settings sheet; `EditorView` and the
  manuscript views read session state; `SceneNavigatorView` / `ManuscriptTextView` take a `session`
  param (`engine`/`authorshipRef`/`appSupportRoot` stay on `env`).
- `ProjectSession.swift` added to `project.pbxproj` (fileRef C050 + 3 target build files + group +
  3 sources phases).

**Verification:**
- macOS build (`xcodebuild -scheme ScriviApp`) — BUILD SUCCEEDED.
- User manual regression pass 2026-06-24: open/edit/save/close, timeline, inspector, scene
  navigation behave identically to before the refactor; deep-link path verified (see T-0193).

**Acceptance Criteria:**
- [x] `ProjectSession` owns all former per-project state; `AppEnvironment` keeps only engine +
      identity + orchestration.
- [x] App behavior unchanged (still single window this sprint).
- [x] macOS build clean; no regression to project open/save/close.

*Verified by the user 2026-06-24 via manual regression.*
