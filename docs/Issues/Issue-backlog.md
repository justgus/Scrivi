# Issue Backlog

Issues listed here are open and documented but not currently assigned to a Sprint.

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

---

## I-0017: Window maximized state not restored on app relaunch

**Status:** 🔴 Open
**Platform:** macOS
**Component:** `WindowFrameAutosave.swift`
**Severity:** Medium
**Sprint:** Not Assigned
**Related:** I-0051 (multi-window per-project frame/position restore — Verified 2026-06-29, subsumes the position/size part of this); **I-0055** (multi-window maximize-restore defect — the same zoom-restore problem on the per-window model; fix the two together)

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

*Last Updated: 2026-07-06 (I-0019 closed without verification — OBE/superseded by EP-019, user-approved; moved to `Closed/Issue-closed-0019.md`. The ⌘Z requirement is carried by EP-019 AC1/SP-053. I-0017/I-0018 unchanged.)*
