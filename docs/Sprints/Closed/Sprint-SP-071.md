# SP-071: [ScriviCore]+[Linux] Create-chapter-in-place — fix split lost-tail/folder-scramble (I-0074)

**Status:** ✅ **VERIFIED — ready to close (Human-approved 2026-07-18).** Surfaced during the P4 VNC walkthrough on
"The Sentinel of Cenuri": a single mid-scene Ctrl+Shift+Return **lost the split tail** and left a stray folder.
Two coupled defects (below): (1) create-then-reorder used a stale pre-rename path; (2) open-time self-heal stole a
same-named scene. Both fixed. **Verified over VNC 2026-07-18** — all four split cases update correctly and reopen
cleanly; I-0074, I-0075 Verified. ctest **302/302 macOS**.
**Activated:** 2026-07-17
**Epic:** EP-027 (`[ScriviCore]` core change) + EP-023 (`[Linux]` split orchestration). **`scrivi.h` CHANGED**
(additive: `scrivi_create_chapter` gained a trailing `afterChapterID` param).

## Design

- **`createChapter` is now position-aware.** `CreateChapterRequest.afterChapterID` (empty = append) makes
  `ChapterCreator` pick the order key with `keyBetween(after, after's successor)` instead of always
  `keyAfter(last)`. The folder is **born with its final name** (`chapter-c`) — no reorder, no rename, so the
  returned K0 paths stay valid. This mirrors `ChapterReorderer`'s neighbour-window logic (the two can't diverge).
- **Threaded through all layers:** `Requests.hpp` → `ChapterCreator.cpp` → `scrivi.h` + `scrivi_c_api.cpp`
  (additive C ABI param, `""`/NULL = append) → `ScriviBridge::createChapter(..., afterChapterID = {})`.
- **App split (`EditorShell::onCreateChapterRequested`) rewritten:** one `createChapter(after = C)` call replaces
  create-then-reorder; deleted the reorder step **and** the stale-path re-resolve workaround. Human review fixes
  folded in: removed the obsolete "renumber N subsequent chapters" dialog (chapters don't renumber on disk under
  order keys — C1); hoisted the follower-move loop out of the `if` and **stopped deleting K0** at end-of-scene (K0
  is the blank scene the writer types into — C3, an original error condition); the create-failure path now shows a
  warning instead of returning silently (C4).

## Tasks

| ID | Title | Status |
| -- | ----- | ------ |
| T-0279 | **`CreateChapterRequest.afterChapterID`** (empty = append). | ✅ Done (not verified). |
| T-0280 | **`ChapterCreator` create-in-place** — `(lo,hi)` window + `keyBetween`; guard empty-key error. | ✅ Done (not verified). |
| T-0281 | **C ABI + facade** — `scrivi_create_chapter` trailing `afterChapterID` (additive; `scrivi.h` updated); facade passthrough. | ✅ Done (not verified). |
| T-0282 | **`ScriviBridge::createChapter(..., afterChapterID = {})`.** | ✅ Done (not verified). |
| T-0283 | **`EditorShell` split rewrite** — one create-in-place call; drop reorder + stale re-resolve; C1/C3/C4 fixes; remove `[split]` debug logs. | ✅ Done (not verified) — **needs Linux VNC verify.** |
| T-0284 | **Tests** — create-in-place unit test (create after Ch2 → `chapter-c`, **never** `chapter-w`; K0 path valid); split repro updated to the one-call form + no-`chapter-w` assertion. | ✅ Done (not verified). |
| T-0285 | **2nd defect (found in VNC verify): same-named scene stolen on reopen.** After a follower reordered OUT of a chapter, `rebuildChapterScenesIfInconsistent` **preserved the stale ref**; `migrateScenes` orphan-repair then **matched by filename** and stole an identically-named scene (`k-scene`) from another chapter (order-key filenames repeat per chapter). **Fix:** cache is a **pure disk mirror** (drop stale refs — removed the "preserve missing refs" rule); **removed filename-based orphan relocation** from `migrateScenes` (ownership = physical folder; project always opens self-consistent); `migrateScenes` legacy-reslug pass now **disk-driven** (not cache-driven). Reconciled 2 dependent tests: delete-meta case now reports `missingContent` (still repairRequired); C6 test asserts "opens cleanly + self-consistent" not filename relocation. Deterministic `[I-0074]` Centauri regression (4 chapters w/ repeating `001/V/k`, split Ch2, open → Ch1 keeps all scenes, no repairRequired). Full suite **302/302** macOS. | ✅ Done (not verified) — **needs Linux VNC verify.** |
| T-0286 | **App hardening:** `EditorShell` split now **checks every `saveScene`/`reorderScene` return and aborts+warns on the first failure** (was silently continuing into a half-applied state — part of why the corruption was invisible). `[split]` diagnostic logging retained for the next VNC round (strip after). | ✅ Done (not verified). |

## Exit criteria

- `createChapter(afterChapterID = C)` creates the folder with the between-neighbours key directly; no intermediate
  folder, no rename; returned K0 paths resolve. ✅ met (ctest).
- The split's tail is preserved (writes to K0's valid path) and K0 is never deleted. ✅ met at the core/repro level;
  **app behavior pending Linux VNC re-run** (the original I-0074 symptom).
- `scrivi.h` change is additive (existing 6-arg call sites are the only callers — the Linux bridge, updated here).
  Full suite 301/301 macOS.

| T-0287 | **Remove the split confirmation dialog** — Ctrl+Shift+Return IS the approval. A modal "Split into New Chapter?" prompt on every break destroyed drafting flow (frequent, flow-critical move); the split is non-destructive, so a confirm bought little. `failSplit` warnings (real errors) stay. | ✅ Done — **Verified (2026-07-18, VNC).** |
| T-0288 | **I-0075 — arrows cross scene/chapter boundaries.** `normalizeCaret` snapped to the *nearest* editable edge (tie → previous), so Down at a scene end / Up at a chapter start snapped the caret back → stuck. Added `SceneDocument::editablePositionInDirection(pos, forward)` (forward→next body start, backward→prev body end); `normalizeCaret` picks direction from caret travel. `nearestEditablePosition` kept for clicks/paste. Smoke-test assertions added. | ✅ Done — **Verified (2026-07-18, VNC).** |
| T-0289 | **Strip `[split]` diagnostic logging** — removed the verbose segment-dump + per-call `qInfo` (their job is done). Kept the `failSplit` abort + a single real `qWarning` on an actual failed split step. | ✅ Done. |

## Verify log

- **VNC round 1 (2026-07-18):** create-in-place confirmed working on disk (folder born `chapter-c`, no
  `chapter-w`; head/tail split byte-perfect). But **reopen failed** ("needs repair, missing metadata"): a
  same-named scene (`k-scene`) from Ch1 had been stolen into Ch2. Console proved the app orchestration was
  **correct** — the corruption was in `openProject → migrateScenes` (the 2nd defect). Fixed in T-0285.
- **VNC round 2 (2026-07-18):** ✅ **VERIFIED.** All four split cases (mid-scene, mid-scene-with-followers,
  end-of-scene-with-followers, end-of-last-scene-in-chapter) update correctly in the UI and **reopen cleanly with
  no warnings/errors**. Split confirmation dialog removed per the writer's flow requirement (T-0287). **I-0074
  Verified.**

