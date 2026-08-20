# Epics - Index

This is the main index for all Scrivi Epics. Epics are strategic milestones that span multiple Sprints and deliver a major product capability.

> **Related:** [Tasks (T)](../Tasks/Task-Documentation.md) | [Issues (I)](../Issues/Issue-Documentation.md) | [Sprints](../Sprints/Sprint-Documentation.md)

## Organization

- **Epic-backlog.md** — Proposed Epics queued for future planning (goal and rough scope only)
- **Epic-active.md** — Epics that are Draft, Active, or Complete-pending-close (full detail)
- **Closed/Epic-EP-XXX.md** — Archived closed Epics, one file each

## Active Epics

🟡 **EP-034** `[Cross]` **Object Detail & Media — Detail Sheets & Project↔World Interaction** — **ACTIVE
2026-08-20**, promoted from the backlog and ⚠️ **widened in place** from "object fields beyond
`displayName`" to the full Detail Sheet, relationship surface, and Project↔World interaction.

**Design doc** `Scrivi_Object_Detail_Sheets_Design_v0_1.md` is 🟢 **APPROVED FOR PLANNING** — ✅ **all nine
trades D1–D9 and all three decisions Q-a/Q-b/Q-c ruled.** AC1–AC12 written; 8 sprints estimated;
**SP-115 ✅ CLOSED 2026-08-20** (user-approved) — 1 of 8 sprints; seven Tasks + six Issues Verified. **SP-116 next.**

⚠️ **Q-a: EP-034 precedes EP-032**, whose **SP-107–SP-114 stay reserved** — so **sprint IDs run out of
sequence** (precedented: SP-083 before SP-082, SP-102 before SP-100). ⚠️ **Q-b: the Detail Sheet is
ADDITIVE** — the inline editor and list item remain, so **I-0139 is a real fix, not a disposition.**
⚠️ **Q-c: iOS/visionOS explicitly deferred**; `[Linux]` parity stays in scope.

> ⚠️ **The finding that reshaped it:** the object-editing backend **and its Swift binding already exist** —
> `scrivi_save_object` takes full object JSON, every edge endpoint ships, and `ScriviEngine` wraps them all,
> while the app exposes a single `TextField("Name")`. **`project_capability_without_surface` at its largest
> scale yet.** The lesson, per the user: *"Ensure all core features have a UI the writer can reach!"*

**Previously active:** **EP-031** `[ScriviCore]` ✅ **CLOSED 2026-08-19** (user-approved) →
[`Closed/Epic-EP-031.md`](Closed/Epic-EP-031.md) — 11 sprints, all 10 ACs verified. It filed
**I-0135–I-0139** and fixed none by design; ⚠️ **all five are now assigned to EP-034's SP-115.**
**EP-019** and **EP-030** `[Apple]` ✅ closed 2026-08-11; **EP-029** `[Cross]` ✅ closed 2026-08-03.

See: [Epic-active.md](Epic-active.md)

## Backlog Epics

Currently: **3 Backlog Epics** —
- **EP-026** `[Linux]` (Undo/Redo, Menus, Settings & Parity Verification), 🔵 Draft — the last of the planned
  `[Linux]` family. (EP-023 ✅ closed 2026-07-19; EP-024 ✅ closed 2026-07-22; EP-025 Timeline Panel ✅ closed
  2026-07-24.)
- **EP-032** `[Cross]` (Inline Object References in the Manuscript — footnotes / pull quotes), 🔵 Draft,
  opened 2026-08-12; **promoted and returned 2026-08-20**. ⚠️ **Retains a full planning pass** (AC1–AC10,
  Q1–Q6) and ⚠️ **holds RESERVED sprint IDs SP-107–SP-114**. Owes a **Q5 export ruling** before promotion.
- **EP-033** `[Cross]` (World Lifecycle Management — in-app view vs. dedicated application), 🔵 Proposed,
  opened 2026-08-14 from the I-0118 Q1 ruling. Its first deliverable is the product fork itself.

See: [Epic-backlog.md](Epic-backlog.md)

## All Epics

⚠️ **This table is the single source for Epic status (P7).** Counts are not restated here or in
Statistics — read the rows. Next available ID: **EP-035**.
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
| EP-031 | `[ScriviCore]` Worldbuilding Object Model & Relationship Graph | ✅ Closed | 2026-08-12 | 2026-08-19 |
| EP-032 | `[Cross]` Inline Object References in the Manuscript (footnotes / pull quotes); ⚠️ **holds reserved SP-107–SP-114** | 🔵 Draft (backlog) | — | — |
| EP-033 | `[Cross]` World Lifecycle Management — in-app view vs. dedicated application | 🔵 Proposed (backlog) | — | — |
| EP-034 | `[Cross]` Object Detail & Media — **Detail Sheets & Project↔World Interaction** (widened 2026-08-20) | 🟡 **Active** | 2026-08-20 | — |

## Statistics

⚠️ **The All-Epics table above is the SINGLE SOURCE for per-Epic status and counts (P7).** This section
carries **only what the table cannot express** — per-status totals were removed 2026-08-19 under audit
ruling **R-23(②)**. **To count Epics by status, read the table.**

- **Next available Epic ID:** **EP-035**
- **Total Epic IDs issued:** 34 (EP-001–EP-034); none cancelled, skipped or superseded
- **Where per-Epic detail lives:** [`Epic-active.md`](Epic-active.md) is authoritative for the **active**
  Epic's acceptance criteria and sprint table. ⚠️ **This index does not restate AC state** — that is what
  produced the *"AC9 and AC10 are also outstanding"* error, when AC9 had been met two days earlier.

> ⚠️ **This file is where the correct diagnosis was FIRST written down** — *"every one of these was a
> count or status that drifted while the table beneath it stayed correct, which is the argument for
> deriving these lines from the table rather than restating them"* — and then **not acted on**. It has now
> been acted on. **Do not reintroduce per-status counts here.**

---

*Last Updated: 2026-08-20 (**EP-034: SP-115 ✅ CLOSED, user-approved** — 1 of 8 sprints. All five carried
EP-031 Issues fixed, plus **I-0142 found by the user during verification**. ⚠️ **Two items carried out**:
T-0420's writer-facing surface (**unowned**) and I-0140/I-0141 (**SP-116**). EP-034 stays 🟡 Active. Prior
note follows.)*

*Last Updated: 2026-08-20 (**SP-115 🟡 ACTIVATED** — EP-034's first sprint; six Tasks **T-0419–T-0424**
created and Active, all five open Issues **I-0135–I-0139** assigned one Task each. ⚠️ Scope is those five
**and nothing else**; the Detail Sheet begins **SP-116**. ⚠️ **T-0424 FILES two kind-list findings
(I-0140, I-0141) rather than fixing them** — cured by D5 in SP-116. Next available: Sprint **SP-116**,
Task **T-0425**, Issue **I-0142** (after T-0424 files two). Prior note follows.)*

*2026-08-20 (**EP-034 `[Cross]` PROMOTED backlog → 🟡 ACTIVE and widened in place** — the
Detail Sheet, relationship surface and Project↔World interaction. Design doc 🟢 **APPROVED FOR PLANNING**:
✅ **nine trades D1–D9 + three decisions Q-a/Q-b/Q-c all ruled.** AC1–AC12 written; **SP-115 🔵 Planning**
(the five Issues, nothing else). ⚠️ **Q-a: EP-034 precedes EP-032** — its SP-107–SP-114 stay reserved and
**IDs run out of sequence.** ⚠️ **Q-b: additive** — inline editor stays, **I-0139 is a real fix.**
⚠️ **Q-c: iOS/visionOS deferred**, `[Linux]` parity in scope. Backlog Epics 4 → 3; Active 0 → 1; next
available Epic **EP-035**. Prior note follows.)*

*Last Updated: 2026-08-20 (**EP-032 RETURNED to the Epic backlog by user ruling** — promoted and returned
the same day; **no Epic is active.** Its full planning pass (AC1–AC10, Q1–Q6, the code-verified claim table,
the Q5 export gap) moved with it to `Epic-backlog.md` and was **not discarded**. ⚠️ **Sprint IDs
SP-107–SP-114 remain RESERVED to EP-032** — next available Sprint for other work is **SP-115**, recorded in
both Sprint files. Backlog Epics 3 → 4; next available Epic **EP-035**. Prior note follows.)*

*2026-08-20 (**EP-032 `[Cross]` Inline Object References PROMOTED backlog → 🔵 Draft and is
now the ACTIVE Epic** — EP-031 having closed 2026-08-19. The Active-Epics section above was REWRITTEN: it
still carried EP-031's full body under a "No Epic is currently active" heading, which read as though the
closed Epic were still live. That body survives intact at `Closed/Epic-EP-031.md` (769 lines); its Issues
I-0135–I-0139 are in `../Issues/Issue-active.md` and T-0416 in `../Tasks/Task-backlog.md`. EP-032 carries
**AC1–AC10** and **Q1–Q6** for its design sprint **SP-107** to rule. ⚠️ **Two promotion findings:** the
`source`→`scene` "additive" claim **CONFIRMED** against `RelationTypes.hpp:40-43`, and the **"export
behaviour"** scope item **has NO existing path** — `scrivi_export_project_timeline` is the only export
endpoint in the C ABI and it exports the timeline, not the manuscript (now **Q5**). Backlog Epics 4 → 3;
next available Epic **EP-035**. Prior note follows.)*

*2026-08-17 (**EP-031: SP-106 ✅ closed (user-approved), SP-102 activated — 9 of 11 sprints
closed.** SP-106's 3 Tasks and both Issues (I-0121, I-0122) all ✅ Verified and archived. **SP-102 is now the
active Sprint**, delivering **AC23 + AC24** — the last two clauses of AC9; **SP-100 runs last** and still owns
the **AC1 re-verification**. This file said "no Sprint is currently active" and listed SP-106 → SP-102 →
SP-100 as all 🔵 Planning; both statements are corrected above. **EP-031 remains 🟡 Active** — no Epic state
changed. Prior note follows.)*

*2026-08-16 (**Consistency audit — this index was three weeks stale and contradicted its own
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
