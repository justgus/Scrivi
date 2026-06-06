# Verified Issues — I-0001 to I-0010

---

## I-0001: git commit fails on CI runners with no global git identity configured

**Status:** ✅ Resolved - Verified
**Platform:** Linux (CI / Ubuntu runner)
**Component:** ScriviCore/src/git/SystemGitProvider.cpp
**Severity:** High
**Sprint:** SP-002

**Description:**
The real-git integration tests (Tests 81–82) passed locally on macOS but failed on the GitHub Actions Ubuntu runner. The `git commit` call returned a non-zero exit code, causing `enableGitSnapshots()` and `createSnapshot()` to propagate a `gitError` failure.

**Expected Behavior:**
Tests 81 and 82 pass on all CI platforms when `git` is available in PATH.

**Actual Behavior:**
Tests 81 and 82 failed on Ubuntu CI with `REQUIRE( result.ok() )` expanding to `false`. The `git commit` command exited non-zero because no committer identity was available.

**Steps to Reproduce:**
1. Run the test suite on a Linux machine (or CI runner) with no global `user.name` / `user.email` in `~/.gitconfig`.
2. Tests 81 and 82 fail even though `git` is in PATH and `SystemGitProvider::available()` returns `true`.

**Impact:**
- CI Ubuntu check reported 2 failed tests out of 82, blocking merge.
- Real-git snapshot functionality silently broken on any machine without a global git identity configured.

**Date Identified:** 2026-05-20

**Root Cause Analysis:**
`git commit --author "Name <email>"` sets the *author* identity but Git still requires a *committer* identity resolvable from configuration (`user.name` / `user.email` in `.gitconfig` or the repo's local config). The GitHub Actions Ubuntu runner has no global git identity, so git exits with an error asking the user to configure their identity.

**Resolution:**

**Fix Date:** 2026-05-20
**Verification Date:** 2026-05-20

**Implementation:**

1. **Pass committer identity via `-c` flags** (`SystemGitProvider.cpp` — `commit()` method)
   - Added `-c user.name=<name>` and `-c user.email=<email>` before the `commit` subcommand.
   - The `-c` flag applies a config override for the single invocation only — no system or global config is touched.
   - Both author and committer identity are now supplied entirely from the `CommitRequest`.

**Code Changes:**
```cpp
// Before
auto r = util::runProcess("git",
    {"commit", "--allow-empty", "--author", authorStr, "-m", req.message},
    rootPath);

// After
auto configName  = "user.name=" + req.author.name;
auto configEmail = "user.email=" + req.author.email;
auto r = util::runProcess("git",
    {"-c", configName, "-c", configEmail,
     "commit", "--allow-empty", "--author", authorStr, "-m", req.message},
    rootPath);
```

**Result:**
- 82/82 tests pass locally on macOS.
- GitHub Actions Ubuntu CI run passed all 82 tests.

**Files Affected:**
- `ScriviCore/src/git/SystemGitProvider.cpp` — `commit()` method

**Verification:**
- ✅ GitHub Actions Ubuntu CI workflow passed (Tests 81 and 82)

**Related Issues:**
- None

---

## I-0002: Stale Coordinator Parent Causes Silent Save Failure and Dead Live Titles

**Status:** ✅ Resolved - Verified
**Platform:** macOS
**Component:** `ManuscriptTextView.swift`, `ViewportSceneLoader.swift`
**Severity:** Critical
**Sprint:** SP-024

**Description:**
All text typed or pasted into a scene was lost on `⌘↩` (scene advance) and on app quit. Scene titles in the Navigator always displayed "Scene 1", "Scene 2", etc. and never updated, even after typing. On reload, Navigator titles reverted to "Scene X" even when text was present on disk.

**Expected Behavior:**
- Text typed in a scene persists to disk within 1 second of the last keystroke, and immediately on `⌘↩` or app quit.
- The Scene Navigator title updates to the first line of typed text within ~300ms.
- On reload, Navigator titles reflect persisted content immediately without requiring the author to type.

**Actual Behavior:**
- No text was ever written to disk. On `⌘↩`, the previous scene's text vanished.
- Navigator titles remained "Scene X" permanently during a session.
- Navigator titles reverted to "Scene X" on every reload.

**Steps to Reproduce:**
1. Open a project in the macOS app.
2. Type text into Scene 1.
3. Press `⌘↩` to create Scene 2.
4. Observe: Scene 1's text disappears.
5. Observe: Navigator shows "Scene 1", "Scene 2" regardless of typed text.
6. Quit and reopen: Navigator still shows "Scene X" even after fixing saves.

**Impact:**
- Complete data loss for all typed content — the primary use case of the app was broken.
- Scene Navigator titles non-functional both live and on reload.

**Date Identified:** 2026-06-03

**Root Cause Analysis:**
Three distinct bugs:

**Bug A — Stale `Coordinator.parent`:** `NSViewRepresentable` creates the `Coordinator` once via `makeCoordinator()` and reuses it. SwiftUI calls `updateNSView` on every re-render with a fresh `ManuscriptTextView` struct, but `Coordinator.parent` was never updated. All delegate callbacks accessed `parent.env.authorshipRef` through the stale first-render snapshot — always nil — so every save silently no-oped.

**Bug B — Frozen `sceneBoundaries`:** `sceneBoundaries` (per-segment `NSRange`s into `NSTextStorage`) was only set in `rebuildStorage`, which only fires when the segment list changes. When the author typed, the text storage grew but `sceneBoundaries` stayed frozen at the original lengths. `textDidChange` extracted the stale range — discarding every keystroke before it could reach `updateText`.

**Bug C — `liveTitles` not seeded on load:** `liveTitles` is in-memory and starts empty on every launch. Titles only populated after the author typed in a scene. On reload, the Navigator fell back to "Scene X" for all scenes until typing resumed.

**Fix Date:** 2026-06-03
**Verification Date:** 2026-06-03

**Implementation:**

1. **`coordinator.parent = self` in `updateNSView`** (`ManuscriptTextView.swift`)
   - Added as the first statement in `updateNSView`. Ensures every delegate callback sees the current `env`, `loader`, and bindings on every SwiftUI render cycle.

2. **`recomputeBoundaries()` at top of `textDidChange`** (`ManuscriptTextView.swift`)
   - New helper scans the live `NSTextStorage` for `NSTextAttachment` divider characters (U+FFFC) to recompute accurate per-segment `NSRange` values before any extraction. Called as the first action in `textDidChange`.

3. **Seed `liveTitles` in `loadScene`** (`ViewportSceneLoader.swift`)
   - After loading scene content, extract the first non-empty line and store it in `liveTitles[sceneID]`. Runs for every scene entering the viewport, so titles are populated immediately on launch.

**Files Affected:**
- `Scrivi/Views/ManuscriptTextView.swift` — `updateNSView`, `textDidChange`, new `recomputeBoundaries`
- `Scrivi/Views/ViewportSceneLoader.swift` — `loadScene` title seeding

**Verification:**
- ✅ Text typed in a scene persists across quit and reopen
- ✅ Scene 1 text survives `⌘↩` scene advance
- ✅ Navigator title updates within ~300ms of typing the first line
- ✅ Navigator titles show first-line content immediately on launch (no typing required)
- ✅ Text persists correctly across multiple scenes and chapters

**Related Issues:**
- Discovered during SP-024 verification (T-0083–T-0086)

---

## I-0003: Rename does not update Navigator title in real time

**Status:** ✅ Resolved - Verified
**Platform:** macOS
**Component:** `SceneNavigatorView.swift`, `ViewportSceneLoader.swift`
**Severity:** High
**Sprint:** SP-030

**Description:**
After successfully renaming a scene or chapter, the Navigator continued showing the old title until the project was closed and reopened.

**Root Cause Analysis:**
`performRename` called the engine and discarded the result. Nothing mutated `loader.allScenes`, so `@Observable` did not trigger a Navigator refresh.

**Fix Date:** 2026-06-04
**Verification Date:** 2026-06-04

**Implementation:**
1. Added `updateSceneTitle(forMetadataPath:)` and `updateChapterTitle(forChapterMetadataPath:)` to `ViewportSceneLoader` — mutate the matching `SceneInfo` in `allScenes`, triggering an `@Observable` diff.
2. Called both from `performRename` after a successful engine call.

**Files Affected:**
- `Scrivi/Views/ViewportSceneLoader.swift`
- `Scrivi/Views/SceneNavigatorView.swift`

**Verification:**
- ✅ Rename a scene → Navigator title updates immediately without restart
- ✅ Rename a chapter → chapter header updates immediately without restart

---

## I-0004: Renamed scene and chapter titles not displayed in Navigator after restart

**Status:** ✅ Resolved - Verified
**Platform:** macOS
**Component:** `ManuscriptOrderResolver.cpp`, `SceneNavigatorView.swift`
**Severity:** High
**Sprint:** SP-030

**Description:**
After renaming a scene or chapter and reopening the project, the Navigator showed "Scene N" / "Chapter N" instead of the stored custom title.

**Root Cause Analysis:**
`ResolvedScene` had no `chapterTitle` field — the chapter title was read during the resolver walk but discarded. `SceneNavigatorView.chapterGroups` used a two-step fallback (live text → "Scene N") that never consulted `info.title`.

**Fix Date:** 2026-06-04
**Verification Date:** 2026-06-04

**Implementation:**
1. Added `chapterTitle` and `chapterMetadataPath` to `ResolvedScene`, `SceneSummary`, C API JSON, and `SceneInfo` in Swift.
2. Fixed Navigator fallback to consult stored titles (later corrected by I-0006).

**Files Affected:**
- `ScriviCore/src/manuscript/ManuscriptOrderResolver.hpp/.cpp`
- `ScriviCore/include/scrivi/Types.hpp`
- `ScriviCore/src/project_package/ProjectOpener.cpp`
- `ScriviCore/src/public_api/scrivi_c_api.cpp`
- `Scrivi/Engine/ScriviEngine.swift`
- `Scrivi/Views/SceneNavigatorView.swift`

**Verification:**
- ✅ Rename a scene → close and reopen → Navigator shows custom title
- ✅ Rename a chapter → close and reopen → Navigator shows custom chapter title

---

## I-0005: Delete confirmation dialog excessively wide when scene has a long live title

**Status:** ✅ Resolved - Verified
**Platform:** macOS
**Component:** `SceneNavigatorView.swift`
**Severity:** Medium
**Sprint:** SP-030

**Description:**
When deleting a scene whose Navigator title was the first sentence of its prose, the confirmation dialog button label contained the full sentence, making the dialog absurdly wide.

**Root Cause Analysis:**
`entry.title` (which could be a full prose sentence) was embedded verbatim into the `confirmationDialog` button label with no length cap.

**Fix Date:** 2026-06-04
**Verification Date:** 2026-06-04

**Implementation:**
Added `truncated(_:limit:)` helper capping at 30 characters with `…`; applied to all confirmation dialog title uses.

**Files Affected:**
- `Scrivi/Views/SceneNavigatorView.swift`

**Verification:**
- ✅ Short title: shown in full, no ellipsis
- ✅ Long live title (> 30 chars): truncated with `…`, dialog width normal

---

## I-0007: Rename fails with "metadataPath must not be empty" on a newly created scene

**Status:** ✅ Resolved - Verified
**Platform:** macOS
**Component:** `ScriviEngine.swift`, `ViewportSceneLoader.swift`
**Severity:** High
**Sprint:** SP-030

**Description:**
Right-clicking a scene created during the current session and selecting Rename delivered the error "metadataPath must not be empty". After restarting the app, rename worked correctly.

**Root Cause Analysis:**
`SceneInfo`'s custom `Decodable` init used `decodeIfPresent` for `metadataPath`, silently defaulting to `""` on a missing key. `insertScene` and `insertChapterFirstScene` also omitted `chapterMetadataPath` from newly created `SceneInfo` entries.

**Fix Date:** 2026-06-04
**Verification Date:** 2026-06-04

**Implementation:**
1. Changed `metadataPath` and `contentPath` to hard `decode` in `SceneInfo.init(from:)`.
2. `insertScene` now inherits `chapterMetadataPath` and `chapterTitle` from the predecessor scene.
3. `insertChapterFirstScene` now passes `chapterMetadataPath` from `CreateChapterResult`.

**Files Affected:**
- `Scrivi/Engine/ScriviEngine.swift`
- `Scrivi/Views/ViewportSceneLoader.swift`

**Verification:**
- ✅ Create a new scene → immediately rename → succeeds without error
- ✅ Renamed title appears in Navigator without restart

---

## I-0006: Scene custom title overridden by prose first-line text in Navigator

**Status:** ✅ Resolved - Verified
**Platform:** macOS
**Component:** `SceneNavigatorView.swift`, `SceneCreator.cpp`, `ChapterCreator.cpp`
**Severity:** High
**Sprint:** SP-030

**Description:**
Newly created scenes were assigned `title = "Scene N"` in their sidecar JSON. The Navigator's fallback chain (custom title → live text → "Scene N") interpreted this as a custom title, permanently blocking live prose text from appearing. Clearing a rename also had no effect.

**Root Cause Analysis:**
`SceneCreator.cpp` and `ChapterCreator.cpp` wrote `"Scene N"` / `"Scene 1"` into the `title` field at creation time. There was no semantic distinction between a user-set title and the creation default, so the Navigator always treated it as explicit and overrode live text.

**Fix Date:** 2026-06-04
**Verification Date:** 2026-06-04

**Implementation:**
1. `SceneCreator.cpp`: `sceneMeta.title = ""` (was `"Scene N"`)
2. `ChapterCreator.cpp`: `sceneMeta.title = ""` for the first scene (was `"Scene 1"`)

Chapter `ch.title` left as `"Chapter N"` — chapters have no live-text fallback, so a stored default is harmless.

**Files Affected:**
- `ScriviCore/src/manuscript/SceneCreator.cpp`
- `ScriviCore/src/manuscript/ChapterCreator.cpp`

**Verification:**
- ✅ New scene with no content shows "Scene N" (ordinal fallback)
- ✅ Typing a first line updates Navigator to prose text
- ✅ Rename overrides prose text with custom title
- ✅ Clearing rename (save blank) falls back to prose text
- ✅ 195/195 ctests green

---

## I-0008: Scene Navigation: Scroll to End of Scene, Place Cursor, Transfer Focus

**Status:** ✅ Resolved - Verified
**Platform:** macOS
**Component:** `Scrivi/Views/ManuscriptTextView.swift`
**Severity:** High
**Sprint:** SP-031

**Description:**
When clicking a scene in the Navigator, the viewport did not land at the end of the selected scene with the cursor positioned for writing, and did not transfer focus to the text view.

**Expected Behavior:**
Clicking a scene scrolls the last line of that scene to the bottom of the visible viewport, places the cursor at the end of the scene's text, and transfers first responder to the text view so the writer can type immediately.

**Actual Behavior:**
Navigating upward landed approximately at the middle of the scene. Cursor was placed at the scene start. Focus was not transferred to the text view.

**Fix Date:** 2026-06-05
**Verification Date:** 2026-06-05

**Files Affected:**
- `Scrivi/Views/ManuscriptTextView.swift`

**Verification:**
- ✅ Clicking any scene scrolls the end of that scene to the bottom of the viewport
- ✅ Cursor placed at end of selected scene's text
- ✅ `ManuscriptNSTextView` gains first responder on macOS immediately

---

## I-0009: Manuscript Text Unreadable in Dark Mode (Black Text on Dark Background)

**Status:** ✅ Resolved - Verified
**Platform:** macOS
**Component:** `Scrivi/Views/ManuscriptTextView.swift`
**Severity:** High
**Sprint:** SP-031

**Description:**
Manuscript text rendered in black regardless of system appearance, making it unreadable in dark mode.

**Expected Behavior:**
Manuscript text color adapts to the system color scheme using a semantic color (e.g., `.label` / `.primary`).

**Actual Behavior:**
Text color was hardcoded, remaining black against a dark background in dark mode.

**Fix Date:** 2026-06-05
**Verification Date:** 2026-06-05

**Files Affected:**
- `Scrivi/Views/ManuscriptTextView.swift`

**Verification:**
- ✅ Manuscript text readable in both light and dark mode
- ✅ Text color matches system appearance automatically

---

*Closed: 2026-06-05*
