# SP-075 (Closed)

## SP-075: [Apple] Adopt the merge endpoints (regression-safe swap)

**Status:** ✅ Closed (Human-approved 2026-07-21)
**Epic:** EP-028: [Cross] Scene & Chapter Merging — Linux Parity & Filesystem-Coherence Fix (2nd of 3 sprints)
**Goal:** Point the macOS app's existing merge commands at SP-074's atomic ScriviCore endpoints so that
`⌘-Backspace` (scene) and `⇧⌘-Backspace` (chapter) behave exactly as today from the user's view — but the
**chapter-merge now survives quit/reopen with no scene loss** (the I-0083 regression fix). No new UI, no new
keybindings, no behavior change beyond correctness. This is a regression-safe swap of the merge's backend:
the Swift-composed `saveScene`-into-predecessor + `deleteChapter` path (which deletes on-disk scene files
under EP-027) is replaced by `scrivi_merge_scene` / `scrivi_merge_chapter`.
**Start Date:** 2026-07-21
**End Date:** 2026-07-21
**Capacity:** ~4–6 hours

### Assigned Tasks

| ID     | Title | Priority | Status |
| ------ | ----- | -------- | ------ |
| T-0302 | `[Apple]` `ScriviEngine.mergeScene` / `mergeChapter` wrappers — call `scrivi_merge_scene` / `scrivi_merge_chapter`, decode the JSON envelopes into `MergeSceneResult` / `MergeChapterResult` Swift structs, throw `ScriviError` on failure; interop tests in `ScriviInteropTests` | High | ✅ Verified (2026-07-21) |
| T-0303 | `[Apple]` Point `ManuscriptTextView.handleMergeScene` / `handleMergeChapter` at the engine wrappers (replace the `ViewportSceneLoader` in-memory reassign + `scrivi_delete_chapter` composition); keep the `⌘-Backspace` / `⇧⌘-Backspace` bindings, the manuscript-start no-op, and the `sceneMerge` / `chapterMerge` history barriers; refresh segment paths from the result (I-0081 contract); re-anchor caret | High | ✅ Verified (2026-07-21) |

### Assigned Issues

| ID     | Title | Severity | Status |
| ------ | ----- | -------- | ------ |
| I-0083 | Chapter-merge loses scenes on reopen (in-memory reassign + `deleteChapter` deletes on-disk scene files under EP-027) — **core fixed in SP-074; this sprint delivered the macOS app adoption** | High | ✅ Resolved - Verified (2026-07-21) |
| I-0084 | Caret jumps to the next scene/chapter start after a scene merge — redundant `updateNSView` rebuild dropped the selection (found during SP-075 GUI test) | Medium | ✅ Resolved - Verified (2026-07-21) |
| I-0085 | App crashes constructing the Open Project panel (`NSOpenPanel _initBridgeAndStuff` semaphore timeout) — **environmental, NOT a code bug**; clean relaunch worked (duplicate instance + macOS 27.0 beta panel-XPC timeout) | High | ✅ Closed — Not a Bug (2026-07-21) |
| I-0086 | Build warnings: "Result of 'try?' is unused" at three call sites (`HistoryCapture.close`/`seedBaselineIfNeeded`, `ProjectSettingsSheet.saveHistorySettings`) — fixed with `_ = try?` | Low | ✅ Resolved - Not Verified (2026-07-21) |

### Sprint Notes

- **Backend already done (SP-074).** Both endpoints are exported in `libScriviCore.a`
  (`_scrivi_merge_scene`, `_scrivi_merge_chapter`) and cross-platform green (macOS 317/317, Linux 324/324).
  This sprint is Swift-only; `scrivi.h` and ScriviCore are untouched.
- **Result envelopes to decode:**
  - `scrivi_merge_scene` → `survivorSceneID`, `mergedSceneID`, `chapterID`, `survivorMetadataPath`,
    `survivorContentPath`, `chapterMetadataPath`, `merged`.
  - `scrivi_merge_chapter` → `survivorChapterID`, `mergedChapterID`, `survivorChapterMetadataPath`,
    `scenesRelocated`, `merged`.
- **Stale-path contract (I-0081):** both merges relocate/rename files, so the app MUST refresh the affected
  segments' `metadataPath`/`contentPath`/`chapterMetadataPath` from the result — do not keep pre-merge paths.
- **Touch points:** `Scrivi/Engine/ScriviEngine.swift` (new wrappers; the old `scrivi_delete_chapter` call at
  `ScriviEngine.swift:521` is the composition being retired for chapter-merge),
  `Scrivi/Views/ManuscriptTextView.swift` (`handleMergeScene` ~808, `handleMergeChapter` ~872, key bindings
  ~1213/1217), `Scrivi/Views/ViewportSceneLoader.swift` (`mergeSceneIntoPredecessor` ~495,
  `mergeChapterIntoPredecessor` ~590 — the in-memory reassign that caused I-0083).
- **pbxproj:** any NEW `.swift` file MUST be added to `Scrivi.xcodeproj/project.pbxproj` in the same step
  (CLAUDE.md — the user commits from the command line). Editing existing files needs no pbxproj change.
- **Verify:** `xcodebuild -scheme ScriviApp -destination 'platform=macOS' build test`; then the AC4 flow —
  chapter-merge, quit, reopen, confirm every scene present and in order (the regression that SP-074 fixes at
  the core, now proven end-to-end in the app). Records the merge history barriers (AC6).
- **Out of scope:** Linux parity (SP-076), any user-visible behavior change, structural undo.

### Implementation summary (2026-07-21)

- **T-0302 — engine wrappers.** `ScriviEngine.mergeScene(projectRootPath:sceneID:)` /
  `mergeChapter(projectRootPath:chapterID:)` call the C ABI and `decodeC` the envelopes into new
  `MergeSceneResult` / `MergeChapterResult` structs (fields per SP-074); failure envelopes throw `ScriviError`
  via the shared `decodeC` path. 4 interop tests in `ScriviInteropTests` (scene merge body-survives-reopen;
  chapter merge all-scenes-survive-reopen = the I-0083 fix through Swift; first-scene / first-chapter throw).
- **T-0303 — handler swap.** `handleMergeScene` now calls `engine.mergeScene` (retiring the
  `saveScene(joinText)`+`deleteScene` composition); `handleMergeChapter` now calls `engine.mergeChapter`
  (retiring the in-memory-reassign + `deleteChapter` that caused I-0083). Kept `⌘-Backspace` / `⇧⌘-Backspace`
  bindings, the at-start-of-scene / first-scene-of-chapter no-op guards, and the `sceneMerge` / `chapterMerge`
  history barriers. Chapter-merge relocates files (new order-key names) so `handleMergeChapter` now refreshes
  all scene paths from a fresh `openProject` via the new `ViewportSceneLoader.refreshScenePaths(from:)` (I-0081
  stale-path contract). Both scenes flushed to disk before scene-merge so the on-disk join is authoritative.
- **Intentional behavior change (user-approved 2026-07-21):** scene-merge now joins the two bodies with a
  **blank line** (the endpoint's `SceneMerger.joinBodies` default), where the old macOS path ran them
  together with no separator. `Coordinator.joinedMergeBody` mirrors the endpoint's elision so in-memory text
  equals disk.
- **No new `.swift` files → no pbxproj change** (edits to existing files only). **No ScriviCore change** —
  `scrivi.h` untouched; the app links the SP-074 `libScriviCore.a` (symbols already exported).

### Verification

- `xcodebuild -scheme ScriviApp -destination 'platform=macOS' build` → **BUILD SUCCEEDED**.
- `... test` → **TEST SUCCEEDED**, 36/36 interop tests incl. all 4 new merge tests. The AC4 regression
  (chapter merge → reopen → both scenes present, ch2 body intact) is proven end-to-end through the Swift
  boundary.
- ✅ **User GUI verification (2026-07-21):** the live in-editor `⌘-Backspace` / `⇧⌘-Backspace` merge, caret
  placement, and quit→reopen flow confirmed — chapter-merge keeps every scene in order. **I-0083 Verified.**

### Two defects found & fixed during GUI testing

- **I-0084** (Medium) — caret jumped to the next scene/chapter start after a scene merge: a redundant
  `updateNSView` rebuild dropped the selection. Fixed in `ManuscriptTextView.rebuildStorage` by stamping the
  change-detection keys (`lastSegmentIDs`/`lastShowChapterTitles`) so the follow-up pass sees no change and
  skips the redundant rebuild. ✅ **Verified (2026-07-21).**
- **I-0086** (Low) — three "Result of 'try?' is unused" build warnings (`HistoryCapture.close` /
  `seedBaselineIfNeeded`, `ProjectSettingsSheet.saveHistorySettings`); fixed with `_ = try?`. Build green,
  warnings gone. Resolved - Not Verified.

**Also closed this sprint:** **I-0085** (Open Project panel `NSOpenPanel` crash) — investigated and closed
as **Not a Bug**: environmental duplicate-instance + macOS 27.0 beta panel-XPC handshake timeout; a clean
relaunch reproduced a working Open dialog. No code change.

### Retrospective

- **What went well:** The SP-074 core split paid off — the app-side swap was a small, contained edit that
  compiled and passed interop on the first build, with the I-0083 regression proven through the Swift
  boundary before any GUI testing.
- **What surfaced:** Live GUI testing (not caught by interop) exposed the I-0084 caret regression and the
  pre-existing I-0086 warnings; the I-0085 crash turned out to be a beta-OS + duplicate-instance environmental
  artifact, not code. The lesson holds: native-text-view caret behavior can't be verified from CI and needs a
  GUI pass.
- **Outcome:** EP-028 AC1–AC4 all Verified. Only the `[Linux]` leg (AC5–AC7) remains → SP-076.

---

*Closed 2026-07-21 (Human-approved). SP-075 delivered the macOS adoption of SP-074's merge endpoints;
I-0083/I-0084 Verified, I-0086 fixed, I-0085 closed Not-a-Bug. EP-028 AC1–AC4 complete; SP-076 `[Linux]`
parity is the final leg.*
