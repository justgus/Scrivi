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

*Closed: 2026-06-03*
