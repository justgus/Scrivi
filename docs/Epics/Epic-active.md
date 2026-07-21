# Active Epics

## EP-028: [Cross] Scene & Chapter Merging — Linux Parity & Filesystem-Coherence Fix

**Status:** 🟡 Active
**Goal:** A writer can merge scenes and chapters by keyboard on **both** the macOS and Linux apps, and
the merge is filesystem-coherent under the EP-027 model (survives quit/reopen with no data loss).
Scene-merge = `⌘/Ctrl-Backspace` at the start of a scene → the scene joins the previous scene.
Chapter-merge = `⇧⌘/Ctrl-Shift-Backspace` at the start of a chapter's first scene → the whole chapter
merges into the previous chapter. No confirmation dialogs (both platforms behave identically).
**Date Created:** 2026-07-20
**Target Close Date:** 2026-07-27 (estimated, 3 sprints)
**Actual Close Date:** —

> **Codebase:** `[Cross]` — genuinely split: SP-074 is `[ScriviCore]` (two new C ABI merge endpoints +
> the coherence fix), SP-075 is `[Apple]` (adopt the endpoints, no behavior change for the user), SP-076
> is `[Linux]` (new parity). Kept as one Epic because the three legs deliver a single capability whose
> correctness (SP-074) is shared by both platforms.

### Background

The user asked to add "scene merging." Exploration found the **macOS app already implements** scene- and
chapter-merge (bound to `⌘-Backspace` / `⇧⌘-Backspace`, no confirmation) — it did not fall through the
cracks. So this Epic's real work is (1) Linux parity and (2) fixing a coherence bug the current
Swift-composed merge has under EP-027's filesystem-authoritative identity/ordering model.

**The coherence bug (I-0083):** Apple's chapter-merge reassigns scenes to the predecessor chapter
**in memory only** (`ViewportSceneLoader.mergeChapterIntoPredecessor`), then calls
`deleteChapter(currentChapterID)`. Under EP-027 the scene files physically live in the chapter's folder,
so `deleteChapter` deletes the scene files the in-memory model thinks it preserved → **scene loss on
reopen**. The fix is a first-class ScriviCore `scrivi_merge_chapter` that atomically **relocates** the
scene files into the predecessor folder before removing the emptied chapter. A companion
`scrivi_merge_scene` gives both platforms one shared, atomic path (same-chapter scene-merge is likely
already coherent as-composed; folding it into the endpoint removes the duplicated multi-step logic).

**Confirmed scope decisions (user, 2026-07-20):** match Apple's existing triggers on both platforms; the
cross-chapter case merges the **whole chapter** (existing Apple behavior), not a single scene; **no**
confirmation dialogs.

### Acceptance Criteria

- [x] AC1 — The chapter-merge data-loss path is reproduced as a failing ScriviCore integration test, and
  passes after the fix (no scene loss on reopen). (I-0083) — **SP-074:** `MergeSceneTests.cpp` documents the
  loss on the old `deleteChapter`-composed path; `scrivi_merge_chapter`'s test proves a 4-scene/2-chapter
  merge round-trips intact. _(core-verified; user verification pending)_
- [x] AC2 — `scrivi_merge_scene` merges a scene into the previous scene **within the same chapter**:
  text concatenated, absorbed scene's files removed, chapter cache rebuilt from disk; reopen round-trips.
  — **SP-074** (`SceneMerger`). _(core-verified; user verification pending)_
- [x] AC3 — `scrivi_merge_chapter` merges a whole chapter into the previous chapter by **relocating** all
  its scene files into the predecessor folder (order keys minted after the predecessor's last scene),
  then removing the emptied chapter folder + `manuscript.meta.json` entry; reopen round-trips with every
  scene present and in order. — **SP-074** (`ChapterMerger`). _(core-verified; user verification pending)_
- [x] AC4 — macOS: `⌘-Backspace` (scene) and `⇧⌘-Backspace` (chapter) behave exactly as today from the
  user's view, now backed by the endpoints; **chapter-merge survives quit/reopen** (the regression fix).
  — **SP-075** (T-0302/T-0303): handlers call `engine.mergeScene`/`mergeChapter`; `xcodebuild test` proves
  chapter-merge→reopen keeps every scene. _(build+interop-verified; live GUI/quit-reopen user verification
  pending.)_ **Note:** scene-merge now joins bodies with a blank line (was: run-together) — user-approved.
- [ ] AC5 — Linux: `Ctrl-Backspace` (scene) and `Ctrl-Shift-Backspace` (chapter) at the start of a
  scene/chapter perform the merge with parity to macOS (no confirmation), including reopen.
- [ ] AC6 — No-op at the very start of the manuscript on both platforms; history barriers
  (`sceneMerge`/`chapterMerge`) recorded on both platforms.
- [ ] AC7 — No regression: backend `ctest` + Apple interop suites + Linux smoke suites green; auto-save,
  navigation, structure ops, external-change scan unchanged. Writing-surface spec updated (merge is now
  supported — the current spec says it is not).

### Sprints

| Sprint | Title | Status | Dates |
| ------ | ----- | ------ | ----- |
| SP-074 | `[ScriviCore]` Merge endpoints (`scrivi_merge_scene` / `scrivi_merge_chapter`) + coherence fix | ✅ Closed | 2026-07-20 – 2026-07-21 |
| SP-075 | `[Apple]` Adopt the merge endpoints (regression-safe swap) | 🟡 Active | 2026-07-21 – |
| SP-076 | `[Linux]` Scene & chapter merge parity | 🔵 Planning | — |

### Tasks

| ID     | Title | Sprint | Status |
| ------ | ----- | ------ | ------ |
| T-0298 | Reproduce chapter-merge data-loss (`MergeSceneTests.cpp`) + confirm same-chapter scene-merge coherence | SP-074 | 🟢 Implemented, Not Verified |
| T-0299 | `scrivi_merge_scene` — `SceneMerger`, request/result, facade, C ABI, `scrivi.h`, CMake | SP-074 | 🟢 Implemented, Not Verified |
| T-0300 | `scrivi_merge_chapter` — atomic cross-folder relocation + emptied-chapter removal (fixes I-0083) | SP-074 | 🟢 Implemented, Not Verified |
| T-0301 | Merge integration coverage + register in `tests/CMakeLists.txt`; `ctest` green macOS + Linux | SP-074 | 🟢 Implemented, Not Verified |
| T-0302 | `[Apple]` `ScriviEngine` `mergeScene`/`mergeChapter` wrappers + result structs + interop tests | SP-075 | 🟢 Implemented, Not Verified |
| T-0303 | `[Apple]` Point `handleMergeScene`/`handleMergeChapter` at the endpoints; keep bindings/barriers | SP-075 | 🟢 Implemented, Not Verified |
| T-0304 | `[Linux]` `ManuscriptEditor` `Ctrl/Ctrl-Shift-Backspace` → `mergeScene/ChapterRequested` signals | SP-076 | 🔵 Backlog |
| T-0305 | `[Linux]` `ScriviBridge` `mergeScene`/`mergeChapter` + `SceneDocument` merge splices | SP-076 | 🔵 Backlog |
| T-0306 | `[Linux]` `EditorShell` merge slots (guards, caret re-anchor) + `scene_merge_smoke` test | SP-076 | 🔵 Backlog |
| T-0307 | Update `docs/Scrivi_WritingSurface_Behavior_Spec_v0_1.md` (merge now supported) | SP-076 | 🔵 Backlog |

### Issues

| ID     | Title | Severity | Status |
| ------ | ----- | -------- | ------ |
| I-0083 | Chapter-merge loses scenes on reopen (in-memory reassign + `deleteChapter` deletes on-disk scene files under EP-027) | High | 🟢 Resolved (core, SP-074), Not Verified — app adoption SP-075/076 |

### Scope Notes

- Cross-chapter merge = **whole-chapter** merge (user decision), matching existing macOS behavior — not a
  single-scene-across-the-boundary merge.
- **No confirmation dialogs** on either platform (user decision); both apps behave identically.
- Merge records history **barriers** (`sceneMerge`/`chapterMerge`), consistent with EP-019 §4.5 (structural
  undo remains out of scope; the barrier vocabulary is already reserved in `HistoryService`).
- The boundary stays pure C ABI / JSON-over-`std::string` — two new `scrivi_*` entry points, no struct interop.

### Completion Summary

_(filled in when the Epic reaches 🟠 Complete)_

---

*Last Updated: 2026-07-21 (**SP-074 ✅ closed** (Human-approved) — both merge endpoints + the atomic
I-0083 core fix; AC1/AC2/AC3 met at core. **SP-075 `[Apple]` T-0302 + T-0303 🟢 Implemented, Not Verified**
— `ScriviEngine` merge wrappers + `handleMergeScene`/`handleMergeChapter` now call the endpoints (retiring the
`deleteChapter`-composed chapter-merge = the I-0083 cause); `xcodebuild build`+`test` green (36/36 interop,
incl. 4 new merge tests proving chapter-merge→reopen keeps all scenes). **AC4 met at build+test level**
(live GUI verification pending). Scene-merge join separator changed to blank-line (user-approved). Remaining:
SP-075 user GUI verify + close, then SP-076 `[Linux]` parity. No other Active Epics.)*
