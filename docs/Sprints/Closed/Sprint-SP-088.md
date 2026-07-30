# SP-088: [ScriviCore] EP-029 — cut-with-merge ✅ CLOSED

**Status:** ✅ **Closed 2026-07-27 (Human-approved).**
**Epic:** EP-029 `[Cross]` — Cross-Boundary Structured Cut/Copy/Paste (4th of 5 sprints; 3rd ScriviCore op).
**Start / Close:** 2026-07-27 / 2026-07-27.

## Goal (met)

ScriviCore **cut-with-merge** — extract the fragment, delete the spanned text, and collapse the spanned
scenes/chapters into one continuous scene, as ⌘X behaves in a flat document (design §4.3). Returns the reversible
data a `structuredCut` undo needs.

## Delivered (T-0353 — ✅ Verified)

- **`ScriviCore/src/manuscript/FragmentCutter.{hpp,cpp}`** — extracts the fragment (reuses `FragmentExtractor`),
  then **delete-and-fold**: folds the head scene = `headPrefix + tailSuffix`, deletes every other span scene via
  `SceneDeleter`, removes any emptied chapter via `ChapterDeleter`. Returns `{fragment, survivingSceneID,
  removedSceneIDs, removedChapterIDs}`. Head fold happens BEFORE any deletion (no lost text on mid-sequence
  failure).
- **`scrivi_fragment_cut(projectRootPath, spansJson)`** C ABI endpoint. Refactored shared `parseSpans` /
  `serializeFragment` helpers (extract + cut share them). Documented in `scrivi.h` (additive, pure C ABI).
- **`ScriviCore/tests/integration/FragmentCutTests.cpp`** — 6 cases / 93 assertions.

## Two behaviours ruled during implementation (user, 2026-07-27) — folded into design §4.3

1. **Delete-and-fold, not "compose SceneMerger/ChapterMerger."** The merge primitives don't fit a general span
   (`SceneMerger` same-chapter only; `ChapterMerger` merges a *whole* chapter → would pull in post-span scenes;
   no single-scene-cross-chapter merge). Delete-and-fold is uniform across same/cross-chapter spans and leaves
   post-span scenes untouched. Composes existing `SceneDeleter`/`ChapterDeleter` + a direct content write.
2. **Cut joins with direct concatenation, no blank-line seam.** A cut just deletes the selected characters and
   closes the gap (like deleting a selection in any editor). **Caught by a failing test** (first draft reused
   `SceneMerger`'s seam).

## History note (T3=A) — ScriviCore side complete

Per design §5, the `structuredCut`/`structuredPaste` history *events* are recorded **app-side (Swift)** via the
existing `scrivi_history_record_event` path — SP-089. SP-088's ScriviCore job is to return the reversible data,
which it does (cut → fragment + removed IDs; paste → created IDs). No ScriviCore history-engine change needed.

## Verification

- **`ctest` 352/352** (was 346; +6 — same-chapter collapse, cross-chapter + post-span scene preserved,
  whole-chapter emptied → removed, single-scene in-place delete, gap-closed joins, 2 rejection cases). No regression.
- **macOS app `xcodebuild build` SUCCEEDED**.
- `scrivi.h` additive-only; **no pbxproj**.

## Files

- New: `src/manuscript/FragmentCutter.{hpp,cpp}`, `tests/integration/FragmentCutTests.cpp`.
- Edited: `src/public_api/scrivi_c_api.cpp` (endpoint + shared helpers), `include/scrivi/scrivi.h`, both
  `CMakeLists.txt`, `docs/Scrivi_Structured_CutCopyPaste_Design_v0_1.md`.

## Retrospective

- **Went well:** the three ScriviCore ops (extract/paste/cut) now compose cleanly on existing primitives; cut
  reuses extract outright. Two genuine ambiguities surfaced + ruled + doc'd (one via a failing test).
- **Next:** SP-089 — `[Apple]` editor wiring: route ⌘C/⌘X/⌘V + ⌘/⌃/⌥1–9 through the fragment endpoints on
  boundary-crossing; structured buffer storage (T4=A); caret-in-heading refusal (flash); `structuredCut`/
  `structuredPaste` history integration; single-scene fast path preserved (AC5); **verify AC1–AC7 live**.

## Notes

Next available Sprint: **SP-089**. Next available Task: **T-0354**.
