# T-0195: Session manifest persistence + restore all previously-open windows

**Status:** ✅ Implemented - Verified
**Component:** `Scrivi/App/OpenSessionManifest.swift` (new); `AppEnvironment.swift`, `ScriviApp.swift`
**Epic:** EP-018
**Sprint:** SP-049
**Date Requested:** 2026-06-24
**Date Implemented:** 2026-06-24
**Date Verified:** 2026-06-24
**Design Reference:** `Scrivi_PerWindow_Project_Model_Design_v0_1.md` §5 (persistence & restore, R4 = restore all — Q1)

**Goal:**
On relaunch, restore **all** project windows that were open at last quit (R4). Within each window,
scene/cursor/scroll restoration is already handled by the backend open flow.

**Resolution:**
- **`OpenSessionManifest`** — persists the open projectID set to UserDefaults (`stringArray`).
  Paths are not stored; each projectID resolves via `ProjectBookmarkStore` (the same mechanism as
  deep-link reopen — reused, not duplicated).
- Manifest rewritten on every open (`loadProject`) and close (`closeProject`).
- **`restoreOpenProjects()`** — on launch, reads the manifest and `ensureOpenAndShow`s each project,
  skipping any whose bookmark no longer resolves. Runs **once** per launch (`didLaunchSetup` guard),
  since the Welcome window's `.task` re-fires when Welcome reopens.
- **Quit-time freeze (key fix):** the naive version wrote an empty manifest, because the quit-time
  window-close cascade persisted the shrinking set and the last close wrote `[]`. Fixed with
  `applicationWillTerminate` → `env.beginTermination()`, which **freezes the manifest to the
  still-open set before teardown** and sets `isTerminating` to suppress further writes during the
  close cascade. Verified via the container plist (empty `( )` before the fix → correct set after).
- New file: `OpenSessionManifest.swift`.

**Verification (user, 2026-06-24, against the running app):**
- Open A and B, ⌘Q, relaunch → both windows reopen; Welcome does not linger. ✅
- Open one project, ⌘Q, relaunch → that project reopens. ✅
- Close all (back to Welcome), ⌘Q, relaunch → just Welcome (nothing to restore). ✅
- No regression to normal open/close/reopen/deep-link. ✅

**Acceptance Criteria:**
- [x] All previously-open project windows restored on relaunch (R4).
- [x] Per-window scene/cursor/scroll restored by the existing backend open flow.
- [x] Manifest survives the quit-time close cascade (not wiped to empty).
- [x] macOS build clean; no regression.

*Verified by the user 2026-06-24 via manual relaunch testing.*
