# Active Issues

These are the open Issues (still awaiting verification). Verified Issues are removed from this
table and stay in this file as full entries only until the next batch archive (I-0051–I-0060).

| ID | Title | Severity | Sprint | Status |
| -- | ----- | -------- | ------ | ------ |
| I-0086 | `[Apple]` **Build warnings: "Result of 'try?' is unused" at three call sites.** Fire-and-forget engine calls used `try?` on a non-Void-returning throwing function without discarding the result, tripping Swift's unused-result warning. Sites: `HistoryCapture.close()` (`historyClose`), `HistoryCapture.seedBaselineIfNeeded()` (`historySeedScene`), and `ProjectSettingsSheet.saveHistorySettings()` (`historySetSettings`). All three are intentional discards (a failure needs no handling here). **Fix:** prefix each with `_ = ` (matching the existing `_ = try?` at `HistoryCapture.swift:94`). One-token change per site; no behavior change. | Low | **SP-075** | ✅ Resolved - Not Verified (2026-07-21) — `_ = try?` applied at `HistoryCapture.swift:103` & `:140` and `ProjectSettingsSheet.swift:127`; `xcodebuild -scheme ScriviApp -destination 'platform=macOS' build` → **BUILD SUCCEEDED** with all three warnings gone. |
| I-0085 | `[Apple]` **App crashes constructing the Open Project panel.** From `LandingView` (or File ▸ Open Project…), clicking Open calls `AppEnvironment.presentOpenProjectPanel()` → `NSOpenPanel()`, which aborts with `*** Assertion failure in -[NSOpenPanel _initBridgeAndStuff], NSSavePanel.m:466 — Advance to configuration phase semaphore timed out.` This fires at panel **construction** (`NSOpenPanel()`), before `runModal`. **Investigation (2026-07-21):** the assertion is the sandbox Open/Save-panel ViewBridge XPC service (`com.apple.appkit.xpc.openAndSavePanelService`) timing out its init handshake. Evidence it is **environmental, not a code defect**: (a) this exact `NSOpenPanel()` code dates to **SP-035** and is unchanged through ~40 sprints of working Open flows; (b) entitlements are correct (`app-sandbox` + `files.user-selected.read-write`) and `ENABLE_APP_SANDBOX = YES`; (c) **a second Scrivi instance was found running** (`…Scrivi -NSDocumentRevisionsDebugMode YES`, an Xcode debug launch) alongside the `open`-launched one — two instances of a sandboxed app contend for the single per-user panel XPC service, a known trigger for this timeout — even though `LSMultipleInstancesProhibited = true`; (d) host is **macOS 27.0 beta (26A5378n)**, where ViewBridge XPC flakiness is common; (e) **not merge-related** — `presentOpenProjectPanel` is untouched by SP-075. **Resolution (2026-07-21):** confirmed **environmental, NOT a code defect.** The user quit all instances and relaunched clean; the Open Project dialog constructed and ran fine, exactly as the investigation predicted (duplicate sandboxed instance + macOS 27.0 beta panel-XPC handshake timeout). No code change made or warranted — `presentOpenProjectPanel`/`NSOpenPanel()` are unchanged and correct. Closed as Not-a-Bug. | High | **SP-075** (found) | ✅ **Closed — Not a Bug (2026-07-21)**: clean relaunch reproduced the working Open dialog; the crash was the environmental duplicate-instance + beta-OS XPC panel timeout, not Scrivi code. |
| I-0084 | `[Apple]` **Caret jumps to the next scene/chapter start after a scene merge.** After `⌘-Backspace` scene-merge (SP-075 T-0303), the cursor lost its intended seam position and landed at what looked like the start of the following chapter/scene. **Root cause:** `handleMergeScene` mutates `loader.segments` (`@Observable`), which schedules a `updateNSView` pass; that pass rebuilt storage a SECOND time (its `segIDs != lastSegmentIDs` guard was still true because the coordinator's manual `rebuildStorage` never updated `lastSegmentIDs`), and the redundant `setAttributedString` dropped the selection to 0 — stomping the caret that `rebuildStorageAndPlaceCursor` had just placed. Latent in the original merge code; SP-075's added `await` exposed it. **Fix:** `rebuildStorage` now stamps `lastSegmentIDs`/`lastShowChapterTitles` at the end, so the follow-up `updateNSView` sees no change and skips the redundant rebuild — the caret placement survives. Benefits every manual-rebuild handler (merge/split/create). | Medium | **SP-075** | ✅ Resolved - **Verified (2026-07-21)** — fix in `ManuscriptTextView.rebuildStorage`; user confirmed in the GUI that the caret holds its seam position after a scene merge (no jump to the next scene/chapter start). |
| I-0083 | `[Apple]`+`[ScriviCore]` **Chapter-merge loses scenes on reopen.** The macOS `⇧⌘-Backspace` chapter-merge (`ManuscriptTextView.handleMergeChapter`) reassigns the merged chapter's scenes to the predecessor chapter **in memory only** (`ViewportSceneLoader.mergeChapterIntoPredecessor`), then calls `scrivi_delete_chapter(currentChapterID)`. Under EP-027 the scene `.md`/`.meta.json` files physically live in the chapter's own `chapter-<orderKey>/` folder, so `deleteChapter` removes the folder **and every scene file the in-memory model believed it had preserved** → on quit/reopen those scenes are gone. **Fix (EP-028/SP-074):** a first-class atomic `scrivi_merge_chapter` that **relocates** the scene files into the predecessor folder (minting order keys after the predecessor's last scene) before removing the emptied chapter; the app calls it instead of composing merge from delete. | High | **SP-074/SP-075** | ✅ Resolved - **Verified (2026-07-21)** — core `scrivi_merge_chapter` (T-0300) relocates every scene file into the predecessor before removing the emptied chapter; macOS app adoption (SP-075 T-0303) points `handleMergeChapter` at it, retiring the lossy `deleteChapter` composition. User confirmed in the GUI that a chapter-merge survives quit→reopen with every scene present and in order. |
| I-0082 | `[Linux]` **Chapter drag never starts — the heading can't be picked up; the drag rubber-band-selects the nearest scene instead.** Found in the SP-073 VNC walkthrough (chapter selection box shows pale blue = non-selectable; dragging a heading just moves the scene selection up/down). **Root cause:** `QAbstractItemView::mouseMoveEvent` enters `DraggingState` (and calls `startDrag`) only when the pressed row is among the **selected** draggable indexes — and chapter rows were `setSelectable(false)` (SP-061 "chapters group; scenes select"), so a chapter row could never be in the selection and the view fell into rubber-band drag-selection instead. The SP-073 manual-`QDrag` path in `NavigatorTree::startDrag` was correct but unreachable. **Fix:** chapter rows are now selectable (`rebuildNavigator`); clicking a heading is harmless (`onNavigatorActivated` ignores rows without a sceneID). Headless smokes can't drive a real mouse drag — VNC-only class, caught exactly where expected. | High | **SP-073** | ✅ Resolved - **Verified (2026-07-19, VNC round 2)** — chapter drag works (teal selection, container move, persists across quit→reopen) |
| I-0081 | `[ScriviCore]`+`[Linux]` **Scene rename (and save) fails after a drag-reorder — the segment's paths are stale.** Found in the SP-073 VNC walkthrough: a scene moved to another chapter can't be renamed until quit→reopen (fresh paths), while newly-created scenes rename fine. **Root cause:** under EP-027 §8, `scrivi_reorder_scene` **renames the scene's files** to their new order-key stem — and a cross-chapter move **relocates them** into the target chapter's folder — but its envelope reported no paths, and `EditorShell::onSceneDropped` (SP-067 code, written pre-§8 when a reorder was an index shuffle) kept the pre-move `metadataPath`/`contentPath`/`chapterMetadataPath` in the segment. A later `rename_scene`/`save_scene` through them targets vanished files (the I-0074/I-0079/SP-073-chapter stale-path class — this closes the last member: scene drag). **Fix:** `ReorderSceneResult` + the `reorder_scene` envelope now report the post-move `metadataPath`/`contentPath`/`chapterMetadataPath` (payload-only, `scrivi.h` untouched); `SceneDocument::refreshScenePaths` applies them in `onSceneDropped`. Core regression `reorderScene - reports the scene's post-move paths (I-0081)` + smoke coverage (rename+save through refreshed paths in `scene_reorder_smoke` Case C). | High | **SP-073** | ✅ Resolved - **Verified (2026-07-19, VNC round 2)** — a drag-reordered scene renames immediately, no restart needed |
| I-0080 | `[ScriviCore]` **Open-time chapter migration UNDOES a legitimate chapter reorder** — `migrateChapterOrderKeys` ran on every open with **no legacy gate**: after a `scrivi_reorder_chapter` (whose index-array order is stale by design under B3 — disk is the authority, the array a cache), the next open treated the disk-vs-array disagreement as "legacy project needing migration" and **reslugged the folders back to the stale array order**, reverting the user's reorder. Previously masked only because the reslug usually crashed into `renameChapterFolder`'s no-clobber guard and silently aborted (`ProjectOpener` discards its result). Found by SP-073's new `chapter_reorder_smoke` (Case C move-to-front) and reproduced at core level (`ReorderTests` `[SP-073]` — move-between survived only by no-clobber luck; move-to-front was undone by the following open). **Fix:** (a) **legacy gate** — the reslug runs only when EVERY on-disk chapter folder key is digits-only (the pre-EP-027 creator's shape); any letter key = new scheme → skip (a stale array is `rebuildIndexIfInconsistent`'s job); (b) **eager cache coherence** — `ChapterReorderer` + `ChapterCreator` call `rebuildIndexIfInconsistent` after their folder ops so the array order always matches disk (create-in-place's blind append had the same staleness). I-0077's regression fixture reshaped to a genuine all-numeric legacy project (matching the real `the-twisted-remains` case). Regression test proven RED without the fix. | High | **SP-073** | ✅ Resolved - **Verified (2026-07-19, VNC round 2)** — chapter reorder persists across quit→reopen (the exact failure mode); ctest **306/306 macOS, 313/313 Linux**; **11/11** Linux smokes |
| I-0079 | `[Linux]` **Scene-reorder smoke test fails on CI** (`FAIL: E: tail landed in K's first scene`) — Linux App CI red since "EP-027 P1 and P2 complete" (2026-07-16). `scene_reorder_smoke.cpp` Case E (mid-scene chapter-split) replayed the **old** split orchestration: `createChapter()` (append) → `reorderChapter(K, ch1)` → `saveScene(tail, firstSceneMetadataPath/ContentPath captured from the createChapter result)`. Under EP-027 P2, `reorderChapter` **reslugs K's folder**, so the captured scene paths went stale and the `saveScene` wrote to the vanished path → the tail was lost on reopen (the same I-0074 stale-path class, but in the test's own manual orchestration). **Fix:** both Case D and Case E now **create K in place** — `createChapter(projectID, afterChapterID=ch1)` (the app's post-I-0074 orchestration) — so K's folder is born in its final position and no reslug invalidates the paths. Reproduced + fixed in the Qt 6.4 Docker container; all 10 Linux console smokes green. | Medium | — | ✅ **Verified (2026-07-18)** — fix in `platforms/linux/tests/scene_reorder_smoke.cpp` (test-only) committed as `1ca59eb`; **Linux App CI #15 (run 29662554648) SUCCESS**, ending the red streak that ran since 2026-07-16. |
| I-0076 | `[ScriviCore]` **EP-027 migration leaves legacy scenes with a stale `content.path` → "Missing scene content file" on open.** A legacy project whose scene sidecars carry a full `content.path = "manuscript/chapter-NNN/…md"` (pre-P6 shape) migrates its chapter folders to order-keys (`chapter-001`→`chapter-V`, …) but the scene `content.path` is **never normalised to a bare filename**, so it still points at the vanished `chapter-NNN` folder → `ProjectValidator` reports "Missing scene content file" and the app refuses to open. **Root cause:** `util::isOrderKey("001"/"002"/"003"/"004")` returns **true** (base-62 alphabet accepts digits; only a trailing `'0'` is rejected), so `SceneIndex::migrateScenes` Pass 2 classifies zero-padded numeric scene filenames as *already order-keyed* and **skips** them — `renameSceneFiles` (which rewrites `content.path` to a bare filename) never runs. Compounded by `renameChapterFolder`, which by design touches **zero** scene fields (it assumes `content.path` is already bare — true for P6-created projects, false for these legacy sidecars). Found in P5 on `the-twisted-remains-of-myself.scrivi` (13/14 chapters affected; only the un-migrated `chapter-004` had correct paths). ctest-invisible (fixtures already use bare content paths). | High | **EP-027** | ✅ Resolved - Not Verified (2026-07-18). **Fix:** (a) `parseSceneMeta` normalises `content.path` → bare filename on read (`SceneMetaJson.cpp`), so every consumer resolves it in the scene's own folder regardless of stored shape → the project OPENS; (b) `migrateScenes` gains `normalizeSceneContentPathIfStale` — rewrites a canonical-key sidecar whose stored path is still full, idempotently (`SceneIndex.cpp`). `isOrderKey` left unchanged (its digit-acceptance is correct for the key space). Regression test `open - normalises a legacy full-path scene content.path… (I-0076)` — verified RED without the fix. ctest 304/304 macOS, 311/311 Linux. ✅ **Verified (2026-07-18)** — user opened a fresh copy of the real legacy project in the rebuilt macOS app; it opened cleanly with all chapters + content. |
| I-0078 | `[Apple]` **macOS app failed to build — `ScriviEngine.swift` `createChapter` wrapper drifted behind the C ABI.** SP-071 (I-0074 create-in-place) added a 7th `afterChapterID` parameter to `scrivi_create_chapter` in `scrivi.h` and updated only the Linux bridge; `ScriviEngine.createChapter` still called it with 6 args → `error: missing argument for parameter #7` (`ScriviEngine.swift:601`). The macOS target had not compiled since c949d0b. **Fix:** thread `afterChapterID: String = ""` (empty ⇒ append, preserving prior macOS behavior) through the wrapper via `withCString`; both macOS callers only append, so no other change. Build succeeds. Found at the start of P5. | High | **EP-027** | ✅ **Verified (2026-07-18)** — fix applied to `Scrivi/Engine/ScriviEngine.swift`; `xcodebuild ScriviApp` BUILD SUCCEEDED and the user ran the rebuilt app (opened a project). |
| I-0077 | `[ScriviCore]` **EP-027 chapter migration skips a chapter whose index/sidecar `chapterID` disagree, leaving it half-migrated.** When `manuscript.meta.json` lists a chapter under one `chapterID` but the chapter's sidecar has a different one, `migrateChapterOrderKeys::keyForID` can't map the index id to any on-disk folder, so line 227 treats it as a **phantom index entry and drops it** from the migration set. That chapter keeps its legacy numeric folder (`chapter-004`) and old-scheme scene refs (`metadataPath`+`sceneID`) while every other chapter is reslugged to order-keys; because numerics sort before letters it also lands **first** in reading order (wrong position). Open-time self-heal then adopts the sidecar id into the index but does **not** re-run the folder migration, so the half-migrated state persists. Found in P5 (`chapter-004`, index `…942e…` vs sidecar `…9cdd…`). | Medium | **EP-027** | ✅ Resolved - Not Verified (2026-07-18). **Fix:** `migrateChapterOrderKeys` resolves each index ref to its current folder by chapterID **then falls back to the ref's `path` folder** when the id doesn't match a sidecar (`keyForRef` in `ChapterIndex.cpp`); a mismatched-but-present chapter now participates in the reslug instead of being dropped as phantom. Only a ref with neither an id match nor an existing folder is skipped. Regression test `open - migrates a chapter whose index/sidecar chapterID disagree (I-0077)` — verified RED without the fix (the `chapter-001` folder survived). ctest 304/304 macOS, 311/311 Linux. ✅ **Verified (2026-07-18)** — user opened a fresh copy of the real legacy project (which had the `chapter-004` id mismatch) in the rebuilt macOS app; it opened cleanly. |
| I-0075 | `[Linux]` Arrow keys can't cross a scene/chapter boundary: pressing Down at a scene's end or Up at a chapter's start leaves the caret stuck. `ManuscriptEditor::normalizeCaret` snapped to the **nearest** editable edge, which (tie → previous) always snapped the caret **back** the way it came. **Fix:** directional snap — `SceneDocument::editablePositionInDirection(pos, movingForward)` snaps to the NEXT body start when travelling forward (Down/Right) and the PREV body end when backward (Up/Left); `normalizeCaret` picks direction from `pos` vs. the previous caret position. `nearestEditablePosition` kept for clicks/paste. Smoke-test assertions added (`editor_map_smoke`). | Low | **SP-071** | ✅ **Verified (2026-07-18, VNC)** — arrows cross scene/chapter boundaries in both directions. |
| I-0074 | `[Linux]` Chapter split (Ctrl+Shift+Return) corrupted the manuscript. **TWO defects.** (a) **Lost tail / stray folder:** the app did `createChapter` (append → `chapter-w`) then `reorderChapter` (rename → `chapter-c`), so the tail `saveScene` wrote to the **stale pre-rename path** and K0 stayed empty. Fix = **create-in-place** (`createChapter(afterChapterID)`). (b) **Same-named scene STOLEN from another chapter → "needs repair, missing metadata" on reopen:** after a follower reordered OUT of Ch2, `rebuildChapterScenesIfInconsistent` **preserved the stale ref**, then `migrateScenes` orphan-repair **matched orphans by filename** and dragged Ch1's identically-named `k-scene` into Ch2 (order-key scene filenames repeat across chapters). Fix = scene cache is a **pure disk mirror** (drop stale refs) + **removed filename-based orphan relocation** (ownership follows physical location; a project always opens self-consistent). Also: removed obsolete "renumber N chapters" dialog (C1), stopped deleting K0 at end-of-scene (C3), surfaced create-failure + **abort-on-any-failed-split-step** instead of silent continue (C4); **removed the split confirmation dialog entirely** (Ctrl+Shift+Return is the approval — a modal prompt broke drafting flow; the split is non-destructive). | High | **SP-071** | ✅ **Verified (2026-07-18, VNC)** — all four split cases (mid-scene, mid-scene-with-followers, end-of-scene-with-followers, end-of-last-scene) update in the UI and **reopen cleanly with no warnings/errors**. ctest 302/302 macOS. |
| I-0072 | `[ScriviCore]` `chapter-<count+1>` slug collides after a delete → clobbers a sidecar + phantom/duplicate index entries in `manuscript.meta.json`; breaks `reorder_scene` ("sourceChapterID not found") | High | **EP-027** | 🔵 Root defect of **EP-027** (A4b+B3 rework fixes the class) |
| I-0071 | `[Linux]` Dragging a chapter's **last remaining scene** into another chapter orphans an empty chapter (no delete, no replacement scene) | Medium | **EP-027** | ✅ Resolved - **Verified (2026-07-18, VNC)** — last-scene drag leaves no empty/orphaned chapter (P6 scene model). |
| I-0070 | `[Linux]` Ctrl+Shift+Return at **end-of-scene with no followers** appends the new chapter at the manuscript end instead of inserting it after the current chapter (I-0064 residual for this branch) | Medium | **EP-027** | ✅ Resolved - **Verified (2026-07-18, VNC, B4)** — P6 model (see I-0074/SP-071). |
| I-0069 | `[Linux]` Ctrl+Shift+Return at **end-of-scene with followers** renumbers but produces no visible new chapter/scenes (followers not moved / K not shown) | Medium | **EP-027** | ✅ Resolved - **Verified (2026-07-18, VNC, B4)** — P6 model (see I-0074/SP-071). |
| I-0073 | `[Linux]` Scene drag-drop has a **1–2 s lag** before the drop target/insertion line is realized (observed over Docker+VNC; may be environmental) | Low | — | 🔵 Open — needs triage (VNC vs. code) |
| I-0068 | `[Linux]` On a scene drag-drop the scene **disappears from the navigator** and no new order is shown (Qt MoveAction auto-removes the source row) | High | SP-067 | ✅ Resolved - **Verified (2026-07-16, user "It's clean" over VNC on a fresh project)** |
| I-0067 | `[Linux]` Scene drag-reorder **does not persist** — Qt auto-removes the row while the backend move fails/aborts; order reverts on quit→relaunch (AC4) | High | SP-067 | ✅ Resolved - **Verified (2026-07-16, user "It's clean" over VNC on a fresh project)** |
| I-0066 | `[ScriviCore]`/`[Apple]` A scene/chapter deleted from the navigator leaves its history in the log (no barrier, no prune) — orphaned diffs accumulate and are what mismatched on open (I-0065) | Medium | — | ✅ Resolved - Verified (2026-07-15) |
| I-0065 | `[ScriviCore]`/`[Apple]` A mismatched/stale history diff crashes the macOS app on project open — a C++ `std::length_error` in history replay-on-load crosses the C ABI and terminates the process | High | — | ✅ Resolved - Verified (2026-07-15) |
| I-0064 | `[Linux]` Ctrl+Shift+Return appends an empty chapter at the manuscript end instead of splitting/inserting the chapter at the caret (no scene reassignment, no renumber) | Medium | **EP-027** | ✅ Resolved - **Verified (2026-07-18, VNC, B3)** — split inserts the chapter at the caret (P6 model, see I-0074/SP-071). |
| I-0063 | `[Linux]` Deleting/inserting a chapter doesn't renumber later **created** (stored-"Chapter N") chapters | Low | SP-067 | ✅ Resolved - **Verified (2026-07-16, user-confirmed over VNC)** |
| I-0062 | `[Linux]` A newly-created chapter's heading reads "Chapter" (not "Chapter N") until the project is reloaded | Low | SP-066 | ✅ Resolved - Verified (2026-07-15) |
| I-0061 | `[Linux]` Landing **Quit** button does nothing after the shell flip (`QQmlEngine::quit()` unconnected) | Medium | SP-062 | ✅ Resolved - Verified (2026-07-14) |

**Verified, awaiting batch archive:** I-0051, I-0053, I-0054, I-0055, I-0056 (all Verified 2026-06-29), I-0052 (Verified 2026-06-26), I-0057 (Verified 2026-07-01), and **I-0058** (Verified 2026-07-09; full entry in `Issue-backlog.md`) — full entries retained until the I-0051–I-0060 batch is archived (pending I-0059/I-0060).

---

## I-0073: [Linux] Scene drag-drop has a 1–2 s lag before the drop target is realized

**Status:** 🔵 Open — needs triage. Observed by the user during I-0067/I-0068 VNC verification (2026-07-16):
the drop target / insertion line takes "a second or two" to catch up while dragging. **Unknown whether this is
environmental (Docker+VNC) or a code issue** — flagged so it isn't lost.
**Platform:** Linux (`platforms/linux/`), observed **over the Docker+VNC harness** (not real hardware).
**Component:** drag-hover path — `NavigatorTree::dragMoveEvent` (`NavigatorTree.cpp:117`); or, more likely, the
Xvfb → x11vnc → VNC-client pointer-motion pipeline.
**Severity:** Low (cosmetic responsiveness; the drop itself resolves correctly — I-0067/I-0068 verified clean).
**Sprint:** — (triage first)
**Epic:** EP-023 `[Linux]`

**Description:** while dragging a scene row, the insertion-line drop indicator lags the cursor by ~1–2 s before
the landing position updates.

**Analysis (code path is cheap — points away from our code):** `dragMoveEvent` does only `QTreeView::
dragMoveEvent` (base indicator/autoscroll) + `indexAt` + `resolveDrop` (a handful of `data()` reads and string
compares) — **no I/O, no backend call, no notable allocation.** Nothing on the per-move path costs ~1 s.
High-frequency pointer motion during a drag is exactly what a remote framebuffer protocol (VNC over
Xvfb-in-Docker) throttles/batches, so the most probable cause is **environmental round-trip + frame-encode
latency**, not compute.

**Triage plan (before treating as a code bug):**
1. **Real-hardware datapoint** — the alpha tester runs the drag on native Ubuntu (no VNC). If smooth there, it's
   the harness → close as environmental / won't-fix (harness-only).
2. If it lags on real hardware too, profile `dragMoveEvent` (is `indexAt` or `dropIndicatorPosition` unexpectedly
   costly on this model?) and check whether the base `QTreeView::dragMoveEvent` autoscroll timer or a repaint is
   the cost; consider throttling our own resolve or reducing repaints.

**Do not act until step 1 gives a real-hardware reading** — optimizing a VNC-only artifact would be wasted work.

---

## I-0072: [ScriviCore]/[Linux] manuscript.meta.json chapters[] diverges from the chapter sidecars on disk

**Status:** 🔴 Open — discovered while instrumenting the I-0067/I-0068 drag failure (2026-07-16). The reorder
rejection `"sourceChapterID not found: chapter_019e9cdd-…"` traced to a **corrupt manuscript index**, not the
drag code.
**Platform:** Data/`[ScriviCore]` (whatever writes `manuscript.meta.json`); surfaced on `[Linux]`.
**Component:** `manuscript/manuscript.meta.json` writer — the chapter-create/reorder/split orchestration that
edits `structure.chapters[]`. On Linux this is `EditorShell::onCreateChapterRequested`
(`create_chapter`→`reorder_chapter`→…) and `ScriviCore` `ChapterCreator`/`ChapterReorderer`. Reader that trips
on it: `SceneReorderer::reorder` (`ScriviCore/src/manuscript/SceneReorderer.cpp:40-51`).
**Severity:** High (a manuscript whose index disagrees with its sidecars makes `reorder_scene` — and any op
keyed on the chapter list — fail or misbehave; latent structural corruption)
**Sprint:** SP-067 (found here; the writer is likely today's split path)
**Epic:** EP-023 `[Linux]`
**Related:** I-0067/I-0068 (the drag failure this explains); I-0064/I-0069/I-0070 (chapter-split — the most
likely corruptor); **I-0065** (the SAME project, "The Twisted Remains of Myself," crashed macOS on open — this
project has been a repeated corruption vector; its state may predate today and mix several defects).

**Description:**
In the project **"The Twisted Remains of Myself"**, `manuscript/manuscript.meta.json`'s `structure.chapters[]`
does not match the chapter sidecars on disk:
- **Wrong id for a real chapter:** the index lists `chapter-004` as `chapter_019e942e-…`, but
  `manuscript/chapter-004/chapter.meta.json` actually contains `chapter_019e9cdd-…`. The id `019e942e` appears
  in **no** sidecar (phantom).
- **Duplicate path:** `manuscript/chapter-015/chapter.meta.json` is listed **twice**, under two different
  chapter ids (`019ed110-…` and `019f6b84-…`).
- **Missing/nonexistent dirs referenced or skipped:** dirs `chapter-011` and `chapter-014` don't exist; the
  index ordering (…, chapter-007, chapter-017, chapter-008, …) is scrambled.

Because the scene being dragged reports `chapterID = 019e9cdd` (from its sidecar, correctly surfaced by
`open_project`), but the manuscript index has `chapter-004` under the phantom `019e942e`, `SceneReorderer`
walks `chapters[]`, never finds `019e9cdd`, and returns `invalidArgument: "sourceChapterID not found"`.

**Expected Behavior:** `manuscript.meta.json`'s `chapters[]` is always a faithful, de-duplicated, correctly
ordered list of the real chapter ids/paths on disk; structural edits keep it consistent (atomic, no phantom or
duplicate entries).

**Actual Behavior:** the index carries phantom ids, a duplicated path under two ids, and references to absent
dirs — diverging from the sidecars.

**Root Cause — CONFIRMED (2026-07-16, forensic disk + timestamp analysis).** The bug is in
**`ScriviCore/src/manuscript/ChapterCreator.cpp`**: the new chapter's on-disk **directory slug is derived from
the chapter *count*** — `newChapterOrdinal = ms.chapters.size() + 1` (L38) → `chapterSlug = "chapter-<ordinal>"`
(L42-44) → `chMetaRelPath` (L46-48). This is **not unique once any chapter has been deleted** (gaps make
`count+1` land on an existing directory number). On such a collision the create path:
1. `atomicWriteTextFile`s the new chapter's `chapter.meta.json` over the **already-existing** colliding
   directory's sidecar (L100) → **destroys the colliding chapter's identity** (its real id now survives ONLY as
   a stale entry in the index = phantom id), and
2. `ms.chapters.push_back({newID, chMetaRelPath})` (L104) **appends a second index entry for that same path**
   → duplicate path; the pre-existing entry now points at an id no sidecar has.

Both observed corruptions are this one bug:
- **Entry 3** (`chapter-004` path, phantom `019e942e`): UUIDv7 timestamps show `019e942e` was a real chapter
  created **Jun 4 19:49**, later clobbered when a `count+1` collision reused the `chapter-004` slug and
  overwrote its sidecar with `019e9cdd` (Jun 6). Index kept the original `019e942e` ref → phantom.
- **Entries 13/14** (`chapter-015` path listed twice): the current `chapter-015` sidecar `019f6b84` was written
  **today Jul 16 15:20** (matches the 11:29 index rewrite and the user's VNC split test), clobbering the
  **Jun 16** chapter `019ed110` that had that slug — leaving `019ed110` as a duplicate/phantom index entry.
  **This is the user's Failure-2 ("a new Chapter 15 was created" at end-of-Ch4/Scene-3), caught by timestamp.**

The user's Failure-1 (end-of-Ch4/Scene-2, follower present → "no new chapter created") is the same collision in
the **with-followers split**: `create_chapter` collides and clobbers the source chapter's sidecar, then a
downstream `reorder_scene` fails on the now-mismatched ids and the orchestration aborts — so the chapter was
"re-identified before the failure stopped creation" (the user's exact hypothesis, confirmed by the code path).

**Provenance:** the corruption is entirely application-generated, triggered by the user's **in-app** split/create
actions during VNC testing (NOT manual file edits, NOT Claude's automated tests). The user did nothing wrong —
they exercised a latent `ChapterCreator` slug-collision defect.

**Architecture study (2026-07-16):** the slug scheme + a second issue the user surfaced (`chapterID` is stored
in BOTH `manuscript.meta.json` and `chapter.meta.json` with no declared source of truth) are analyzed in
**`docs/Scrivi_Chapter_Folder_and_Identity_Trade_Study_v0_1.md`** (folder-naming options A1–A4 + id/order
source-of-truth options B1–B3). The durable fix for I-0072 should follow that study's decision; the minimum
safe close is "A2 monotonic slug + B1 sidecar-owns-id + a validator that prunes index entries whose sidecar id
differs." **Awaiting the user's decision on the trade study before implementation.**

**Immediate impact on SP-067 verification:** drag-reorder (and possibly split) **cannot be trusted-tested on
this project** — its index is already corrupt. Re-run AC4 verification on a **freshly created** project. Also
provide/confirm a repair path (rebuild `manuscript.meta.json` from the sidecars) for damaged projects.

**Fix direction (now scoped — the primary fix is in ChapterCreator):**
1. **`ChapterCreator` slug must be collision-free.** Do NOT derive the directory slug from `chapters.size()+1`.
   Instead pick a slug that can't collide — e.g. `max(existing chapter-NNN dir number)+1`, or a slug derived
   from the unique `chapterID`, or scan-and-skip existing dirs. **Never `atomicWriteTextFile` over a path that
   already exists** for a *new* chapter (a create should fail or pick a fresh dir, never clobber). This is a
   `[ScriviCore]` change affecting all platforms (Apple included) — same collision can occur on macOS after a
   delete; confirm and cover there too.
2. **De-dup / repair the index.** Add an integrity check that rebuilds/repairs `manuscript.meta.json`
   `chapters[]` from the authoritative on-disk sidecars (drop phantom ids whose path's sidecar has a different
   id; collapse duplicate paths) — a load-time self-heal in the spirit of I-0066's history prune and the
   External-Change-Repair matrix. Needed to recover already-damaged projects like this one.
3. `reorder_scene`/consumers already fail loudly ("sourceChapterID not found") — keep that; the durable fix is
   #1 (stop creating the corruption) + #2 (heal existing damage).

**Repro plan (to lock the fix):** fresh project → create several chapters → **delete** one (make a gap) →
create another → assert its dir slug did not collide and `chapters[]` stays 1:1 with the sidecars. Extend to
the split path (I-0064/I-0069/I-0070) once the create is fixed.

**Files:**
- **`ScriviCore/src/manuscript/ChapterCreator.cpp:38,42-48,100,104`** — the slug-collision + clobber + duplicate
  (PRIMARY fix)
- `ScriviCore/src/manuscript/SceneReorderer.cpp:40-56` — where the mismatch is detected (fails correctly)
- `ScriviCore/src/manuscript/ChapterReorderer.cpp` — verified clean (id-keyed, atomic; NOT a corruptor)
- `platforms/linux/src/EditorShell.cpp` `onCreateChapterRequested` — split orchestration that triggers create
- affected data: `…/the-twisted-remains-of-myself.scrivi/manuscript/manuscript.meta.json` (needs repair or
  discard; it's a damaged test project)

---

## I-0067: [Linux] Scene drag-reorder does not persist across quit→relaunch (AC4)

**Status:** 🟠 **ROOT-CAUSED via instrumented VNC (2026-07-16) — fix not yet written.** The two prior
hypotheses (wrong/empty dragged id; success-but-no-op reorder) were BOTH falsified by runtime evidence. The
`startDrag` latch (kept — it's still more correct) had no effect because id-resolution was never the failing
step. **Confirmed cause (unified with I-0068 — one event, both symptoms, as the user predicted):** on a drag,
`NavigatorTree` uses `setDragDropMode(DragDrop)` + `setDefaultDropAction(Qt::MoveAction)` and `dropEvent`
calls `event->acceptProposedAction()` **before** the backend move is known to succeed. `onSceneDropped` then
calls `scrivi_reorder_scene`, which **failed** for this project (see below), so the handler correctly did
NOTHING (early-returned on the empty envelope) — **hence no persist**. But because the drop was accepted as a
`MoveAction`, **Qt's own `QAbstractItemView::startDrag` removed the source row from the model** after
`dropEvent` returned — **hence the vanish (I-0068)**. The removal (Qt) and the persist (ScriviCore) are
decoupled and the former isn't gated on the latter.

**Why `reorder_scene` failed here:** instrumentation captured the raw envelope —
`{"ok":false,"error":{"code":1,"message":"sourceChapterID not found: chapter_019e9cdd-…"}}`. The test project
**"The Twisted Remains of Myself"** has a **corrupt `manuscript.meta.json`** whose `chapters[]` list disagrees
with the on-disk chapter sidecars (e.g. it lists `chapter-004` as `019e942e` — a phantom id in no sidecar —
while `chapter-004/chapter.meta.json` actually holds `019e9cdd`; also lists `chapter-015` twice and references
nonexistent `chapter-011`/`014`). So the scene's real chapter (`019e9cdd`) isn't in the manuscript index →
ScriviCore can't find the source chapter → rejects. **That data corruption is filed separately as I-0072**
(likely fallout of today's I-0064/I-0069/I-0070 split path — same project that also crashed macOS in I-0065).

**Fix direction (app layer — this Issue):** the navigator must change ONLY via our own `rebuildNavigator`, never
via Qt's drag auto-remove.

**FIX IMPLEMENTED (2026-07-16, SP-067 T-0260):** `NavigatorTree` now runs the drag as **`Qt::CopyAction`, never
`MoveAction`** — `startDrag` forces `QTreeView::startDrag(Qt::CopyAction)`, the ctor sets
`setDefaultDropAction(Qt::CopyAction)`, and `dragMoveEvent`/`dropEvent` `setDropAction(Qt::CopyAction)` before
accept. With a Copy drop the base class never removes the source row, so the navigator changes *only* via
`sceneDropRequested → EditorShell::onSceneDropped → rebuildNavigator`. A backend-rejected/failed reorder now
leaves the tree untouched (no vanish); a successful one reflects solely through our rebuild. TEMP drag
instrumentation removed. Container build + 275 ctest smokes green; **needs VNC re-verification on a FRESH
(non-corrupt) project** — the old test project's I-0072 index corruption would still make `reorder_scene`
legitimately fail (that's EP-027's job).

**Prior finding (2026-07-16):** VNC cases 2a/2b: dragging a scene appeared to move it live (the row left its
position), but after Quit→relaunch the chapter returned to its original order. Now explained: the disk move
never happened; only Qt's row-removal did.
**Platform:** Linux (`platforms/linux/`)
**Component:** `platforms/linux/src/EditorShell.cpp` (`onSceneDropped`, ~L528–582) →
`ScriviBridge::reorderScene`; possibly `NavigatorTree::draggedSceneID`/`resolveDrop`
(`NavigatorTree.cpp`).
**Severity:** High (AC4 — the headline SP-067 deliverable — does not work end-to-end; the move is not durable)
**Sprint:** SP-067 (regression against T-0260's exit criterion "the new order persists across quit→reopen")
**Epic:** EP-023 `[Linux]`
**Related:** I-0068 (the sibling live symptom — scene vanishes); T-0260/T-0263. **Note:** the headless
`scene_reorder_smoke` **Case C passes** — it calls `ScriviBridge::reorderScene` directly and confirms the
reopened on-disk order changed. So the bridge + ScriviCore endpoint persist correctly *when called with the
right arguments*; the fault is in the **live app path** that computes and issues that call, which the smoke
never exercises.

**Description:**
A scene dragged in the navigator does not keep its new position after the project is closed and reopened —
the manuscript reverts to the pre-drag order on disk.

**Expected Behavior (AC4):** the drop calls `scrivi_reorder_scene`, the continuous viewport + map re-splice
to the new order, and **the new order persists across quit→reopen**.

**Actual Behavior:** live view changes momentarily (see I-0068), but disk is unchanged; quit→relaunch shows
the original order.

**Root Cause (hypothesis — needs a live-path debug run to confirm):**
The persistence primitive is proven good by `scene_reorder_smoke` Case C, so the break is in `onSceneDropped`
issuing the bridge call. The most probable causes, in order:
1. **`reorderScene` returns empty and the early `return` at `EditorShell.cpp:557` fires**, so `moveScene`
   never runs on disk — but then the row wouldn't "disappear" live, so this alone doesn't fit I-0068. More
   likely the call is made with **wrong/empty arguments** (`draggedSceneID`, `sourceChapterID`,
   `targetChapterID`, or `afterSceneID`), so ScriviCore rejects or no-ops it while the *in-memory* `moveScene`
   still splices (producing the visual change that reverts on reload).
2. **`draggedSceneID()` resolves from `currentIndex()`/selection** (`NavigatorTree.cpp:22–28`), not from the
   drag's actual source row. During a Qt drag the current index can differ from the grabbed row, so the wrong
   (or empty) sceneID is sent to `reorderScene`. This also explains I-0068 (the *visually* removed row is the
   selected one, not necessarily the one reordered on disk).
3. **`sourceChapterID` mismatch:** `onSceneDropped` reads `moved.chapterID` from the segment at `fromIdx`; if
   `fromIdx` was resolved from a stale/again-selected scene, the source chapter passed to `reorder_scene` is
   wrong and ScriviCore's move is a no-op.

**5-Whys:**
1. *Why did the reorder not persist?* — Because `scrivi_reorder_scene` did not change the on-disk index for
   the intended scene.
2. *Why didn't it change disk?* — Because the live `onSceneDropped` call either returned empty (early-return
   before disk write) or issued the call with arguments that don't identify the intended move.
3. *Why were the arguments wrong / the call empty?* — Because `draggedSceneID`/`sourceChapterID` are derived
   from the tree's **current selection** at drop time rather than from the drag's committed source row, and a
   Qt drag can leave the selection pointing elsewhere.
4. *Why was selection used instead of the drag source?* — Because `NavigatorTree` never captured the source
   sceneID at `startDrag`/drag-start; it re-reads `currentIndex()` in both `dragMoveEvent` and `dropEvent`.
5. *Why wasn't this caught before VNC?* — Because `scene_reorder_smoke` validates the **bridge + SceneDocument
   primitives directly** (Case C reorders and reopens successfully) but **never drives `onSceneDropped` or a
   real `QDropEvent`**, so the argument-derivation path had zero automated coverage. **Root cause: the live
   drag→drop→persist path has no test; only its primitives do.**

**Fix direction (proposed, not yet implemented):**
- Capture the dragged sceneID at **drag start** (override `startDrag`, stash the source row's `kSceneIDRole`
  and its parent chapterID) instead of reading `currentIndex()` at drop time; pass both into
  `sceneDropRequested`.
- In `onSceneDropped`, when `reorderScene` returns empty, do **not** run the in-memory `moveScene` (keep disk
  and view in lock-step — no optimistic splice that can't be persisted).
- Add a **live-path smoke** that constructs a real `QDropEvent` (or calls `onSceneDropped` with resolved
  args) and asserts the reopened on-disk order — closing the coverage gap named in why #5.

---

## I-0068: [Linux] Scene disappears from the navigator on drop (Qt MoveAction auto-removes the source row)

**Status:** 🟠 **ROOT-CAUSED via instrumented VNC (2026-07-16) — fix not yet written. SAME root event as
I-0067** (see it for the full evidence). The vanish is **Qt removing the dragged source row itself**: with
`DragDrop` mode + `defaultDropAction = MoveAction`, `dropEvent`'s `acceptProposedAction()` tells the drag loop
the move succeeded, so `QAbstractItemView::startDrag` calls its remove-source-rows path — deleting the row
from the `QStandardItemModel` — **regardless of whether `onSceneDropped` did anything.** In this project
`onSceneDropped` early-returned (backend rejected the move, I-0072), so the row was removed by Qt with no
compensating rebuild → it disappears until reload. The instrumentation proved our handler never touched the
tree (envelope empty → early return before `moveScene`).

**Fix direction:** shared with I-0067 — stop accepting a `MoveAction` in `dropEvent` so Qt never auto-removes;
let `rebuildNavigator` be the only thing that changes the model. The "outline box, no insertion line" in the
original 2a was a separate cosmetic (drop indicator gating), re-check after the fix.

**Prior finding (2026-07-16):** VNC 2a/2b: on drop the dragged scene disappeared from the navigator entirely
rather than reappearing at the drop position.
**Platform:** Linux (`platforms/linux/`)
**Component:** `platforms/linux/src/EditorShell.cpp` (`onSceneDropped` → `SceneDocument::moveScene` +
`rebuildNavigator`, ~L560–580); `NavigatorTree::dragMoveEvent` (indicator draw, `NavigatorTree.cpp:89–109`).
**Severity:** High (the reorder UI visibly loses the scene from the tree — alarming; the scene is not gone on
disk, but the navigator no longer shows it until reload)
**Sprint:** SP-067
**Epic:** EP-023 `[Linux]`
**Related:** I-0067 (same drop path; the persistence failure). Likely the same root cause (wrong
dragged/source id).

**Description:**
Dropping a dragged scene removes its row from the navigator instead of re-placing it at the new position. The
tree ends up missing the scene until the project is reloaded.

**Expected Behavior:** after the drop the scene appears at its new position in the navigator (and the
viewport reflows), caret preserved.

**Actual Behavior:** the row disappears; no re-inserted row is shown at the target.

**Root Cause (hypothesis — confirm with a live run):**
`onSceneDropped` runs the in-memory `SceneDocument::moveScene` (which lifts the scene out via `removeScene`
then re-inserts) and then `rebuildNavigator()` (which projects the tree from `segments()`). A disappeared row
means **either** (a) `moveScene` removed the scene but re-inserted it into a target the navigator then
doesn't render (e.g. wrong `targetChapterID`, so it's spliced under a non-existent/hidden chapter node),
**or** (b) `moveScene` returned `newIdx < 0` and the fallback full `load()` at `EditorShell.cpp:570` ran with
a bad title/argument and dropped the row, **or** (c) the dragged id and the reordered id diverged (I-0067
cause #2) so `removeScene` took out one row while nothing was re-inserted for it. The "outline box, no
insertion line" in 2a further suggests `dragMoveEvent` `ignore()`d the position (no accepted drop target), yet
`dropEvent` still emitted — i.e. the drop fired at a position the gate had rejected.

**5-Whys:**
1. *Why did the scene disappear?* — `rebuildNavigator` projected a segment list in which the dragged scene's
   row was removed but not re-inserted at a rendered position.
2. *Why was it removed-but-not-re-inserted?* — `moveScene` re-inserted it under a `targetChapterID` the
   navigator didn't render, or the dragged id ≠ reordered id so the removed row had no matching re-insert.
3. *Why did the ids diverge / the target come out wrong?* — Same origin as I-0067: `draggedSceneID`/target
   resolved from `currentIndex()` + drop-indicator at drop time rather than from the committed drag source.
4. *Why did the drop fire at a rejected position (2a)?* — `dropEvent` re-resolves `resolveDrop` independently
   of `dragMoveEvent`; if the indicator position at drop differs from the last accepted move position, the
   drop can proceed on a target the move gate would have vetoed (no shared "is this drop currently legal?"
   state).
5. *Why wasn't this caught before VNC?* — No test drives the real `QDropEvent`/navigator rebuild; the smoke
   asserts `moveScene` on a `SceneDocument` in isolation (Cases A/B), which always re-inserts correctly
   because it's handed correct arguments. **Root cause: the navigator-rebuild-after-drop path is untested with
   real drop-resolved arguments.**

**Fix direction (proposed):** shares I-0067's fix (capture drag source at drag-start; don't splice in-memory
unless the disk reorder succeeded). Additionally, have `dropEvent` reuse the **last `dragMoveEvent`-accepted**
resolution (store it) instead of re-resolving, so a drop can never land where the move gate said no.

---

## I-0069: [Linux] Ctrl+Shift+Return at end-of-scene WITH followers renumbers but creates no visible chapter/scenes

**Status:** 🔴 Open — found during SP-067 VNC verification (2026-07-16, case 3a). The confirmation dialog
fired and, on confirm, the later chapters **renumbered** — but **no new chapter or scenes appeared**. The
followers were not visibly moved into a new chapter.
**Platform:** Linux (`platforms/linux/`)
**Component:** `platforms/linux/src/EditorShell.cpp` (`onCreateChapterRequested`, end-of-scene-with-followers
branch, ~L1126–1136 → reload L1146 → `renumberCreatedChapters` L1151).
**Severity:** Medium (the ⌘⇧↩ split gesture appears to do nothing structural for this case, though it renumbers)
**Sprint:** SP-067
**Epic:** EP-023 `[Linux]`
**Related:** I-0064 (parent), I-0070 (the no-followers branch), I-0067/I-0068 (share the `reorder_scene`
plumbing — a shared reorder defect could explain all four). macOS parity: `ManuscriptTextView` ⌘⇧↩.

**Description:**
With the caret at the **end of a scene that has following scenes in its chapter**, Ctrl+Shift+Return should
insert a new chapter right after the current one and **move the followers into it**. Instead the followers
stayed put and no new chapter/scenes showed; only the (unrelated) renumber visibly happened.

**Expected Behavior:** a new chapter K appears immediately after the current chapter C, C's post-caret scenes
become K's scenes, subsequent chapters renumber, the blank K0 is dropped.

**Actual Behavior:** chapters renumbered but no new chapter/scenes are visible; followers not reassigned in
the UI.

**Root Cause (hypothesis — confirm with a live run):**
The branch at `EditorShell.cpp:1126–1136` does: for each follower `reorderScene(f, chapterC, newChapterID,
afterID)`, then `deleteScene(firstSceneID)` (drop blank K0), then a full `load()` at L1146. If **`reorderScene`
here fails/no-ops** (the same suspected argument defect as I-0067 — e.g. a wrong `chapterC`/`newChapterID`, or
`afterID` handling), the followers never join K; then `deleteScene(K0)` removes K's *only* scene, leaving K
**empty**, and an empty chapter has no segments so the post-`load()` navigator **doesn't render it at all** —
hence "no new chapter/scenes," while `renumberCreatedChapters()` still walked the chapters and renamed
sidecars (the visible renumber). The mid-scene branch (3b) passes because it **saves the tail into K0** (K is
never empty) and doesn't depend on follower reassignment.

**5-Whys:**
1. *Why did no new chapter/scenes appear?* — The new chapter K ended up **empty** (no scenes), and the
   navigator doesn't render an empty chapter.
2. *Why was K empty?* — The followers were not reassigned into K, and its only born scene (blank K0) was then
   deleted.
3. *Why were the followers not reassigned?* — The per-follower `reorderScene` call no-op'd/failed — the same
   `reorder_scene` argument/plumbing fault suspected in I-0067.
4. *Why did K0 get deleted anyway?* — The code unconditionally deletes K0 in the with-followers branch,
   assuming the followers already populated K; it doesn't verify K is non-empty before dropping K0.
5. *Why wasn't this caught before VNC?* — `scene_reorder_smoke` Case D replays these bridge steps **directly
   with correct ids** and passes, so it proves the *orchestration recipe* but not the *app's execution* of it
   (caret→segment resolution, follower collection, argument passing). **Root cause: the smoke tests the recipe,
   not `onCreateChapterRequested`; and the with-followers branch deletes K0 without a "K is non-empty" guard.**

**Fix direction (proposed):** fix the shared `reorder_scene` argument path (I-0067); guard the K0 delete on
"K now has ≥1 scene" (never leave/observe an empty chapter — consistent with the I-0071 no-empty-chapter
policy); add a live-path test that drives `onCreateChapterRequested` end-of-scene-with-followers and asserts
the reopened structure.

---

## I-0070: [Linux] Ctrl+Shift+Return at end-of-scene with NO followers appends the new chapter at the manuscript end

**Status:** 🔴 Open — found during SP-067 VNC verification (2026-07-16, case 3c). Splitting at the end of a
chapter's **last** scene (no followers) **added a new chapter at the very end of the document** instead of
inserting it right after the current chapter. This is the **original I-0064 symptom** surviving for this
branch.
**Platform:** Linux (`platforms/linux/`)
**Component:** `platforms/linux/src/EditorShell.cpp` (`onCreateChapterRequested`, no-followers path — falls
through to the "else" at ~L1137 after `reorderChapter` at L1102).
**Severity:** Medium (a stray end-of-manuscript chapter — the exact wrong structure I-0064 was meant to fix)
**Sprint:** SP-067
**Epic:** EP-023 `[Linux]`
**Related:** I-0064 (parent — this is the residual), I-0069 (sibling end-of-scene branch). macOS parity:
`ManuscriptTextView` ⌘⇧↩ + `insertChapterFirstScene`.

**Description:**
Caret at the **end of the last (or only) scene of a chapter**; Ctrl+Shift+Return creates the new empty chapter
but it lands at the **end of the manuscript**, not immediately after the current chapter.

**Expected Behavior:** a new empty chapter is inserted **directly after the current chapter C**.

**Actual Behavior:** the new chapter is appended at the manuscript end (no repositioning).

**Root Cause (hypothesis — confirm with a live run):**
Step 2, `reorderChapter(projectPath_, newChapterID, chapterC)` at `EditorShell.cpp:1102`, is supposed to move
the freshly-appended K to sit right after C. For the no-followers branch nothing else moves K, so if
`reorderChapter` **no-ops or fails**, K stays where `create_chapter` put it — appended at the end. Candidates:
(a) `scrivi_reorder_chapter`'s `afterChapterID = chapterC` isn't honored when C is the last chapter (moving
after the last chapter = staying last is *correct* only if C really is last — but here C is **not** meant to be
last; check whether `chapterC` is the right id and whether reorder_chapter treats "after the last chapter" as a
no-op); (b) `chapterC` is resolved wrong (stale caret→segment). Note the smoke's Case D calls
`reorderChapter(kCh, ch1)` where ch1 is **not** last and it passes — so the failing condition is specifically
**C being the current last chapter** (the natural no-followers case), which the smoke never exercises.

**5-Whys:**
1. *Why did the new chapter land at the end?* — K was never repositioned after C.
2. *Why wasn't it repositioned?* — `reorderChapter(K, afterChapterID=C)` didn't move K.
3. *Why didn't reorderChapter move it?* — Suspected: when C is the manuscript's **last** chapter, "insert K
   after C" collapses to "K stays last" — but K was appended **after** the also-appended nothing… i.e. the
   after-anchor/ordering for the last-chapter case isn't handled, or `chapterC` was resolved incorrectly.
4. *Why is the last-chapter case unhandled?* — The orchestration assumes C has chapters after it (the general
   split case); the no-followers-at-end case (C is last, K should still be a distinct new last chapter placed
   immediately after C) wasn't distinguished.
5. *Why wasn't it caught before VNC?* — `scene_reorder_smoke` Case D positions K after a **non-last** chapter
   (ch1, with ch2 after it), so `reorder_chapter`'s last-chapter behavior is never tested; and no test drives
   `onCreateChapterRequested` with the caret at the end of the last scene. **Root cause: the end-of-last-chapter
   reposition path is untested, and the app path isn't covered.**

**Fix direction (proposed):** verify `chapterC` resolution; confirm/handle `reorder_chapter`'s
"after the current last chapter" semantics so K is placed as the new chapter immediately following C; add a
live-path + smoke case with C as the last chapter.

---

## I-0071: [Linux] Dragging a chapter's last remaining scene orphans an empty chapter

**Status:** 🔴 Open — identified during SP-067 review (2026-07-16). **Decision (user, 2026-07-16):** the move
is allowed, and the vacated chapter gets a **new empty replacement scene** so it never becomes an empty
chapter. If the writer then wants the chapter gone, she deletes it explicitly.
**Platform:** Linux (`platforms/linux/`)
**Component:** `platforms/linux/src/EditorShell.cpp` (`onSceneDropped`); `SceneDocument::moveScene`
(the vacated-chapter case, `SceneDocument.cpp:443–444` already anticipates "target chapter has no segments
left"); `ScriviBridge` (needs a create-scene call in the vacated chapter).
**Severity:** Medium (data-structure hazard: `scrivi_reorder_scene` leaves the source chapter with zero
scenes; the navigator projects nothing for it, so it silently vanishes from the UI while it may persist on
disk — a chapter the writer can neither see nor manage)
**Sprint:** SP-067
**Epic:** EP-023 `[Linux]`
**Related:** I-0067/I-0068 (the drag path this rides on); I-0069 (same empty-chapter hazard on the split path
— apply the same "never leave an empty chapter" rule). Cross-platform: check whether macOS has the same gap.

**Description:**
Dragging the **only** scene of a chapter into a different chapter reassigns that scene's `chapterID` on disk
but does nothing about the now-empty source chapter — `scrivi_reorder_scene` neither deletes it nor backfills
a scene. The navigator, projecting from segments, renders nothing for the empty chapter, so it disappears from
view while potentially remaining on disk.

**Expected Behavior (user decision 2026-07-16):** the move succeeds; the vacated chapter is immediately given
a **new blank scene** (so it stays a visible, valid, empty-bodied chapter). Deleting the chapter is a separate,
explicit user action.

**Actual Behavior:** the source chapter is left with zero scenes (orphaned/empty); no replacement scene is
created.

**Root Cause:**
`scrivi_reorder_scene` is a pure reassignment primitive with no source-chapter bookkeeping, and the Linux drop
path (`onSceneDropped`) doesn't detect "this was the source chapter's last scene" to backfill. `moveScene`
already tolerates an emptied target chapter (`SceneDocument.cpp:443`) but nothing creates the replacement
scene on disk.

**5-Whys:**
1. *Why can a chapter end up empty?* — A reorder can remove a chapter's last scene with no compensating action.
2. *Why is there no compensating action?* — `scrivi_reorder_scene` only moves the scene; the app drop path
   doesn't check for or handle the "last scene of the source chapter" case.
3. *Why doesn't the app handle it?* — SP-067's scope covered moving scenes, not the empty-chapter side effect;
   the case wasn't enumerated in the AC4 exit criteria.
4. *Why wasn't it enumerated?* — Drag-reorder was specified around within/cross-chapter placement of the moved
   scene, not the state left behind in the source chapter.
5. *Why does an empty chapter matter?* — The navigator projects only from scene segments, so an empty chapter
   is invisible and unmanageable — a chapter the user can't see, select, or delete. **Root cause: reorder has
   no source-chapter-emptied policy, and the UI can't represent an empty chapter.**

**Fix direction (per user decision):** in `onSceneDropped`, after a successful cross-chapter `reorderScene`,
detect that the source chapter now has no scenes and `createScene` a blank scene in it (bridge call), then
re-splice/rebuild. Same "never leave an empty chapter" guard applies to the I-0069 split path. (Alternative
rejected by user: forbid the drag, or auto-delete the chapter.)

---

## I-0066: [ScriviCore]/[Apple] A deleted scene/chapter leaves orphaned history in the log

**Status:** ✅ Resolved - Verified (2026-07-15, user-confirmed on macOS — the previously-crashing project opens;
its history log self-heals after one open; navigator scene/chapter deletes record a `sceneDelete` barrier).
Load-time prune + delete-time barrier landed; ctest green (268/268).
**Platform:** macOS/Apple (shared ScriviCore history + the Apple navigator delete path). Not Linux.
**Component:** `Scrivi/Views/SceneNavigatorView.swift` (`performDeleteScene`/`performDeleteChapter` — the missing
barrier); `ScriviCore/src/history/HistoryService.cpp` (`pruneInconsistentNodes`, `diffMatches`);
`ScriviCore/src/history/HistoryStore.cpp` (`openOrCreate` prune + `ctl:purge` persist).
**Severity:** Medium (root cause of the I-0065 crash; on its own it produces silently-degraded/orphaned undo
history for deleted scenes, and — before I-0065's clamp — the crash).
**Sprint:** — (out-of-band fix during EP-023)
**Epic:** EP-019 `[Apple]` (Custom Undo/Redo History)
**Related:** **I-0065** (the crash this feeds — clamp + C ABI guard was the immediate stop; this is the durable
fix). macOS already records a `sceneMerge`/`sceneSplit` barrier on the *editor* delete/split path
(`ManuscriptTextView.swift`); the **navigator** delete path never did.
**Date Identified:** 2026-07-15 (diagnosing I-0065 — the mismatched diff traced back to a navigator scene delete)
**Date Resolved:** 2026-07-15

**Description:**
When a scene or chapter is deleted **from the Scene Navigator** (`performDeleteScene`/`performDeleteChapter`),
the app called `deleteScene`/`deleteChapter` but recorded **no history barrier** and did **nothing** to that
scene's history in `history/log-000001.jsonl`. The scene's `rec:"floor"` and `rec:"event"` records stayed in the
log. On the next open, replay faithfully reconstructed the deleted scene's history; because the scene (and its
baseline) were gone or changed, a diff no longer matched — an **orphaned/inconsistent diff**. That is exactly the
mismatched node that crashed the app in **I-0065**. (The editor-driven delete/merge/split paths in
`ManuscriptTextView` *do* record barriers; only the navigator delete path was missing one.)

**Expected Behavior:**
Deleting a scene/chapter records a structural **`sceneDelete` barrier** (undo can't cross into the removed
content), and any history the deletion orphaned is cleaned up so a later open never replays a diff against a
missing scene — no crash, no silent head corruption, no perpetual re-degrade.

**Actual Behavior:**
Orphaned floor+event records accumulated in the log with no barrier; on reload they mismatched (crash pre-I-0065;
silently-clamped best-effort head post-I-0065, re-degraded on every open).

**Root Cause:**
Two gaps: (a) the navigator delete path recorded no barrier (unlike the editor paths); (b) `scrivi_delete_scene`
/`scrivi_delete_chapter` have no history-side effect, and the load path had no integrity check — a persisted diff
was trusted to match its scene.

**Fix (2026-07-15):**
1. **Delete-time barrier (Apple)** — `performDeleteScene`/`performDeleteChapter` now call
   `session.historyCapture?.recordBarrier(kind: "sceneDelete", …)` **before** the delete, matching the editor
   paths. Undo stops at the barrier instead of walking into the removed scene.
2. **Load-time self-heal (ScriviCore)** — `HistoryService::pruneInconsistentNodes()` runs in
   `HistoryStore::openOrCreate` after `finalizeLoad()`/eviction. It DFS-walks the tree carrying each scene's
   replayed text and, via the new `diffMatches(oldText, diff)` predicate, drops any node whose diff can't have
   come from its scene (offset past end, or removed bytes absent) **together with its subtree**; the current
   pointer is walked back to a surviving ancestor. It returns the detached subtree roots, and the store persists
   a **`ctl:purge`** per root — so the log is clean on the *next* open (replay honors `ctl:purge`). The bad
   history **self-heals** on first open instead of degrading forever.

Together with I-0065's clamp (never crash) and C ABI guard (never cross the boundary), history is now: never
crashes, never silently mangles a head, and repairs an already-corrupt log on load.

**Verification (2026-07-15 — ctest green; user run pending):**
- New tests (`HistoryServiceTests.cpp`, tag `[History][load][I-0066]`): (a) an inconsistent node is dropped while
  a consistent sibling scene's history stays intact and undoable; (b) the current pointer is moved out of a
  dropped subtree (whole bad chain → one purge root); (c) a fully-consistent tree prunes nothing (no false
  positives). **15 assertions, 3 cases, all pass.**
- Full suite: `./ScriviCoreTests` → **268 cases / 1568 assertions pass**; `ctest` → **268/268**.
- **USER-CONFIRMED (2026-07-15):** on a rebuilt macOS app the previously-crashing project opens; after that open
  its history log no longer carries the orphaned nodes (self-healed via the persisted `ctl:purge`); deleting a
  scene/chapter from the navigator now records a `sceneDelete` barrier (undo stops there).

**Files Affected:**
- `Scrivi/Views/SceneNavigatorView.swift` — `sceneDelete` barrier before navigator scene/chapter delete
- `ScriviCore/src/history/HistoryService.{hpp,cpp}` — `pruneInconsistentNodes()`, `diffMatches()`
- `ScriviCore/src/history/HistoryStore.cpp` — prune-on-load + `ctl:purge` persistence
- `ScriviCore/tests/unit/HistoryServiceTests.cpp` — I-0066 prune tests

---

## I-0065: [ScriviCore]/[Apple] A mismatched history diff crashes the macOS app on project open

**Status:** ✅ Resolved - Verified (2026-07-15, user-confirmed on macOS — "The Twisted Remains of Myself" opens
on a rebuilt app instead of crashing). Fix landed in ScriviCore; ctest reproduces the crash and is green
(268/268 with the I-0066 prune tests).
**Platform:** macOS (any Apple platform — the fault is in shared ScriviCore + its C ABI). Not Linux.
**Component:** `ScriviCore/src/history/HistoryService.cpp` (`applyForward`/`applyReverse`,
`rebuildHeadCache`/`finalizeLoad` replay-on-load); `ScriviCore/src/public_api/scrivi_c_api.cpp`
(`scrivi_history_open`/`_undo`/`_redo` boundary). Swift caller: `Scrivi/App/HistoryCapture.swift`.
**Severity:** High (hard crash — `libc++abi: terminating due to uncaught exception`; the project never opens).
**Sprint:** — (out-of-band fix during EP-023; not a Linux sprint)
**Epic:** EP-019 `[Apple]` (Custom Undo/Redo History) — the history subsystem that shipped the replay-on-load path.
**Related:** **I-0066** (the durable root-cause fix — a navigator scene delete left the orphaned diff that
mismatched here; that Issue adds the delete-time barrier + load-time prune). EP-019 SP-054 (HistoryStore
load/replay); `HistoryCapture.swift` best-effort `open()` try/catch (which could not catch this because the
exception `std::terminate`d *inside* the C ABI, before returning).
**Date Identified:** 2026-07-15 (user — macOS app crash opening a real project)
**Date Resolved:** 2026-07-15

**Description:**
Opening the project **"The Twisted Remains of Myself"** on the macOS app crashed **before the project opened**,
with:
```
libc++abi: terminating due to uncaught exception of type std::length_error: basic_string
```
The crash fired during history **replay-on-load**: `scrivi_history_open` → `HistoryStore` replays the persisted
JSONL log → `HistoryService::finalizeLoad()` → `rebuildHeadCache()`, which walks the root→current path and
applies each node's diff forward with `applyForward`. When a persisted node's diff no longer matches the scene's
baseline text — the shape produced when **a diff is replayed against a deleted or externally-changed scene**
(the user's hypothesis: "applying a historical diff to the wrong Scene") — `applyForward` computed
`reserve(oldText.size() - removed.size() + inserted.size())`. With `removed.size() > oldText.size()` that
subtraction **underflowed** (both are `std::size_t`, unsigned) to a near-`SIZE_MAX` value, and
`std::string::reserve` threw `std::length_error`. A sibling `append(oldText, offset+removed, npos)` could throw
`std::out_of_range` on the same mismatch.

Because that exception was thrown **inside** an unguarded C ABI function (`scrivi_history_open`), it crossed the
`extern "C"` boundary — which is a hard `std::terminate` (a C++ exception may never unwind through a C frame).
The Swift side (`HistoryCapture.open()`) already wraps `engine.historyOpen` in a best-effort try/catch ("a
failure here must never block editing"), but it never got the chance: the process was already dead.

**Expected Behavior:**
A corrupt, stale, or mismatched history log degrades **history** to best-effort (no undo, or a truncated tree)
and the **project still opens and is editable**. No crash, ever, from replaying a bad diff.

**Actual Behavior:**
The whole app terminated at project-open before the editor appeared.

**Root Cause:**
Two independent latent faults, both required for the crash:
1. **`applyForward`/`applyReverse` assumed the diff matched `oldText`.** The `reserve()` size arithmetic
   underflowed on `removed.size() > oldText.size()`, throwing `std::length_error`; the tail `append` could throw
   `std::out_of_range`. A mismatched/stale/corrupt persisted diff triggers both.
2. **The C ABI history entrypoints were unguarded.** Any C++ exception escaping `scrivi_history_open` (or
   `_undo`/`_redo`) unwound into a C frame → `std::terminate`. The boundary contract (`scrivi.h` is a pure C ABI;
   exceptions must never cross it) was violated by omission for these three functions.

**Fix (2026-07-15):**
- **Clamp-safe apply (defense in depth #1)** — `applyForward`/`applyReverse` now clamp the offset and the
  removed/inserted span into the actual text:
  `off = min(offsetUtf8, text.size())`, `cut = min(off + span.size(), text.size())`, and build the result from
  `[0,off) + patch + [cut, end)`. No unsigned underflow, no out-of-range; a mismatched diff yields a best-effort
  string instead of throwing. (`HistoryService.cpp` ~L96–119.)
- **Guarded C ABI (defense in depth #2)** — a `guarded(fn)` helper wraps the history entrypoints in
  `try { … } catch (const std::exception& e) { → errorEnvelope(internalError, "unhandled exception: …") } catch (...) { … }`,
  so **no** exception can ever cross the boundary again; a failure returns a `{"error":…}` envelope Swift already
  handles. Applied to `scrivi_history_open`, `scrivi_history_undo`, `scrivi_history_redo`.
  (`scrivi_c_api.cpp` ~L175, L1498/L1645/L1680.)

Both layers are deliberate: the clamp fixes *this* bug at the source; the guard ensures *any* future history-path
exception becomes an error envelope (best-effort history) rather than a process kill — matching what
`HistoryCapture.open()` was already written to expect.

**Verification (2026-07-15 — ctest green; user run pending):**
- New regression **`replay-on-load survives a diff whose removed span exceeds the floor`**
  (`ScriviCore/tests/unit/HistoryServiceTests.cpp`, tag `[History][load][I-0065]`) rehydrates a service via
  `addLoadedFloor`/`addLoadedNode`/`setPointers` with a persisted node whose `diff.removed` (20 bytes) exceeds
  the scene's floor ("hi"), then asserts `finalizeLoad()` **does not throw**, yields a well-formed best-effort
  head, and that `undo()`/`redo()` on the mismatched node also don't throw. **Before the fix this reproduced the
  `std::length_error`.**
- `ctest --test-dir build --output-on-failure` → **268/268 pass** (the I-0065 replay test + 3 I-0066 prune tests).
- **USER-CONFIRMED (2026-07-15):** on a rebuilt macOS app, **"The Twisted Remains of Myself"** opens (history for
  the affected scene truncated/best-effort) instead of crashing.

**Files Affected:**
- `ScriviCore/src/history/HistoryService.cpp` — clamp-safe `applyForward`/`applyReverse`
- `ScriviCore/src/public_api/scrivi_c_api.cpp` — `guarded()` helper + wrapped history entrypoints
- `ScriviCore/tests/unit/HistoryServiceTests.cpp` — I-0065 regression test

---

## I-0064: [Linux] Ctrl+Shift+Return appends a chapter at the end instead of splitting at the caret

**Status:** ⚠️ **Partially Resolved — VNC 2026-07-16.** The **mid-scene split** path works (VNC case 3b:
caret mid-sentence → head stays, tail becomes the new chapter's first scene, new chapter inserted after
the current one). The **end-of-scene** paths do **not**: end-of-scene *with followers* renumbers but shows
no new chapter/scenes (**I-0069**), and end-of-scene *with no followers* still appends the new chapter at
the manuscript end (**I-0070** — the exact original I-0064 symptom, surviving for that branch). So the fix
landed for the mid-scene case only; the two end-of-scene branches are split out as I-0069/I-0070 and I-0064
stays open until those close. (Prior "Resolved - Not Verified" note below retained for history.)

**Prior status (2026-07-15, superseded by the VNC result above):** ✅ Resolved - Not Verified (SP-067 / T-0261). `onCreateChapterRequested` now SPLITS
the current chapter at the caret instead of appending an empty chapter at the manuscript end. Disk-correct
orchestration (unlike macOS, which splices only in-memory): `create_chapter` (append K + blank K0) →
`reorder_chapter(K, afterChapterID = C)` → for each scene that followed the caret's scene S within C,
`reorder_scene(scene, C, K, afterSceneID)` in order → **mid-scene:** `save_scene` head into S, tail into K0;
**end-of-scene with followers:** followers become K's scenes and the redundant blank K0 is dropped;
**end-of-scene, no followers:** K0 stays as a genuinely-new empty chapter after C → full reload (the split
touches multiple scenes + chapters, so re-reading disk is the safest source-of-truth path) →
`renumberCreatedChapters()` (I-0063) → caret lands at the start of the caret scene (K0/tail for mid-scene,
first follower for end-of-scene). A confirmation dialog fires first when ≥1 subsequent chapter will renumber
(macOS `ManuscriptTextView` parity). Verified headless by `scene_reorder_smoke` Cases D (end-of-scene) + E
(mid-scene head/tail); **VNC user-verification pending.**
**Platform:** Linux (`platforms/linux/`)
**Component:** `platforms/linux/src/EditorShell.cpp` (`onCreateChapterRequested`) →
`SceneDocument::insertSceneAfter`. The Linux create-chapter path has appended-at-end since EP-022/SP-062.
**Severity:** Medium (the ⌘⇧↩ chapter gesture does not do what a writer expects; data is not corrupted, but
the manuscript structure produced is wrong — a stray end-of-manuscript chapter instead of a split).
**Sprint:** discovered SP-066; **fix targeted SP-067**
**Epic:** EP-023 `[Linux]`
**Related:** I-0063 (chapter renumbering); SP-067/068 (drag-reorder — same reorder primitives);
macOS spec: `ManuscriptTextView.swift` (⌘⇧↩ handler, ~L690–795) + `ViewportSceneLoader`
(`splitScene`, `splitChapter`, `insertChapterFirstScene`, `renumberChapterTitlesFrom`).

**Description:**
Positioning the caret inside/at the end of a scene and pressing **Ctrl+Shift+Return** should **split the
current chapter at the caret** (macOS parity), not append a new empty chapter at the end of the manuscript.

Reproduced (user, 2026-07-15): three chapters with multiple scenes; Chapter 2 has three scenes; caret at the
**end of Scene 2 of Chapter 2**. Expected: Chapter 2 splits — Scene 3 (and any following scenes of Chapter 2)
becomes the first scene(s) of a **new Chapter 3** inserted right there, and the old Chapters 3+ renumber down
the line. Actual: a brand-new empty **Chapter 4** with a single blank scene was **appended to the end** of the
document; no split, no scene reassignment, no renumber.

**Expected Behavior (macOS parity — `ManuscriptTextView` ⌘⇧↩):**
- **Caret at end of a scene:** insert the new chapter **immediately after the current scene**; the scenes that
  followed the caret's scene **within the current chapter** are reassigned into the new chapter; subsequent
  chapters renumber. (macOS `insertChapterFirstScene(result, after: segIdx)` + `renumberChapterTitlesFrom`.)
- **Caret mid-scene:** **split the scene** at the caret — the head stays in the current scene/chapter; the tail
  becomes the **first scene of the new chapter**; following scenes of the old chapter move into the new
  chapter; renumber. (macOS `splitScene` + `splitChapter` + `renumberChapterTitlesFrom`.)
- A confirmation is shown when the split will renumber ≥1 subsequent chapter.

**Actual Behavior:**
`onCreateChapterRequested` calls `scrivi_create_chapter` (which is **append-only** — `ChapterCreator.cpp:104`
`ms.chapters.push_back`, always a fresh chapter + blank scene at the manuscript end) and then
`SceneDocument::insertSceneAfter(lastIdx, …)`, so the chapter always lands at the end with no positional
insert, no scene reassignment, and no renumber. The caret position is ignored entirely.

**Root Cause:**
`scrivi_create_chapter` is an append-only primitive with no position argument and no scene-move capability.
macOS builds the split behavior **on top of** it by orchestrating additional calls — `create_chapter` (append),
`save_scene` (head/tail split), `reorder_chapter` (move the new chapter into position), `reorder_scene`
(reassign the following scenes into the new chapter), then in-memory renumber. The Linux path never added that
orchestration; it stops at the raw append.

**Fix direction (SP-067 — no ScriviCore change needed):**
The reorder endpoints already exist (`scrivi_reorder_scene(sceneID, sourceChapterID, targetChapterID,
afterSceneID)`, `scrivi_reorder_chapter(chapterID, afterChapterID)` — `scrivi.h:199/206`). Orchestrate:
1. `create_chapter` (appends new chapter C with blank first scene S).
2. `reorder_chapter(C, afterChapterID = current chapter)` to move C into position.
3. For each scene after the caret's scene in the current chapter: `reorder_scene` into C (in order).
4. Mid-scene case: `save_scene` head into the current scene, tail into S (or the first reassigned scene).
5. Renumber untitled/auto chapters (ties into I-0063) + rebuild the `SceneDocument` splice + navigator.
6. Confirmation dialog when subsequent chapters will renumber.

Do this in SP-067 alongside drag-reorder, which delivers/【exercises the same `reorder_*` orchestration and
the `SceneDocument` move-splice peers.

**Workaround (until fixed):** create chapters at the end and reorder later (once drag-reorder lands), or keep
scenes you want split as separate scenes.

---

## I-0063: [Linux] Deleting/inserting a chapter doesn't renumber later created chapters

**Status:** ✅ Resolved - **Verified (2026-07-16, user-confirmed over VNC** — deleting an earlier chapter
renumbered the later created "Chapter N" chapters to their correct ordinals, and a custom-titled chapter was
left untouched; case 4 of the SP-067 VNC walkthrough). Implemented per **Option A** below. New
`EditorShell::renumberCreatedChapters()` walks chapters in manuscript order and, for each whose STORED title
matches the anchored auto pattern `^Chapter \d+$`, calls `bridge_->renameChapter(chapterMetadataPath, "Chapter
<ordinal>")` to rewrite the sidecar to its new position; custom titles (which don't match the anchored
pattern) are untouched, and untitled chapters already renumber for free via `chapterHeadingText`. Wired into
`deleteChapterByID` (after `removeChapter`) and the T-0261 chapter-split path (after the reload), each followed
by `applyDerivedLabels()` + `rebuildNavigator()` so the live navigator/heading match disk. Also fixed a latent
gap: `EditorShell::load` now copies `chapterMetadataPath` from `open_project`'s scene entries into the segment
map (the renumber + chapter-rename on a freshly-loaded chapter both need it). Verified headless by
`scene_reorder_smoke` Case D's I-0063 assertions (created "Chapter N" chapters carry their correct ordinal on
reopen after a split); **VNC user-verification pending.**
**Platform:** Linux (`platforms/linux/`)
**Component:** `platforms/linux/src/EditorShell.cpp` (`deleteChapterByID` and any future chapter-insert path);
`SceneDocument::chapterHeadingText` / `reflowAllChapterHeadings` (the renumber machinery already exists).
**Severity:** Low (display-only ordinal drift; scene/chapter data and order are correct on disk)
**Sprint:** — (backlog; discovered during SP-066)
**Epic:** EP-023 `[Linux]`
**Related:** SP-065 (delete), SP-066 (rename — added `chapterHeadingText` ordinal derivation + the unused
`reflowAllChapterHeadings` helper this fix would call); macOS parity: `ViewportSceneLoader.renumberChapterTitlesFrom`.

**Description:**
When a chapter is created, **ScriviCore stamps a stored title `"Chapter N"`** into the chapter sidecar
(`ChapterCreator.cpp:90`), and `scrivi_delete_chapter` does **not** renumber the remaining chapters. So after
deleting an earlier chapter (or inserting one between), later **created** chapters keep their now-stale stored
ordinal — e.g. delete "Chapter 1" and the old "Chapter 3" still reads "Chapter 3" instead of "Chapter 2".

Untitled chapters (empty stored title) are unaffected — the Linux app derives their heading from order via
`SceneDocument::chapterHeadingText` (added in SP-066), so those renumber for free. The gap is specifically
chapters carrying a stored `"Chapter N"` string.

**Expected Behavior (macOS parity):**
After any chapter structural change (delete/insert/reorder), every subsequent chapter that is **not** custom-
titled shows its correct ordinal — matching macOS `ViewportSceneLoader.renumberChapterTitlesFrom`, which
rewrites the in-memory `chapterTitle` to `"Chapter N"` for affected chapters (and the engine persists the
ordinals to disk).

**Root Cause:**
Two facts combine: (a) ScriviCore stores `"Chapter N"` as a real title at creation (not empty), so the app's
order-based derivation treats it as a custom title and won't recompute it; (b) `ChapterDeleter` doesn't
renumber survivors on disk. So neither layer renumbers created chapters after a delete.

**Options (not yet chosen):**
- **A.** App-side active renumber (macOS parity): after a chapter delete/insert, walk later chapters and, for
  each whose stored title matches the auto pattern `"Chapter <n>"`, `renameChapter` it to its new ordinal
  (rewrites disk). Uses the existing `reflowAllChapterHeadings` for the live document. Fragile edge: a user
  who deliberately typed "Chapter 5" would be renumbered.
- **B.** ScriviCore stores chapters **untitled** (empty title) and derives `"Chapter N"` only for display —
  then the app's order-based derivation handles everything with no disk rewrite. Cleanest, but a `[ScriviCore]`
  behavior change affecting Apple too (needs a cross-platform decision).
- **C.** Accept the drift for created chapters until a dedicated structure-editing pass (reorder, SP-067/068),
  where renumbering is revisited holistically.

**Deferred:** display-only; created-chapter ordinals self-correct if the user renames them, and untitled
chapters already renumber. Revisit alongside reorder (SP-067/068) or when the cross-platform numbering
policy (Option B) is decided.

---

## I-0062: [Linux] New chapter heading shows "Chapter" (not "Chapter N") until reload

**Status:** ✅ Resolved - Verified (2026-07-15, user-confirmed over VNC — a newly-created chapter's heading shows
its ordinal "Chapter N" immediately, no reload). **Resolution (SP-066 / T-0256):** the Linux app now **derives**
the chapter heading ordinal from segment order (`SceneDocument::chapterHeadingText` — custom title wins, else
"Chapter N" by position), matching macOS `ManuscriptTextView`. `insertSceneAfter` reflows the new (untitled)
chapter's heading to its derived ordinal on splice, so the live heading is correct without a round-trip. (This
supersedes the earlier "Option B openProject re-fetch" plan — order-based derivation is cleaner and is the
macOS mechanism.) Verified by `scene_create_smoke` (asserts the created chapter derives "Chapter 2" live) +
VNC. Note: this covers **untitled** chapters; renumbering **created** (stored-"Chapter N") chapters on a later
delete is the separate I-0063.
**Platform:** Linux (`platforms/linux/`)
**Component:** `platforms/linux/src/EditorShell.cpp` (`onCreateChapterRequested`) →
`SceneDocument::insertSceneAfter` (chapter-heading text); the correct label comes from
`scrivi_open_project`'s `chapterTitle` on reload.
**Severity:** Low (purely a live-vs-reloaded label mismatch; the chapter is created correctly, persists
correctly, and renders its real title on the next open)
**Sprint:** SP-062 (observed during T-0242 verification); **fix scheduled SP-066 / T-0256**
**Epic:** EP-022 `[Linux]`
**Related:** T-0241 (in-editor create chapter)
**Date Identified:** 2026-07-14 (user VNC verification)

**Description:**
When a chapter is created in-editor (`Ctrl+Shift+Return`), its heading in the live document reads the
generic **"Chapter"** rather than **"Chapter 2"** (etc.). After quitting and reopening the project, the
heading renders correctly as "Chapter 2".

**Root Cause:** `scrivi_create_chapter` returns `{chapterID, chapterMetadataPath, firstSceneID,
firstSceneMetadataPath, firstSceneContentPath}` — **no display title**. So `insertSceneAfter` falls back to
the `"Chapter"` label for the freshly-inserted heading. On reopen, `scrivi_open_project` returns the real
`chapterTitle` (the ordinal-derived "Chapter 2"), so it displays correctly.

**Expected Behavior:** The new chapter's heading shows its real ordinal title immediately, matching what a
reload shows.

**Options (not yet chosen):**
- **A.** Have `scrivi_create_chapter` return the derived `chapterTitle` (a `[ScriviCore]` additive change),
  and use it in `insertSceneAfter`. Keeps label derivation in the backend (preferred — no UI-side ordinal
  logic). Additive to `scrivi.h`.
- **B.** Re-fetch just the new chapter's title via a light `open_project` read after create and patch the
  heading. Avoids a core change but adds a round-trip.
- **C.** Accept the fallback until EP-023 (chapter rename/structure editing), where headings become
  first-class and titled explicitly.

**Deferred:** cosmetic and self-correcting on reload; chapter *naming/structure editing* is **EP-023**
scope, not EP-022. Revisit when EP-023 is planned (or promote to a Task if the live label matters sooner).

---

## I-0061: [Linux] Landing Quit button does nothing after the SP-061 shell flip

**Status:** ✅ Resolved - Verified (2026-07-14 — user confirmed the Quit button now quits over VNC, and the quit-path save fired; headless `quit_smoke` guards the regression in CI)
**Platform:** Linux (`platforms/linux/`)
**Component:** `platforms/linux/src/main.cpp` (app bootstrap); `qml/Landing.qml` Quit button (`Qt.quit()`)
**Severity:** Medium (shipped UI action is dead; the app can't be quit from the landing — but the window-X still works, and edits aren't lost)
**Sprint:** SP-062 (regression introduced by SP-061)
**Epic:** EP-022 `[Linux]`
**Related:** SP-061 shell flip (T-0234); SP-062 T-0239 (the quit-path auto-save that depended on this quit working)
**Date Identified:** 2026-07-14 (user VNC verification of SP-062)
**Date Resolved:** 2026-07-14

**Description:**
The SP-061 shell flip moved the landing QML from a top-level `QQmlApplicationEngine` into a
`QQuickWidget`. `QQmlApplicationEngine` auto-connects `QQmlEngine::quit()` (the signal QML's `Qt.quit()`
emits) to `QCoreApplication::quit()`; a `QQuickWidget`'s engine does **not**. So the landing's **Quit**
button emitted into the void.

**Expected Behavior:** Clicking **Quit** on the landing exits the app (and, in the Docker/VNC harness,
tears the container down since the app is the foreground process).

**Actual Behavior:** Nothing happened. Console: `Signal QQmlEngine::quit() emitted, but no receivers
connected to handle it.`

**Root Cause:** Missing signal→slot wiring after the integration direction inverted (QML now lives inside
Widgets). No auto-connect happens for a `QQuickWidget` engine.

**Fix:** `main.cpp` now connects the landing `QQuickWidget`'s engine explicitly —
`QQmlEngine::quit → QApplication::quit` (plus the `exit(int)` variant). This also restores the T-0239
quit-path auto-save chain (Quit → `QApplication::quit` → `aboutToQuit` → `ScriviWindow::flushEditor`).

**Verification:** New headless **`quit_smoke`** wires the connection the way `main.cpp` does, emits
`QQmlEngine::quit()`, and asserts the app reaches `exit(0)` (fail-safe timer fails loudly if unconnected).
Green in Docker + wired into CI. **Awaiting user VNC confirmation that the Quit button now quits.**

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
