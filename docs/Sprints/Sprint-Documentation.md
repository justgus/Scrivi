# Sprints - Index

This is the main index for all Scrivi Sprints. Sprints are fixed-duration iterations that group Tasks and Issues into focused units of work.

> **Related:** [Tasks (T)](../Tasks/Task-Documentation.md) | [Issues (I)](../Issues/Issue-Documentation.md) | [Epics (EP)](../Epics/Epic-Documentation.md)

## Organization

- **Sprint-active.md** — The currently active Sprint(s); normally one, but parallel sprints across different Epics are permitted
- **Closed/Sprint-SP-XXX.md** — Archived closed Sprints, one file each

## Active Sprints

- **SP-070** (EP-027 `[ScriviCore]` — **P6 filesystem-authoritative scene identity & ordering**) —
  🟢 Implemented, Not Verified, activated 2026-07-17. `SceneRef` filename-only (identity derived by sceneID
  sidecar scan); order-key scene filenames; cross-chapter reorder relocates files; chapter rename touches zero
  scene fields; scene migration/orphan-repair on open (before validation). Tasks **T-0271–T-0277**. `scrivi.h`
  untouched. `Sprint-active.md`.
- **SP-069** (EP-027 `[ScriviCore]` — **P1 rename primitive + P2 order-key/disk-authority + P3 migration**) —
  🟢 Implemented, Not Verified, activated 2026-07-16. **P1:** `FileSystem::renamePath` (atomic-within-fs, never
  clobbers, missing-source guard). **P2:** `util/OrderKey` fractional slugs + disk-authoritative chapter
  identity/order (`ChapterIndex`) + order-key `ChapterCreator`/`ChapterReorderer` (**fixes I-0072**) + open-time
  self-heal. **P3:** `migrateChapterOrderKeys` — lazy/idempotent migration of legacy `chapter-NNN` projects at
  open (dual-scheme read; collision-free letter-prefixed keys). **ctest green 290/290** macOS + Linux container
  (all EP-027 tests, 3225 assertions). Tasks **T-0264–T-0270**. `scrivi.h` untouched. `Sprint-active.md`.

Last closed: **SP-072** (EP-027 — **P5 Apple-verify migration fixes**; a real legacy project failed to open on
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
| SP-056 | Copy buffers — store, ABI, HUD/palette UX, history integration | EP-019 | 🔵 Planning |
| SP-057 | Undo/Redo — history panel, perf fixtures, verification & Epic close | EP-019 | 🔵 Planning |

## All Sprints

Currently: **71 Sprints** (66 closed + 1 cancelled + 2 in Planning + 0 active; SP-068 skipped — its EP-023
chapter-drag scope folded into EP-027) | Next available: **SP-073**

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

## Statistics

- **Total Sprints:** 68
- **Closed:** 63 ✅
- **Active:** 1 🟢 (SP-069 EP-027 `[ScriviCore]` P1–P3 — rename primitive + order-key/disk-authority + migration)
- **Planning:** 2 🔵 (SP-056–SP-057 EP-019 `[Apple]` — parked behind deferred EP-019)
- **Cancelled:** 1 ⚪ (SP-034)
- **Paused:** 0 ⏸

---

*Last Updated: 2026-07-16 (**SP-067 ✅ closed** with user approval — EP-023 `[Linux]` third sprint: delivered
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
