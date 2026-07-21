# Sprints - Index

This is the main index for all Scrivi Sprints. Sprints are fixed-duration iterations that group Tasks and Issues into focused units of work.

> **Related:** [Tasks (T)](../Tasks/Task-Documentation.md) | [Issues (I)](../Issues/Issue-Documentation.md) | [Epics (EP)](../Epics/Epic-Documentation.md)

## Organization

- **Sprint-active.md** — The currently active Sprint(s); normally one, but parallel sprints across different Epics are permitted
- **Closed/Sprint-SP-XXX.md** — Archived closed Sprints, one file each

## Active Sprints

**SP-075** `[Apple]` (EP-028 — **Adopt the merge endpoints**): point the macOS `⌘-Backspace` /
`⇧⌘-Backspace` merge commands at SP-074's `scrivi_merge_scene` / `scrivi_merge_chapter` — a regression-safe
backend swap delivering the app-side of the I-0083 chapter-merge fix (AC4). Swift-only; `scrivi.h`/ScriviCore
untouched. 🟡 **Active from 2026-07-21.** Tasks T-0302–T-0303. See `Sprint-active.md`.

Last closed: **SP-074** (EP-028 `[ScriviCore]` — **Merge endpoints + filesystem-coherence fix**) — ✅
**closed 2026-07-21 (Human-approved).** Two new atomic C ABI endpoints `scrivi_merge_scene` (`SceneMerger`) /
`scrivi_merge_chapter` (`ChapterMerger` — the atomic I-0083 fix: relocates scene files into the predecessor
BEFORE removing the emptied chapter). 11 merge tests incl. the I-0083 loss guard on the old
`deleteChapter`-composed path; ctest **macOS 317/317 + Linux 324/324**; both C symbols exported in
`libScriviCore.a`; `scrivi.h` boundary stayed pure C ABI. Fixed stale `devops/docker/linux/Dockerfile`
(missing `libssl-dev`). **I-0083 Resolved at core, Not Verified** — app adoption is SP-075/076. Tasks
T-0298–T-0301. `Closed/Sprint-SP-074.md`.

Prior: **SP-073** (EP-023 `[Linux]` — **chapter drag-reorder AC5 + full EP-023 verify & Epic close**;
renumbered from the skipped SP-068) — ✅ **closed 2026-07-19 (Human-approved).** Chapter rows drag as
containers (CopyAction-only, boundary-only drop resolution, `SceneDocument::moveChapter` block splice,
post-reslug path refresh via new reorder-envelope paths; `scrivi.h` untouched). Fixed & Verified en route:
**I-0080** (`[ScriviCore]` open-time migration undid chapter reorders — legacy gate + eager index-cache
rebuild), **I-0081** (stale scene paths after drag broke rename/save), **I-0082** (non-selectable chapter
rows blocked the drag). ctest **306/306 macOS + 313/313 Linux**; **11/11** container smokes (new
`chapter_reorder_smoke` in CI). Tasks **T-0294–T-0297** Verified & archived. **This closed EP-023** (all
ACs AC1–AC8 Verified; `../Epics/Closed/Epic-EP-023.md`). `Closed/Sprint-SP-073.md`.

Prior: **SP-072** (EP-027 — **P5 Apple-verify migration fixes**; a real legacy project failed to open on
macOS with "Repair required: Missing scene content file"). — ✅ **closed 2026-07-18 (Human-approved).** Fixed
**I-0078** (macOS `createChapter` wrapper drifted behind `scrivi.h`'s `afterChapterID` — app hadn't compiled since
SP-071), **I-0076** (legacy scene `content.path` not bared on migration → dangling after chapter reslug;
`parseSceneMeta` now bares it + `migrateScenes` normalises stale sidecars), **I-0077** (chapter with index/sidecar
id mismatch dropped as phantom → path-folder fallback in `migrateChapterOrderKeys`). 2 regression tests, each proven
RED-without-fix. ctest **304/304 macOS, 311/311 Linux**; Human opened a fresh copy of the real project → clean.
**AC7+AC8 Verified.** Tasks **T-0290–T-0293**. `Closed/Sprint-SP-072.md`.

Prior: **SP-071** (EP-027/EP-023 — **create-chapter-in-place**; fixes the Linux chapter-split
lost-tail/folder-scramble + open-time scene-stealing, **I-0074**; arrows cross scene/chapter boundaries, **I-0075**)
— ✅ **closed 2026-07-18 (Human-approved).** `createChapter(afterChapterID)` (additive `scrivi.h` change) so the
new chapter is born in place — no create-then-reorder, no stale path; scene cache is a pure disk mirror + removed
the unsafe filename-based orphan relocation (order-key scene filenames repeat across chapters). Split dialog removed
(Ctrl+Shift+Return is the approval). **Verified over VNC** (all four split cases update + reopen clean; arrow nav
fixed). ctest **302/302** macOS. Tasks **T-0279–T-0289**. `Closed/Sprint-SP-071.md`. Prior: **SP-067** (EP-023 `[Linux]` Scene drag-reorder + I-0064 chapter-split + I-0063 renumber) — ✅
**closed 2026-07-16**. Delivered **AC4 scene drag-reorder** (I-0067/I-0068 fixed by forcing the drag to
`Qt::CopyAction` so Qt never auto-removes the source row; user-verified "It's clean" on a fresh project over VNC)
and **I-0063** renumber (T-0262, Verified). The chapter-split pieces — **I-0064/I-0069/I-0070**, the root slug
corruption **I-0072**, and **I-0071** — were **re-homed to the new EP-027** (rebuilt on its filesystem-authoritative
on-disk model rather than fixed twice). Follow-on **I-0073** (VNC drag-hover lag, likely environmental) flagged.
`Closed/Sprint-SP-067.md`. Prior: **SP-066** (EP-023 rename + I-0062) ✅ closed 2026-07-15. SP-056/SP-057 (EP-019
`[Apple]`) remain in Planning, parked behind the deferred EP-019 — see Backlog.

## Sprint Backlog

See: [Sprint-backlog.md](Sprint-backlog.md)

| Sprint | Title | Epic | Status |
| ------ | ----- | ---- | ------ |
| SP-056 | Copy buffers — store, ABI, HUD/palette UX, history integration | EP-019 | 🔵 Planning (parked) |
| SP-057 | Undo/Redo — history panel, perf fixtures, verification & Epic close | EP-019 | 🔵 Planning (parked) |

## All Sprints

Currently: **73 Sprints** (70 closed + 1 cancelled + 2 in Planning + 1 active; the SP-068 **ID** was skipped
when EP-027 claimed SP-069–SP-072 — its EP-023 chapter-drag scope shipped as **SP-073**, ✅ closed
2026-07-19) | Next available: **SP-076**

| Sprint | Title | Epic | Start | End | Status |
| ------ | ----- | ---- | ----- | --- | ------ |
| SP-001 | ScriviCore Foundation — Build, Types, and Utilities | EP-001 | 2026-05-19 | 2026-05-20 | ✅ Closed |
| SP-002 | ScriviCore Services — Project Lifecycle and Repair | EP-002 | 2026-05-20 | 2026-05-20 | ✅ Closed |
| SP-003 | ScriviCore Swift Interop | EP-002 / EP-003 | 2026-05-20 | 2026-05-21 | ✅ Closed |
| SP-004 | Skeleton v0.2 Migration — Structural Alignment | EP-007 | 2026-05-26 | 2026-05-26 | ✅ Closed |
| SP-005 | Skeleton v0.2 Migration — Milestone 10 and 11 Verification | EP-007 | 2026-05-26 | 2026-05-26 | ✅ Closed |
| SP-006 | Skeleton v0.2 Migration — Adapter and Swift Completion | EP-007 | 2026-05-26 | 2026-05-26 | ✅ Closed |
| SP-007 | EP-002 Completion — Full MVP Loop Test | EP-002 | 2026-05-26 | 2026-05-26 | ✅ Closed |
| SP-008 | Repair and Recovery — Schema and Facade | EP-004 | 2026-05-26 | 2026-05-26 | ✅ Closed |
| SP-009 | Repair and Recovery — Handlers, Tests, and Adapter | EP-004 | 2026-05-27 | 2026-05-27 | ✅ Closed |
| SP-010 | Objects Layer Foundation — Character Schema and CRUD | EP-005 | 2026-05-27 | 2026-05-27 | ✅ Closed |
| SP-011 | Objects Layer — Remaining Types and Generalized CRUD | EP-005 | 2026-05-28 | 2026-05-28 | ✅ Closed |
| SP-012 | Assets and Comments Layer | EP-005 | 2026-05-28 | 2026-05-28 | ✅ Closed |
| SP-013 | Inbox and Adapter Completion | EP-005 | 2026-05-28 | 2026-05-28 | ✅ Closed |
| SP-014 | Infrastructure — Keychain Identity and JsonDoc Double | EP-006 | 2026-05-28 | 2026-05-29 | ✅ Closed |
| SP-015 | macOS App Target and SwiftUI Shell | EP-006 | 2026-05-29 | 2026-05-29 | ✅ Closed |
| SP-016 | saveScene Wiring and EP-006 Close | EP-006 | 2026-05-30 | 2026-05-30 | ✅ Closed |
| SP-017 | Cross-Platform Build — Ubuntu (GCC/Clang) | EP-008 | 2026-05-30 | 2026-05-31 | ✅ Closed |
| SP-018 | Cross-Platform Build — Windows (MSVC) + SecureStore Trade Study | EP-008 | 2026-05-31 | 2026-06-01 | ✅ Closed |
| SP-019 | Multi-Scene C++ Core — `openProject` Scene List + `openScene` | EP-008 | 2026-06-01 | 2026-06-01 | ✅ Closed |
| SP-020 | clang-tidy Housekeeping — Full Warning Sweep | EP-008 | 2026-06-01 | 2026-06-01 | ✅ Closed |
| SP-021 | C++ Core — `createScene` and `createChapter` | EP-009 | 2026-06-01 | 2026-06-01 | ✅ Closed |
| SP-022 | Adapter + Swift Engine — `createScene` and `createChapter` | EP-009 | 2026-06-01 | 2026-06-01 | ✅ Closed |
| SP-023 | Writing Surface — `NSTextView`, Viewport Loader, and Auto-Save | EP-009 | 2026-06-01 | 2026-06-01 | ✅ Closed |
| SP-024 | Scene Navigator — Sidebar, Live Titles, and Click-to-Navigate | EP-009 | 2026-06-01 | 2026-06-03 | ✅ Closed |
| SP-025 | Scroll-Driven Scene Switching and EP-009 Close | EP-009 | 2026-06-03 | 2026-06-03 | ✅ Closed |
| SP-026 | Apple Platform C API Boundary — Retire C++ Adapter, Wire scrivi.h | EP-009 | 2026-06-02 | 2026-06-02 | ✅ Closed |
| SP-027 | C++ Core: deleteScene and deleteChapter | EP-010 | 2026-06-04 | 2026-06-04 | ✅ Closed |
| SP-028 | C++ Core: renameScene and renameChapter | EP-010 | 2026-06-04 | 2026-06-04 | ✅ Closed |
| SP-029 | C++ Core: reorderScene and reorderChapter | EP-010 | 2026-06-04 | 2026-06-04 | ✅ Closed |
| SP-030 | Adapter + Swift Engine: Delete and Rename | EP-010 | 2026-06-04 | 2026-06-04 | ✅ Closed |
| SP-031 | Drag Reorder: Scene and Chapter (SwiftUI) | EP-010 | 2026-06-04 | 2026-06-05 | ✅ Closed |
| SP-032 | Chapter Title Toggle, Navigator Fallback, and EP-010 Close | EP-010 | 2026-06-06 | 2026-06-06 | ✅ Closed |
| SP-033 | Writing Surface — All-in-Memory Viewport, Separator, Cursor, and Focus | EP-011 | 2026-06-08 | 2026-06-08 | ✅ Closed |
| SP-034 | Writing Surface — Scroll Bar Fidelity and EP-011 Close | EP-011 | — | — | ⚪ Cancelled |
| SP-035 | Menu Bar, Project Commands, and About Panel | EP-012 | 2026-06-09 | 2026-06-09 | ✅ Closed |
| SP-036 | Scene Navigator Swipe Gesture — macOS | EP-013 | 2026-06-10 | 2026-06-10 | ✅ Closed |
| SP-037 | Scene Inspector Panel | EP-014 | 2026-06-10 | 2026-06-10 | ✅ Closed |
| SP-038 | Real-Time Timeline Panel | EP-015 | 2026-06-10 | 2026-06-10 | ✅ Closed |
| SP-039 | Timeline Persistence — ScriviCore schema and API | EP-016 | 2026-06-11 | 2026-06-11 | ✅ Closed |
| SP-040 | Timeline SwiftUI — core rendering, drag, and Time Delta Picker | EP-016 | 2026-06-11 | 2026-06-11 | ✅ Closed |
| SP-041 | Story Structure bands — overlay, drag borders, band assignment | EP-016 | 2026-06-12 | 2026-06-12 | ✅ Closed |
| SP-042 | Historical events, imported timelines, export, and clustering | EP-016 | 2026-06-12 | 2026-06-16 | ✅ Closed |
| SP-043 | EP-016 verification and polish | EP-016 | 2026-06-16 | 2026-06-23 | ✅ Closed |
| SP-044 | Spotlight — design sign-off & ScriviCore indexing facade | EP-017 | 2026-06-23 | 2026-06-23 | ✅ Closed |
| SP-045 | Spotlight — Layer 1: in-app Core Spotlight donations & deep-link | EP-017 | 2026-06-23 | 2026-06-25 | ✅ Closed |
| SP-046 | Spotlight — Layer 2: on-disk `.scrivi` importer extension (Layer 2 descoped — I-0057) | EP-017 | 2026-06-25 | 2026-07-01 | ✅ Closed |
| SP-047 | Spotlight — verification, cross-platform assessment, Epic close | EP-017 | 2026-07-01 | 2026-07-06 | ✅ Closed |
| SP-048 | Per-window model — foundation (spike, ProjectSession, registry) | EP-018 | 2026-06-24 | 2026-06-24 | ✅ Closed |
| SP-049 | Per-window model — windowing & restore-all-windows | EP-018 | 2026-06-24 | 2026-06-24 | ✅ Closed |
| SP-050 | Per-window model — deep-link rewrite & EP-018 verification | EP-018 | 2026-06-24 | 2026-06-25 | ✅ Closed |
| SP-051 | Undo/Redo — design sign-off, ⌘Z-routing spike, schema spec | EP-019 | 2026-07-06 | 2026-07-06 | ✅ Closed |
| SP-052 | Undo/Redo — linear history engine core (C++) + C ABI + Swift wrappers | EP-019 | 2026-07-07 | 2026-07-07 | ✅ Closed |
| SP-053 | Undo/Redo — in-session undo/redo on macOS (capture, apply, barriers) | EP-019 | 2026-07-07 | 2026-07-07 | ✅ Closed |
| SP-054 | Undo/Redo — persistence, sessions, capacity, settings | EP-019 | 2026-07-07 | 2026-07-09 | ✅ Closed |
| SP-055 | Undo/Redo — branching: tree ops, fork popover, purge | EP-019 | 2026-07-10 | 2026-07-13 | ✅ Closed |
| SP-056 | Copy buffers — store, ABI, HUD/palette UX, history integration | EP-019 `[Apple]` | — | — | 🔵 Planning (parked) |
| SP-057 | Undo/Redo — history panel, perf fixtures, verification & Epic close | EP-019 `[Apple]` | — | — | 🔵 Planning (parked) |
| SP-058 | `[Linux]` Docker/VNC harness + Qt6 skeleton + hello-ScriviCore slice + CI | EP-020 `[Linux]` | 2026-07-13 | 2026-07-13 | ✅ Closed |
| SP-059 | `[Linux]` Project Lifecycle — appSupportRoot + recents + landing + create-project | EP-021 `[Linux]` | 2026-07-13 | 2026-07-14 | ✅ Closed |
| SP-060 | `[Linux]` Project Lifecycle — Open + Close + full-loop Verify (EP-021 close half) | EP-021 `[Linux]` | 2026-07-14 | 2026-07-14 | ✅ Closed |
| SP-061 | `[Linux]` Shell flip to Widgets host + Scene navigator + read-only continuous viewport | EP-022 `[Linux]` | 2026-07-14 | 2026-07-14 | ✅ Closed |
| SP-062 | `[Linux]` Editable viewport + per-scene auto-save + in-editor ⌘↩/⌘⇧↩ Scene/Chapter creation | EP-022 `[Linux]` | 2026-07-14 | 2026-07-14 | ✅ Closed |
| SP-063 | `[Linux]` Scroll-driven scene switching + active-scene tracking | EP-022 `[Linux]` | 2026-07-15 | 2026-07-15 | ✅ Closed |
| SP-064 | `[Linux]` Cursor/focus + quit-reopen restore + EP-022 verify & close | EP-022 `[Linux]` | 2026-07-15 | 2026-07-15 | ✅ Closed |
| SP-065 | `[Linux]` Delete scene/chapter — context menu, confirmation, removal splice + delete-of-active | EP-023 `[Linux]` | 2026-07-15 | 2026-07-15 | ✅ Closed |
| SP-066 | `[Linux]` Rename scene/chapter — context menu, inline edit, live heading + I-0062 | EP-023 `[Linux]` | 2026-07-15 | 2026-07-15 | ✅ Closed |
| SP-067 | `[Linux]` Scene drag-reorder (AC4) + I-0063 renumber — split/renumber (I-0064/69/70/72) re-homed to EP-027 | EP-023 `[Linux]` | 2026-07-15 | 2026-07-16 | ✅ Closed |
| SP-069 | `[ScriviCore]` EP-027 P1–P3 — rename primitive + order-key/disk-authority + legacy migration (I-0072 fixed) | EP-027 `[ScriviCore]` | 2026-07-16 | 2026-07-18 | ✅ Closed |
| SP-070 | `[ScriviCore]` EP-027 P6 — filesystem-authoritative scene identity & ordering (scenes) | EP-027 `[ScriviCore]` | 2026-07-17 | 2026-07-18 | ✅ Closed |
| SP-071 | `[ScriviCore]`+`[Linux]` create-chapter-in-place — split lost-tail/scene-steal fix (I-0074) + arrow nav (I-0075) | EP-027 / EP-023 | 2026-07-17 | 2026-07-18 | ✅ Closed |
| SP-072 | `[ScriviCore]`+`[Apple]` EP-027 P5 Apple-verify — legacy migration fixes (I-0076/I-0077/I-0078) | EP-027 `[Apple]` | 2026-07-18 | 2026-07-18 | ✅ Closed |
| SP-073 | `[Linux]` Chapter drag-reorder (AC5) + full EP-023 verify & Epic close — renumbered from SP-068 | EP-023 `[Linux]` | 2026-07-19 | 2026-07-19 | ✅ Closed |
| SP-074 | `[ScriviCore]` Merge endpoints (`scrivi_merge_scene`/`scrivi_merge_chapter`) + filesystem-coherence fix (I-0083) | EP-028 `[Cross]` | 2026-07-20 | 2026-07-21 | ✅ Closed |
| SP-075 | `[Apple]` Adopt the merge endpoints (regression-safe swap) — app-side I-0083 fix (AC4) | EP-028 `[Cross]` | 2026-07-21 | — | 🟡 Active |

## Statistics

- **Total Sprints:** 74 (SP-001–SP-075 issued; SP-068 skipped)
- **Closed:** 70 ✅
- **Active:** 1 🟡 (SP-075 EP-028 `[Apple]`)
- **Planning:** 2 🔵 (SP-056–SP-057 EP-019 `[Apple]` — parked behind deferred EP-019)
- **Cancelled:** 1 ⚪ (SP-034)
- **Paused:** 0 ⏸

---

*Last Updated: 2026-07-21 (**SP-074 ✅ closed with user approval** — EP-028 `[ScriviCore]`: delivered both
merge C ABI endpoints (`scrivi_merge_scene`/`scrivi_merge_chapter`) + the atomic I-0083 core fix (relocate
scene files BEFORE deleting the emptied chapter); 11 merge tests; ctest **macOS 317/317 + Linux 324/324**;
`scrivi.h` boundary pure C ABI; stale `devops/docker/linux/Dockerfile` libssl-dev gap fixed (user-approved).
I-0083 Resolved at core, Not Verified. **SP-075 `[Apple]` activated same step** — adopt the endpoints on
macOS (T-0302/T-0303), the app-side of the I-0083 fix (AC4). Closed 69→70, Active stays 1 (SP-074→SP-075);
next available **SP-076**. Record: `Closed/Sprint-SP-074.md`. Prior note follows.)*

*2026-07-19 (**SP-073 ✅ closed with user approval** — EP-023 `[Linux]`'s final sprint,
planned + activated + implemented + verified + closed same day (renumbered from the skipped SP-068):
**AC5 chapter drag-reorder** (chapter-as-container, CopyAction-only, boundary drop resolution,
`SceneDocument::moveChapter` block splice, post-reslug path refresh via the new reorder-envelope paths) +
full EP-023 verify AC6–AC8. Three defects found & fixed & Verified en route: **I-0080** (`[ScriviCore]`
open-time migration undid chapter reorders — legacy gate + eager index-cache rebuild), **I-0081** (stale
scene paths after drag broke rename/save — envelope now reports post-move paths), **I-0082** (undraggable
non-selectable chapter rows). New `chapter_reorder_smoke` + extended `scene_reorder_smoke` in CI; ctest
**306/306 macOS + 313/313 Linux**; `scrivi.h` untouched. T-0294–T-0297 Verified & archived to
`Tasks/Verified/Task-verified-0294-0297.md`. **This closed EP-023** (all ACs; `Epics/Closed/Epic-EP-023.md`).
Closed 68→69, Active 1→0 — no active sprint, no Active Epic; next available **SP-074**. Record:
`Closed/Sprint-SP-073.md`. Prior note follows.)*

*2026-07-16 (**SP-067 ✅ closed** with user approval — EP-023 `[Linux]` third sprint: delivered
**AC4 scene drag-reorder** (I-0067/I-0068 fixed via `Qt::CopyAction`, VNC-verified on a fresh project) + **I-0063**
renumber (Verified); the chapter-split defects **I-0064/I-0069/I-0070**, the root slug corruption **I-0072**, and
**I-0071** were **re-homed to the new EP-027** `[ScriviCore]` (rebuilt on its filesystem-authoritative model);
follow-on **I-0073** (VNC drag lag) flagged. Closed 62→63, Active 1→0, next available SP-068. Two Epics now Active
(EP-027 `[ScriviCore]`, EP-023 `[Linux]`); recommended next start EP-027 P1. Earlier: **SP-066 ✅ closed** with user approval — EP-023 `[Linux]` rename scene/chapter
(context menu + `QInputDialog` edit + live label/heading + app-derived "Chapter N" ordinal); tasks
T-0254–T-0257 all Verified over VNC & archived to `Tasks/Verified/Task-verified-0254-0257.md`; delivered EP-023
**AC3** + **I-0062 Resolved-Verified**; all 9 headless smokes green; no ScriviCore work (`scrivi.h` untouched).
Closed 61→62, Active 1→0. **EP-023 stays 🟡 Active** — SP-067 reorder (+ I-0064 chapter-split + I-0063 renumber)
is next. `Closed/Sprint-SP-066.md`. Earlier same day: **SP-065 ✅ closed** — EP-023 `[Linux]` delete
scene/chapter
(context menu + confirmation + `SceneDocument` removal splice + delete-of-active → nearest + focus); tasks
T-0250–T-0253 all Verified over VNC & archived to `Tasks/Verified/Task-verified-0250-0253.md`; delivered
EP-023 **AC1 + AC2**; Docker build + 8 headless smokes green; no ScriviCore work (`scrivi.h` untouched).
Closed count 60→61, Active 1→0. **EP-023 stays 🟡 Active** — SP-066 rename is next. `Closed/Sprint-SP-065.md`.)*
