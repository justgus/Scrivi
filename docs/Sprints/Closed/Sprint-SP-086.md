# SP-086: [ScriviCore] EP-029 — `scrivi.fragment.v1` model + extract-fragment ✅ CLOSED

**Status:** ✅ **Closed 2026-07-27 (Human-approved).**
**Epic:** EP-029 `[Cross]` — Cross-Boundary Structured Cut/Copy/Paste (2nd of 5 sprints; 1st implementation).
**Start / Close:** 2026-07-27 / 2026-07-27.

## Goal (met)

ScriviCore **extract-fragment** — turn a manuscript range (an ordered list of `(sceneID, startByte, endByte)`
scene-local UTF-8 byte spans) into a `scrivi.fragment.v1` structured fragment. Read-only (⌘C non-destructive).
First of the three ScriviCore ops in the approved design (§4.1).

## Delivered (T-0351 — ✅ Verified)

- **`ScriviCore/src/manuscript/FragmentExtractor.{hpp,cpp}`** — self-contained read-only primitive (peer to the
  merge primitives, built from `CoreServices`). Produces the fragment model: ordered `pieces` (`opensWith`
  none/scene/chapter, `chapterTitle` on chapter boundaries, per-piece `text`, `partial` head/tail) + a
  blank-line-seam `plainText` flattening. Reuses `ManuscriptOrderResolver` (order + chapter membership + title)
  and `SceneReader` (raw scene bodies — divider/heading-free, so byte spans map straight onto disk content).
- **`scrivi_fragment_extract(projectRootPath, spansJson)`** C ABI endpoint (`scrivi_c_api.cpp`) — parses
  `{ "spans":[{sceneID,startByte,endByte},…] }`, runs the extractor, dumps the fragment envelope, all inside
  `guarded`. Added an `abiServices()` helper (mirrors the singleton wiring) for boundary-constructed primitives.
- **`scrivi.h`** — documented `scrivi_fragment_extract` (additive, pure C ABI).
- **`ScriviCore/tests/integration/FragmentExtractTests.cpp`** — 11 cases / 120 assertions.

## Key decision

**Standalone primitive, not a facade method.** Extract is read-only + self-contained, so it is invoked directly
at the C ABI rather than routed through a `ScriviCore::` facade (no Requests/Results/facade plumbing needed).
Same shape SP-056 used for `BufferStore`. Keeps the boundary a single additive C ABI endpoint.

## Verification

- **`ctest` 338/338** (was 327; +11 new — single-scene one-piece, single partial head, cross-scene same-chapter
  `opensWith:scene`, cross-chapter `opensWith:chapter`+`chapterTitle`, 3-piece head/interior/tail, zero-length
  boundary piece, `plainText` flattening, + 5 rejection cases). No regression.
- **macOS app `xcodebuild build` SUCCEEDED** against the grown header.
- `scrivi.h` additive-only; **no pbxproj** (ScriviCore → CMake; no new app-target files).

## Files

- New: `src/manuscript/FragmentExtractor.{hpp,cpp}`, `tests/integration/FragmentExtractTests.cpp`.
- Edited: `src/public_api/scrivi_c_api.cpp` (endpoint + `abiServices()`), `include/scrivi/scrivi.h`,
  `ScriviCore/CMakeLists.txt`, `ScriviCore/tests/CMakeLists.txt`.

## Retrospective

- **Went well:** the extractor mapped cleanly onto existing primitives (`ManuscriptOrderResolver` gives order +
  chapter membership + title; `SceneReader` gives divider/heading-free bodies), so extract is a thin, well-tested
  walk. The self-contained-primitive decision kept the change small and the boundary pure.
- **Next:** SP-087 — **paste-splice** (`scrivi_fragment_paste`): split the target scene at the caret and create
  the carried scenes/chapters via `createScene(afterSceneID)` / `createChapter(afterChapterID)` + the mid-scene
  split path. Consumes the fragment this sprint produces.

## Notes

Next available Sprint: **SP-087** (planned in EP-029). Next available Task: **T-0352**.
