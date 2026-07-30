# SP-087: [ScriviCore] EP-029 — paste-splice ✅ CLOSED

**Status:** ✅ **Closed 2026-07-27 (Human-approved).**
**Epic:** EP-029 `[Cross]` — Cross-Boundary Structured Cut/Copy/Paste (3rd of 5 sprints; 2nd implementation).
**Start / Close:** 2026-07-27 / 2026-07-27.

## Goal (met)

ScriviCore **paste-splice** — insert a `scrivi.fragment.v1` at a caret, reconstructing every carried
scene/chapter boundary as if inserting into one continuous flat document (design §4.2). The inverse of
extract-fragment (SP-086).

## Delivered (T-0352 — ✅ Verified)

- **`ScriviCore/src/manuscript/FragmentPaster.{hpp,cpp}`** — self-contained primitive (same pattern as
  `FragmentExtractor`). Splits the target scene at the caret; appends the head piece; per subsequent piece,
  `createScene(afterSceneID)` (scene-piece, same chapter) or `createChapter(afterChapterID)` + writes text into
  the new chapter's first scene (chapter-piece); the target's tail-suffix follows the whole pasted run onto the
  last created scene. Composes the standalone `SceneCreator` / `ChapterCreator` (EP-027 order-key placement) +
  `SceneWriter` + `ManuscriptOrderResolver` + `SceneReader`. Reports `{targetSceneID, createdSceneIDs,
  createdChapterIDs}` for history/undo. No new folder/order-key logic.
- **`scrivi_fragment_paste(...)`** C ABI endpoint + a `parseFragment` helper (reads a `scrivi.fragment.v1` back
  into the struct). Documented in `scrivi.h` (additive, pure C ABI).
- **`ScriviCore/tests/integration/FragmentPasteTests.cpp`** — 8 cases / 66 assertions.

## Two behaviours ruled during implementation (user, 2026-07-27) — folded into design §4.2

1. **Tail placement (flat-document).** A mid-scene multi-piece paste sends the target's tail-suffix to the END of
   the pasted run (onto the last created scene, after the trailing piece), NOT back into the original scene —
   which would fracture reading order.
2. **In-scene joins are direct concatenation, not a blank-line seam.** head+first-piece and last-piece+tail
   continue the paragraph at the caret (flat insert, no `\n\n`). The seam `SceneMerger` uses is never used in
   paste (each cross-boundary piece becomes its own scene). **Caught by a failing test** — the first
   implementation reused `joinBodies`; fixed.

## Verification

- **`ctest` 346/346** (was 338; +8 — single-piece plain insert, scene-piece split + tail continuity,
  chapter-piece creates a chapter, caret-at-start, caret-at-end, created-IDs reported, + 3 rejection cases). No
  regression.
- **macOS app `xcodebuild build` SUCCEEDED** against the grown header.
- `scrivi.h` additive-only; **no pbxproj**.

## Files

- New: `src/manuscript/FragmentPaster.{hpp,cpp}`, `tests/integration/FragmentPasteTests.cpp`.
- Edited: `src/public_api/scrivi_c_api.cpp` (endpoint + `parseFragment`), `include/scrivi/scrivi.h`,
  `ScriviCore/CMakeLists.txt`, `ScriviCore/tests/CMakeLists.txt`, `docs/Scrivi_Structured_CutCopyPaste_Design_v0_1.md`.

## Retrospective

- **Went well:** paste-splice composed cleanly onto the EP-027 create primitives (they are standalone
  `CoreServices`-constructed classes, like the extractor), so no facade plumbing. Two genuine behaviour
  ambiguities (tail placement, in-scene join) surfaced early — one via a failing test — and were ruled + doc'd.
- **Next:** SP-088 — **cut-with-merge** (`scrivi_fragment_cut`): extract + delete the span + merge the spanned
  scenes/chapters into one, atomically (composing `SceneMerger`/`ChapterMerger`); plus the `structuredCut` /
  `structuredPaste` history shape (T3=A).

## Notes

Next available Sprint: **SP-088**. Next available Task: **T-0353**.
