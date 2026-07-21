# Active Sprint

## SP-075: [Apple] Adopt the merge endpoints (regression-safe swap)

**Status:** 🟡 Active
**Epic:** EP-028: [Cross] Scene & Chapter Merging — Linux Parity & Filesystem-Coherence Fix (2nd of 3 sprints)
**Goal:** Point the macOS app's existing merge commands at SP-074's atomic ScriviCore endpoints so that
`⌘-Backspace` (scene) and `⇧⌘-Backspace` (chapter) behave exactly as today from the user's view — but the
**chapter-merge now survives quit/reopen with no scene loss** (the I-0083 regression fix). No new UI, no new
keybindings, no behavior change beyond correctness. This is a regression-safe swap of the merge's backend:
the Swift-composed `saveScene`-into-predecessor + `deleteChapter` path (which deletes on-disk scene files
under EP-027) is replaced by `scrivi_merge_scene` / `scrivi_merge_chapter`.
**Start Date:** 2026-07-21
**End Date:** —
**Capacity:** ~4–6 hours

### Assigned Tasks

| ID     | Title | Priority | Status |
| ------ | ----- | -------- | ------ |
| T-0302 | `[Apple]` `ScriviEngine.mergeScene` / `mergeChapter` wrappers — call `scrivi_merge_scene` / `scrivi_merge_chapter`, decode the JSON envelopes into `MergeSceneResult` / `MergeChapterResult` Swift structs, throw `ScriviError` on failure; interop tests in `ScriviInteropTests` | High | 🟢 Implemented, Not Verified |
| T-0303 | `[Apple]` Point `ManuscriptTextView.handleMergeScene` / `handleMergeChapter` at the engine wrappers (replace the `ViewportSceneLoader` in-memory reassign + `scrivi_delete_chapter` composition); keep the `⌘-Backspace` / `⇧⌘-Backspace` bindings, the manuscript-start no-op, and the `sceneMerge` / `chapterMerge` history barriers; refresh segment paths from the result (I-0081 contract); re-anchor caret | High | 🟢 Implemented, Not Verified |

### Assigned Issues

| ID     | Title | Severity | Status |
| ------ | ----- | -------- | ------ |
| I-0083 | Chapter-merge loses scenes on reopen (in-memory reassign + `deleteChapter` deletes on-disk scene files under EP-027) — **core fixed in SP-074; this sprint delivers the macOS app adoption** | High | 🟢 Resolved (core), Not Verified |
| I-0084 | Caret jumps to the next scene/chapter start after a scene merge — redundant `updateNSView` rebuild dropped the selection (found during SP-075 GUI test) | Medium | 🟢 Resolved, Not Verified |
| I-0085 | App crashes constructing the Open Project panel (`NSOpenPanel _initBridgeAndStuff` semaphore timeout) — **pre-existing, NOT merge-related**; found during SP-075 GUI test | High | 🔴 Open |

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

### Verification (developer-level; user verification pending)

- `xcodebuild -scheme ScriviApp -destination 'platform=macOS' build` → **BUILD SUCCEEDED**.
- `... test` → **TEST SUCCEEDED**, 36/36 interop tests incl. all 4 new merge tests. The AC4 regression
  (chapter merge → reopen → both scenes present, ch2 body intact) is proven end-to-end through the Swift
  boundary. Live VNC/GUI walkthrough (caret placement, `⌘-Backspace` in-editor, quit→reopen) is user-pending.

### Retrospective

_(filled in at close)_

---

*Last Updated: 2026-07-21 (SP-075 `[Apple]` T-0302 + T-0303 🟢 Implemented, Not Verified — engine merge
wrappers + handler swap onto SP-074's endpoints; app-side of the I-0083 fix. `xcodebuild build` + `test`
green (36/36 interop). First GUI test surfaced two defects: **I-0084** (caret jumped to next scene after a
scene merge — redundant `updateNSView` rebuild dropped the selection; **fixed** in `rebuildStorage` by
stamping the change-detection keys; build green, GUI re-check pending) and **I-0085** (Open Project panel
`NSOpenPanel` crash — **pre-existing, NOT merge-related**, logged Open). Scene-merge join separator changed
to blank-line (user-approved). `scrivi.h`/ScriviCore untouched; no pbxproj change. Awaiting user (GUI)
re-verification → sprint close. Next available sprint **SP-076**; next task **T-0308**; next issue **I-0086**.)*
