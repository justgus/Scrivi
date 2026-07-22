# EP-028 (Closed)

## EP-028: [Cross] Scene & Chapter Merging — Linux Parity & Filesystem-Coherence Fix

**Status:** ✅ Closed (Human-approved 2026-07-22)
**Goal:** A writer can merge scenes and chapters by keyboard on **both** the macOS and Linux apps, and
the merge is filesystem-coherent under the EP-027 model (survives quit/reopen with no data loss).
Scene-merge = `⌘/Ctrl-Backspace` at the start of a scene → the scene joins the previous scene.
Chapter-merge = `⇧⌘/Ctrl-Shift-Backspace` at the start of a chapter's first scene → the whole chapter
merges into the previous chapter. No confirmation dialogs (both platforms behave identically).
**Date Created:** 2026-07-20
**Target Close Date:** 2026-07-27 (estimated, 3 sprints)
**Actual Close Date:** 2026-07-22

> **Codebase:** `[Cross]` — genuinely split: SP-074 is `[ScriviCore]` (two new C ABI merge endpoints +
> the coherence fix), SP-075 is `[Apple]` (adopt the endpoints, no behavior change for the user), SP-076
> is `[Linux]` (new parity). Kept as one Epic because the three legs deliver a single capability whose
> correctness (SP-074) is shared by both platforms. **SP-077** (`[Linux]` native menu bar) was added mid-Epic
> to unblock chapter-merge verification over VNC.

### Background

The user asked to add "scene merging." Exploration found the **macOS app already implements** scene- and
chapter-merge (bound to `⌘-Backspace` / `⇧⌘-Backspace`, no confirmation) — it did not fall through the
cracks. So this Epic's real work is (1) Linux parity and (2) fixing a coherence bug the current
Swift-composed merge has under EP-027's filesystem-authoritative identity/ordering model.

**The coherence bug (I-0083):** Apple's chapter-merge reassigned scenes to the predecessor chapter
**in memory only** (`ViewportSceneLoader.mergeChapterIntoPredecessor`), then called
`deleteChapter(currentChapterID)`. Under EP-027 the scene files physically live in the chapter's folder,
so `deleteChapter` deleted the scene files the in-memory model thought it preserved → **scene loss on
reopen**. The fix is a first-class ScriviCore `scrivi_merge_chapter` that atomically **relocates** the
scene files into the predecessor folder before removing the emptied chapter. A companion
`scrivi_merge_scene` gives both platforms one shared, atomic path.

**Confirmed scope decisions (user, 2026-07-20):** match Apple's existing triggers on both platforms; the
cross-chapter case merges the **whole chapter** (existing Apple behavior), not a single scene; **no**
confirmation dialogs.

### Acceptance Criteria — all met

- [x] AC1 — The chapter-merge data-loss path is reproduced as a failing ScriviCore integration test, and
  passes after the fix (no scene loss on reopen). (I-0083) — **SP-074:** `MergeSceneTests.cpp` documents the
  loss on the old `deleteChapter`-composed path; `scrivi_merge_chapter`'s test proves a 4-scene/2-chapter
  merge round-trips intact. ✅ **Verified.**
- [x] AC2 — `scrivi_merge_scene` merges a scene into the previous scene **within the same chapter**:
  text concatenated, absorbed scene's files removed, chapter cache rebuilt from disk; reopen round-trips.
  — **SP-074** (`SceneMerger`). ✅ **Verified.**
- [x] AC3 — `scrivi_merge_chapter` merges a whole chapter into the previous chapter by **relocating** all
  its scene files into the predecessor folder (order keys minted after the predecessor's last scene),
  then removing the emptied chapter folder + `manuscript.meta.json` entry; reopen round-trips with every
  scene present and in order. — **SP-074** (`ChapterMerger`). ✅ **Verified.**
- [x] AC4 — macOS: `⌘-Backspace` (scene) and `⇧⌘-Backspace` (chapter) behave exactly as today from the
  user's view, now backed by the endpoints; **chapter-merge survives quit/reopen** (the regression fix).
  — **SP-075** (T-0302/T-0303). ✅ **Verified (2026-07-21)** — user confirmed the live GUI + quit/reopen
  flow (I-0083 closed). **Note:** scene-merge now joins bodies with a blank line (was: run-together) —
  user-approved.
- [x] AC5 — Linux: `Ctrl-Backspace` (scene) and `Ctrl-Shift-Backspace` (chapter) at the start of a
  scene/chapter perform the merge with parity to macOS (no confirmation), including reopen. — **SP-076**
  (T-0304–T-0306). ✅ **Verified (2026-07-22)** — scene-merge live via `Ctrl-Backspace`; chapter-merge live
  via the **SP-077 Chapter ▸ Merge menu** (the `Ctrl-Shift-Backspace` keystroke is swallowed by the
  macOS→VNC input path when Shift is held — proven not an app bug; the binding is kept for native Linux).
  Both survive quit→reopen with nothing lost.
- [x] AC6 — No-op at the very start of the manuscript on both platforms; history barriers
  (`sceneMerge`/`chapterMerge`) recorded. ✅ **Verified.** macOS records the barriers (SP-075); **Linux has
  no history subsystem** (EP-019 `HistoryCapture` is Apple-only — its structure ops record no barrier
  either), so the Linux merge records none — a documented platform gap, not an omission. Manuscript-start
  no-op verified on both.
- [x] AC7 — No regression: backend `ctest` + Apple interop suites + Linux smoke suites green; auto-save,
  navigation, structure ops, external-change scan unchanged. Writing-surface spec updated (merge is now
  supported). ✅ **Verified** — ctest macOS 317/317 + Linux 324/324 (SP-074); Apple interop 36/36 (SP-075);
  Linux container smokes green incl. new `scene_merge_smoke` (SP-076); `Scrivi_WritingSurface_Behavior_Spec_v0_1.md`
  §8 clarified + §8.1 added (T-0307).

### Sprints

| Sprint | Title | Status | Dates |
| ------ | ----- | ------ | ----- |
| SP-074 | `[ScriviCore]` Merge endpoints (`scrivi_merge_scene` / `scrivi_merge_chapter`) + coherence fix | ✅ Closed | 2026-07-20 – 2026-07-21 |
| SP-075 | `[Apple]` Adopt the merge endpoints (regression-safe swap) | ✅ Closed | 2026-07-21 – 2026-07-21 |
| SP-076 | `[Linux]` Scene & chapter merge parity | ✅ Closed | 2026-07-21 – 2026-07-22 |
| SP-077 | `[Linux]` Native menu bar (unblocked AC5/AC6 Linux verification over VNC) | ✅ Closed | 2026-07-22 – 2026-07-22 |

### Tasks

| ID     | Title | Sprint | Status |
| ------ | ----- | ------ | ------ |
| T-0298 | Reproduce chapter-merge data-loss (`MergeSceneTests.cpp`) + confirm same-chapter scene-merge coherence | SP-074 | ✅ Verified |
| T-0299 | `scrivi_merge_scene` — `SceneMerger`, request/result, facade, C ABI, `scrivi.h`, CMake | SP-074 | ✅ Verified |
| T-0300 | `scrivi_merge_chapter` — atomic cross-folder relocation + emptied-chapter removal (fixes I-0083) | SP-074 | ✅ Verified |
| T-0301 | Merge integration coverage + register in `tests/CMakeLists.txt`; `ctest` green macOS + Linux | SP-074 | ✅ Verified |
| T-0302 | `[Apple]` `ScriviEngine` `mergeScene`/`mergeChapter` wrappers + result structs + interop tests | SP-075 | ✅ Verified (2026-07-21) |
| T-0303 | `[Apple]` Point `handleMergeScene`/`handleMergeChapter` at the endpoints; keep bindings/barriers | SP-075 | ✅ Verified (2026-07-21) |
| T-0304 | `[Linux]` `ManuscriptEditor` `Ctrl/Ctrl-Shift-Backspace` → `mergeScene/ChapterRequested` signals | SP-076 | ✅ Verified (2026-07-22) |
| T-0305 | `[Linux]` `ScriviBridge` `mergeScene`/`mergeChapter` invokables (bridge to the C ABI) | SP-076 | ✅ Verified (2026-07-22) |
| T-0306 | `[Linux]` `EditorShell` merge slots (guards, reload-from-disk, caret re-anchor) + `scene_merge_smoke` test | SP-076 | ✅ Verified (2026-07-22) |
| T-0307 | Update `docs/Scrivi_WritingSurface_Behavior_Spec_v0_1.md` (merge now supported — §8.1) | SP-076 | ✅ Verified (doc) |
| T-0308 | `[Linux]` Faint between-scene separator rule (`paintEvent` overlay — Apple `DividerAttachmentCell` parity) | SP-076 | ✅ Verified (2026-07-22) |
| T-0309 | `[Linux]` `Ctrl+Return` mid-scene split (macOS `⌘↩` parity) | SP-076 | ✅ Verified (2026-07-22) |
| T-0310 | `[Linux]` Menu bar scaffold + File/Edit | SP-077 | ✅ Verified (2026-07-22) |
| T-0311 | `[Linux]` Scene/Chapter menus + public `EditorShell` triggers (makes chapter-merge testable over VNC) | SP-077 | ✅ Verified (2026-07-22) |
| T-0312 | `[Linux]` Project ▸ Settings… stub dialog | SP-077 | ✅ Verified (2026-07-22) |
| T-0313 | `[Linux]` Verify chapter-merge live via Chapter ▸ Merge (closes AC5/AC6 Linux) + strip TEMP diagnostics | SP-077 | ✅ Verified (2026-07-22) |
| T-0314 | `[Linux]` File ▸ New Project opens the New Project panel | SP-077 | ✅ Verified (2026-07-22) |
| T-0315 | `[Linux]` File ▸ Open Project shows the folder picker | SP-077 | ✅ Verified (2026-07-22) |
| T-0316 | `[Linux]` Flush-safe editor-leaving paths + remove raw ‹ Close button (data-safety fix) | SP-077 | ✅ Verified (2026-07-22) |
| T-0317 | `[Linux]` Ctrl+W closes the project | SP-077 | ✅ Verified (2026-07-22) |

### Issues

| ID     | Title | Severity | Status |
| ------ | ----- | -------- | ------ |
| I-0083 | Chapter-merge loses scenes on reopen (in-memory reassign + `deleteChapter` deletes on-disk scene files under EP-027) | High | ✅ Resolved - Verified (2026-07-21, SP-074 core + SP-075 app adoption) |
| I-0084 | Caret jumps to the next scene/chapter start after a scene merge (redundant `updateNSView` rebuild dropped selection) | Medium | ✅ Resolved - Verified (2026-07-21, SP-075) |
| I-0085 | App crashes constructing the Open Project panel (`NSOpenPanel` XPC timeout) — environmental, NOT a code bug | High | ✅ Closed — Not a Bug (2026-07-21, SP-075) |
| I-0086 | Build warnings: "Result of 'try?' is unused" at three call sites | Low | ✅ Resolved - Not Verified (2026-07-21, SP-075) |

### Scope Notes

- Cross-chapter merge = **whole-chapter** merge (user decision), matching existing macOS behavior — not a
  single-scene-across-the-boundary merge.
- **No confirmation dialogs** on either platform (user decision); both apps behave identically.
- Merge records history **barriers** (`sceneMerge`/`chapterMerge`) on macOS, consistent with EP-019 §4.5
  (structural undo remains out of scope; the barrier vocabulary is already reserved in `HistoryService`).
  Linux has no history subsystem yet (documented platform gap).
- The boundary stayed pure C ABI / JSON-over-`std::string` — two new `scrivi_*` entry points, no struct interop.

### Completion Summary

EP-028 delivered keyboard scene- and chapter-merge on **both** macOS and Linux, backed by two new atomic
ScriviCore endpoints, and fixed the EP-027 chapter-merge data-loss regression (I-0083) at the core:

- **SP-074 `[ScriviCore]`** — `scrivi_merge_scene` (`SceneMerger`) + `scrivi_merge_chapter` (`ChapterMerger`,
  the atomic I-0083 fix: relocates scene files into the predecessor **before** removing the emptied chapter).
  11 merge tests incl. the I-0083 loss guard on the old path; ctest macOS 317/317 + Linux 324/324; both C
  symbols exported in `libScriviCore.a`; `scrivi.h` boundary stayed pure C ABI.
- **SP-075 `[Apple]`** — pointed the macOS `⌘-Backspace`/`⇧⌘-Backspace` merge commands at the endpoints
  (retiring the `deleteChapter`-composed chapter-merge = the I-0083 cause); new `ScriviEngine` wrappers;
  36/36 interop. Scene-merge join changed to blank-line (user-approved). I-0083/I-0084 Verified live;
  I-0085 Not-a-Bug; I-0086 fixed.
- **SP-076 `[Linux]`** — merge parity: `ManuscriptEditor` signals → `ScriviBridge` invokables → `EditorShell`
  slots (reload-from-disk, disk-authoritative, sidesteps the I-0081 stale-path class). New `scene_merge_smoke`.
  Plus two Apple-parity extras surfaced during VNC testing: the between-scene separator rule (T-0308) and
  `Ctrl+Return` mid-scene split (T-0309).
- **SP-077 `[Linux]`** — native menu bar (File/Edit/Scene/Chapter/Project), added because the macOS→VNC input
  path swallows `Ctrl-Shift-Backspace` (Shift held) — the menu gave chapter-merge a deliverable trigger and
  is standard desktop UX regardless. Also closed a data-loss hole (flush-safe editor-leaving paths, T-0316)
  found while exercising the menu.

All acceptance criteria AC1–AC7 Verified. `scrivi.h` gained only the two additive merge endpoints; no struct
interop; the JSON-over-`std::string` boundary held.

---

*Closed 2026-07-22 (Human-approved). EP-028 delivered cross-platform keyboard scene/chapter merge (macOS +
Linux) and fixed the I-0083 chapter-merge data-loss regression atomically at the core. 4 sprints (SP-074–SP-077),
20 tasks, 4 issues. AC1–AC7 all Verified. Next in line: EP-024–EP-026 `[Linux]` (Draft). Archived here.*
