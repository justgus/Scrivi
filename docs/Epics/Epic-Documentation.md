# Epics - Index

This is the main index for all Scrivi Epics. Epics are strategic milestones that span multiple Sprints and deliver a major product capability.

> **Related:** [Tasks (T)](../Tasks/Task-Documentation.md) | [Issues (I)](../Issues/Issue-Documentation.md) | [Sprints](../Sprints/Sprint-Documentation.md)

## Organization

- **Epic-backlog.md** — Proposed Epics queued for future planning (goal and rough scope only)
- **Epic-active.md** — Epics that are Draft, Active, or Complete-pending-close (full detail)
- **Closed/Epic-EP-XXX.md** — Archived closed Epics, one file each

## Active Epics

Currently: **1 Active Epic + 1 Draft.** **EP-031** `[ScriviCore]` (Worldbuilding Object Model & Relationship
Graph) 🟡 **Active since 2026-08-12** — now an **11-sprint** Epic, **8 closed** (SP-095, SP-096, SP-097,
SP-098, SP-099, SP-103, SP-104, SP-105). **No Sprint is currently active.** Three remain, all 🔵 Planning:
**SP-106** (⚠️ `[Cross]` test integrity & CI trust) runs **first**, then **SP-102** (`[Apple]` pending
presentation + warning view + `sources` card), then **SP-100** (verification & Epic close) last.

⚠️ **AC1 must be RE-VERIFIED — the Epic's one outstanding acceptance criterion.** It was unticked
2026-08-14 when the user's scope ruling (T-0409) moved all ten worldbuilding kinds into the world package;
AC1 had been ticked against the pre-ruling scope table, so its evidence no longer describes the shipped
model. **SP-100 owns the re-verification.** AC9 and AC10 are also outstanding.

Full detail — acceptance criteria, sprint table, task assignments — in [Epic-active.md](Epic-active.md).

**EP-019** `[Apple]` (Custom Undo/Redo History & Multiple Copy Buffers) ✅ **CLOSED 2026-08-11** — AC1–AC8 all
Verified across 7 sprints. AC2 amended (auto-save retired as a commit trigger; 45 s idle boundary added) and
design **§4.d relaxed**, both user-approved as a design change. **EP-030** `[Apple]` (Scene Inspector Card
Framework) ✅ **CLOSED 2026-08-11** — AC1–AC7 all Verified across 4 sprints; AC12 rescoped to soft failures
and delivered by T-0399.
**EP-029** `[Cross]` (Cross-Boundary Structured Cut/Copy/Paste) ✅ **closed 2026-08-03** — all 5 sprints (SP-085
design → SP-086/087/088 ScriviCore → SP-089 Apple wiring) delivered; AC1–AC7 met; manuscript-as-monolithic
copy/cut/paste with structured buffers, cut-that-merges, paste-that-splits, and reversible structured undo
(`Closed/Epic-EP-029.md`). **EP-026** `[Linux]` (Undo/Redo, Menus, Settings & Parity Verification) 🔵 Draft — the
last of the planned `[Linux]` family, sequenced after the Apple copy-buffer reference. **EP-025** `[Linux]` ✅
**closed 2026-07-24**.

See: [Epic-active.md](Epic-active.md)

## Backlog Epics

Currently: **3 Backlog Epics** —
- **EP-026** `[Linux]` (Undo/Redo, Menus, Settings & Parity Verification), 🔵 Draft — the last of the planned
  `[Linux]` family. (EP-023 ✅ closed 2026-07-19; EP-024 ✅ closed 2026-07-22; EP-025 Timeline Panel ✅ closed
  2026-07-24.)
- **EP-032** `[Cross]` (Inline Object References in the Manuscript — footnotes / pull quotes), 🔵 Proposed,
  opened 2026-08-12 from an EP-031 design ruling. Depends on EP-031 + EP-029.
- **EP-033** `[Cross]` (World Lifecycle Management — in-app view vs. dedicated application), 🔵 Proposed,
  opened 2026-08-14 from the I-0118 Q1 ruling. Its first deliverable is the product fork itself.

See: [Epic-backlog.md](Epic-backlog.md)

## All Epics

Currently: **33 Epics** | Next available: **EP-034**
Codebase tags (`[ScriviCore]`/`[Apple]`/`[Linux]`/`[Windows]`/`[Cross]`) per `Epic-GUIDELINES.md`.
Existing EP-001–EP-018 are all `[Apple]`/`[ScriviCore]` era work (pre-convention); tagged going forward.

| Epic   | Title | Status | Start | Close |
| ------ | ----- | ------ | ----- | ----- |
| EP-001 | ScriviCore Foundation | ✅ Closed | 2026-05-19 | 2026-05-20 |
| EP-002 | ScriviCore Services | ✅ Closed | 2026-05-20 | 2026-05-26 |
| EP-003 | Identity and First Launch | ✅ Closed | 2026-05-20 | 2026-05-26 |
| EP-004 | Repair and Recovery | ✅ Closed | 2026-05-26 | 2026-05-27 |
| EP-005 | Full Project Package — Objects, Assets, and Comments | ✅ Closed | 2026-05-27 | 2026-05-28 |
| EP-006 | Swift Interop and Apple Shell | ✅ Closed | 2026-05-28 | 2026-05-30 |
| EP-007 | Skeleton v0.2 Migration | ✅ Closed | 2026-05-22 | 2026-05-26 |
| EP-008 | Multi-Scene Navigation and Cross-Platform Build | ✅ Closed | 2026-05-30 | 2026-06-01 |
| EP-009 | Writing Surface and Scene Navigator | ✅ Closed | 2026-06-01 | 2026-06-03 |
| EP-010 | Manuscript Structure Editing — Delete, Reorder, and Title | ✅ Closed | 2026-06-04 | 2026-06-06 |
| EP-011 | Writing Surface Behavior — Scroll, Cursor, and Focus | ✅ Closed | 2026-06-08 | 2026-06-08 |
| EP-012 | Menu Bar and Toolbar Replacement | ✅ Closed | 2026-06-09 | 2026-06-29 |
| EP-013 | Scene Navigator Swipe Gesture | ✅ Closed (no delivery) | 2026-06-09 | 2026-06-10 |
| EP-014 | Scene Inspector Panel | ✅ Closed | 2026-06-09 | 2026-06-10 |
| EP-015 | Real-Time Timeline Panel | ✅ Closed | 2026-06-09 | 2026-06-10 |
| EP-016 | Timeline Panel — Full Implementation | ✅ Closed | 2026-06-11 | 2026-06-23 |
| EP-017 | Spotlight Search Integration | ✅ Closed | 2026-06-23 | 2026-07-06 |
| EP-018 | Per-Window / Per-Project Window Model | ✅ Closed | 2026-06-24 | 2026-06-25 |
| EP-019 | `[Apple]` Custom Undo/Redo History & Multiple Copy Buffers | ✅ Closed | 2026-07-06 | 2026-08-11 |
| EP-020 | `[Linux]` App Foundation — Qt/QML Toolchain, Bridge & Shell | ✅ Closed | 2026-07-13 | 2026-07-13 |
| EP-021 | `[Linux]` Project Lifecycle & Landing | ✅ Closed | 2026-07-13 | 2026-07-14 |
| EP-022 | `[Linux]` Writing Surface & Scene Navigator | ✅ Closed | 2026-07-14 | 2026-07-15 |
| EP-023 | `[Linux]` Manuscript Structure Editing | ✅ Closed | 2026-07-15 | 2026-07-19 |
| EP-024 | `[Linux]` Scene Inspector Panel | ✅ Closed | 2026-07-22 | 2026-07-22 |
| EP-025 | `[Linux]` Timeline Panel | ✅ Closed | 2026-07-22 | 2026-07-24 |
| EP-026 | `[Linux]` Undo/Redo, Menus, Settings & Parity Verification | 🔵 Draft (backlog) | — | — |
| EP-027 | `[ScriviCore]` Filesystem-Authoritative Chapter/Scene Identity & Ordering | ✅ Closed | 2026-07-16 | 2026-07-18 |
| EP-028 | `[Cross]` Scene & Chapter Merging — Linux Parity & Filesystem-Coherence Fix | ✅ Closed | 2026-07-20 | 2026-07-22 |
| EP-029 | `[Cross]` Cross-Boundary Structured Cut / Copy / Paste | ✅ Closed | 2026-07-27 | 2026-08-03 |
| EP-030 | `[Apple]` Scene Inspector Card Framework (base cards) | ✅ Closed | 2026-08-05 | 2026-08-11 |
| EP-031 | `[ScriviCore]` Worldbuilding Object Model & Relationship Graph | 🟡 Active | 2026-08-12 | — |
| EP-032 | `[Cross]` Inline Object References in the Manuscript (footnotes / pull quotes) | 🔵 Proposed (backlog) | — | — |
| EP-033 | `[Cross]` World Lifecycle Management — in-app view vs. dedicated application | 🔵 Proposed (backlog) | — | — |

## Statistics

- **Total Epics:** 33 (EP-001–EP-033)
- **Closed:** 29 ✅ (each has an archive in [`Closed/`](Closed/); most recent: EP-019 + EP-030, both 2026-08-11)
- **Complete (pending close):** 0 🟠
- **Active:** 1 🟡 (EP-031 `[ScriviCore]` — Worldbuilding Object Model; 8 of 11 sprints closed, **no Sprint
  currently active**; SP-106 → SP-102 → SP-100 remain, all 🔵 Planning)
- **Planned:** 0 🔵
- **Draft:** 1 🔵 (EP-026 `[Linux]`)
- **Deferred (backlog):** 0
- **Proposed (backlog):** 2 🔵 (EP-032 `[Cross]` — Inline Object References, opened 2026-08-12; EP-033
  `[Cross]` — World Lifecycle Management, opened 2026-08-14 from the I-0118 ruling)

---

*Last Updated: 2026-08-16 (**Consistency audit — this index was three weeks stale and contradicted its own
table.** Corrected: the Active-Epics section still described EP-031 as "six sprints SP-095–SP-100" with
**SP-095 "awaiting activation"** — SP-095 closed 2026-08-12 and the Epic is now a **10-sprint** Epic with 8
closed and none active; **"Total Epics: 32" vs "Currently: 33 Epics"** in the same file (33 is correct — the
table has 33 rows); **"Closed: 28"** (29 is correct, matching 29 archives in `Closed/`); and **"1 Backlog
Epic"** (3 — EP-026, EP-032, EP-033). The stale EP-019 "resuming at SP-056" note was dropped — EP-019 closed
2026-08-11. ⚠️ **Every one of these was a count or status that drifted while the table beneath it stayed
correct**, which is the argument for deriving these lines from the table rather than restating them.
Prior note follows.)*

*2026-07-27 (**SP-056 ✅ closed — EP-019 AC6 Verified; EP-019 held pending SP-057; EP-029 opened.**
EP-019 SP-056 delivered multiple copy buffers (T-0213 + T-0214, both Verified live): explicit ⌘1–9/⌃1–9/⌥1–9
copy/paste/cut chords, app-global per-project palette, Edit/Scene/Chapter menu items, bufferID-tagged cut event
(backend schema extended); ctest 327 + interop 43 green. EP-019 is now **held pending** its final sprint SP-057
(AC2/AC7/AC8 verify + history panel + Epic close). New Epic **EP-029** `[Cross]` (Cross-Boundary Structured
Cut/Copy/Paste) opened 🔵 Draft — treat the manuscript as one monolithic document for Cut/Copy/Paste + buffers
(structured fragments, cut-that-merges, paste-that-splits); 5 sprints planned (SP-085 design → SP-089 Apple
wiring), design doc first. Total Epics 28→29, Draft 1→2; next available Epic **EP-030**. Prior note follows.)*

*2026-07-24 (**EP-019 `[Apple]` Undo/Redo & Copy Buffers ✅ un-deferred → Active (user request).**
Moved back from the backlog to lock the copy-buffer implementation on Apple before the Linux side (EP-026)
tackles it, so Apple is the reference. The undo/redo engine + AC1/AC3/AC4/AC5 are delivered & verified (SP-051–
SP-055 closed); resuming at **SP-056** (multiple copy buffers → AC6: 9-slot per-project persistent buffers in
`history/buffers.json`, keyboard HUD + palette + Edit-menu UX per Trades T3/T4, copy≠event/cut=event). Then
SP-057 (AC2/AC7/AC8 verify + Epic close). Epics Active 0→1, Deferred 1→0. Same day: **EP-025 `[Linux]` ✅ CLOSED**
(below). Prior note follows.)*

*2026-07-24 (**EP-025 `[Linux]` Timeline Panel ✅ CLOSED with user approval** — the fifth
`[Linux]` family Epic and the largest since Apple EP-016, delivered in six sprints (SP-079/080/081/083/082/084;
SP-083 brought forward ahead of SP-082). Full Apple EP-015/EP-016 parity rebuilt natively in Qt/C++ over the
complete EP-016 timeline C ABI (`scrivi.h` untouched throughout): bottom timeline strip with story-time scene
dots + drag + Time Delta Picker + chain propagation (AC1–AC3); story-structure bands (AC4); zoom/pan (AC6a);
historical events + imported timelines + export (AC5); and co-located-dot clustering — aggregate dot + hover
fan-out + zoom-resolve (AC6b). All ACs Verified live over Docker+VNC. Issues closed: I-0087/I-0088/I-0089/
I-0090/I-0091. Tests: `timeline_story_time_smoke`, `story_structure_smoke`, `timeline_events_smoke`,
`timeline_cluster_smoke`. Archived to `Closed/Epic-EP-025.md`. Epics Closed 23→24, Active 1→0 — **no Active
Epics**; next in line **EP-026 `[Linux]`** (Draft). Prior note follows.)*

*2026-07-22 (**EP-025 `[Linux]` Timeline Panel activated** — the next `[Linux]` family Epic and
the largest since Apple EP-016: a hideable bottom timeline strip with scene dots in story-time order, drag +
Time Delta Picker, story-structure bands, historical/imported events, export, and clustering — full EP-016
parity (user decision), rebuilt in Qt/C++. The full timeline C ABI already exists (EP-016/SP-039), so this is
a pure `[Linux]` UI Epic like EP-022/EP-023. 5 sprints sequenced (SP-079–SP-083); **SP-079** active (panel
scaffold + scene dots + show/hide + dot↔navigator selection, T-0321–T-0324). Panel docks as a bottom strip
(user decision). Epics Active 0→1, Draft 2→1, Backlog 3→2. Prior note follows.)*

*2026-07-22 (**EP-024 `[Linux]` Scene Inspector Panel ✅ closed with user approval** — the Linux
mirror of Apple EP-014, planned + activated + implemented + verified + closed same day: a hideable right-side
`SceneInspector` (`QTabWidget`, stub "Scene Entities" tab) docked as `EditorShell`'s third splitter pane + a
View ▸ Show Inspector toggle at Ctrl+Alt+I on the SP-077 menu bar; session-scoped, default shown, 120/200 width
(user pref); window enlarged to 1020×760. All 6 ACs Verified live over VNC (SP-078, T-0318–T-0320); container
build green (184/184, 0 warnings) + all Linux smokes PASS. Qt/C++ UI only — no `scrivi_*`/`scrivi.h`/Apple
change. Archived to `Closed/Epic-EP-024.md`. Epics Closed 23→24, Active 1→0 — **no Active Epics**; next in
line EP-025 `[Linux]` (Timeline Panel, Draft). Prior note follows.)*

*2026-07-22 (**EP-028 `[Cross]` Scene & Chapter Merging ✅ closed with user approval** —
delivered keyboard scene/chapter merge on macOS + Linux backed by two new atomic ScriviCore endpoints
(`scrivi_merge_scene` / `scrivi_merge_chapter`), and fixed the EP-027 chapter-merge data-loss regression
(I-0083) at the core (relocate scene files across folders BEFORE removing the emptied chapter). 4 sprints:
SP-074 `[ScriviCore]`, SP-075 `[Apple]`, SP-076 `[Linux]` parity, SP-077 `[Linux]` native menu bar (added
to make chapter-merge verifiable over VNC — macOS→VNC swallows `Ctrl-Shift-Backspace`; the menu also closed
a data-loss hole in the editor-leaving paths). AC1–AC7 all Verified; I-0083/I-0084 Verified, I-0085 Not-a-Bug,
I-0086 fixed. Archived to `Closed/Epic-EP-028.md`. Closed 22→23, Active 1→0 — **no Active Epics**; next in
line EP-024–EP-026 `[Linux]` (Draft). Prior note follows.)*

*2026-07-19 (**EP-023 `[Linux]` ✅ closed with user approval** — full manuscript-structure
editing from the navigator: create/delete/rename + scene & chapter drag-reorder, all persisting on the
EP-027 on-disk model; ACs AC1–AC8 verified over Docker+VNC; SP-073 (final sprint, renumbered from the
skipped SP-068) fixed & Verified I-0080 (core migration undid reorders), I-0081 (stale scene paths after
drag), I-0082 (undraggable chapter rows); `scrivi.h` untouched for the whole Epic; ctest 306/306 macOS +
313/313 Linux; archived to `Closed/Epic-EP-023.md`. Closed 21→22, Active 1→0 — **no Active Epics**; next
in line EP-024–EP-026 `[Linux]` (Draft). Prior note follows.)*

*2026-07-15 (EP-023 `[Linux]` Manuscript Structure Editing — **SP-065 (delete) ✅ closed** with user approval: AC1 + AC2 verified over VNC, tasks T-0250–T-0253 archived; 1 of 4 sprints done, EP-023 stays 🟡 Active (SP-066 rename next). Epic was activated same day (first sprint SP-065); Active count 0→1, Draft 4→3, Backlog 5→4. Delete → rename → scene drag-reorder → chapter drag-reorder + close across 4 sprints; no ScriviCore work (all eight structure endpoints exist); chapter-title toggle deferred to EP-026; reorder via QTreeView drag. Earlier: EP-022 `[Linux]` Writing Surface & Scene Navigator ✅ closed with user approval — the core Linux writing loop (navigator + continuous editable viewport + per-scene auto-save + in-editor scene/chapter creation + scroll-driven switching + cursor/focus + quit→reopen surface restore); all 4 sprints SP-061–SP-064 closed, AC1–AC7 verified over Docker+VNC, `scrivi.h` untouched. Archived to `Closed/Epic-EP-022.md`. No Epic currently Active; EP-023 `[Linux]` structure editing is next. EP-023–EP-026 remain 🔵 Draft.)*
