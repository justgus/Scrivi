# Sprints - Index

This is the main index for all Scrivi Sprints. Sprints are fixed-duration iterations that group Tasks and Issues into focused units of work.

> **Related:** [Tasks (T)](../Tasks/Task-Documentation.md) | [Issues (I)](../Issues/Issue-Documentation.md) | [Epics (EP)](../Epics/Epic-Documentation.md)

## Organization

- **Sprint-active.md** — The currently active Sprint(s); normally one, but parallel sprints across different Epics are permitted
- **Closed/Sprint-SP-XXX.md** — Archived closed Sprints, one file each

## Active Sprints

**No Sprint is currently active.** **SP-100 closed 2026-08-19 (user-approved)** →
[`Closed/Sprint-SP-100.md`](Closed/Sprint-SP-100.md) — ⚠️ **and its close closed EP-031**, whose
**11 sprints are all closed**. Each closed Sprint's record lives in its own archive file and is **not**
restated here.

🟡 **EP-034** `[Cross]` is the active Epic. **No Sprint is active** — **SP-116 ✅ CLOSED 2026-08-21**
(user-approved), all eight Tasks and six Issues Verified → [`Closed/Sprint-SP-116.md`](Closed/Sprint-SP-116.md).
**SP-118 ✅ CLOSED 2026-08-23** (user-approved) → [`Closed/Sprint-SP-118.md`](Closed/Sprint-SP-118.md); five Tasks + **thirteen Issues** Verified, closing **AC5, AC6 and AC7**. ⚠️ **All thirteen Issues came from the live click-through and none from any suite.** **No Sprint is active or planned.** **SP-117 ✅ CLOSED 2026-08-21** (user-approved) → [`Closed/Sprint-SP-117.md`](Closed/Sprint-SP-117.md). **SP-115 ✅ CLOSED 2026-08-20** (user-approved), all seven
Tasks and six Issues Verified. ⚠️ **SP-107–SP-114 stay RESERVED to EP-032**, which now runs AFTER EP-034
(Q-a), so ⚠️ **IDs execute OUT OF SEQUENCE**. **Next available ID is in Statistics** — recorded once,
there, not restated here (P7).

> ✅ **SP-102 delivered AC23 + AC24**, the last two clauses of AC9. ⚠️ **T-0365 closed as a PARTIAL
> delivery** — the `sources` card renders only its empty state because nothing in the app creates a
> `source`; creation is owed to **EP-034 `[Cross]` Object Detail & Media**, opened from that finding.

> ✅ **SP-106 delivered the evidence base SP-100 needs.** ScriviCore CI is green on **x86-64** for the first
> time since 2026-07-30, a 2×2 sanitizer matrix makes UB fail by diagnosis rather than by architecture
> accident, and every CI leg names its architecture. ⚠️ **The sanitizer found a second, unknown defect
> (I-0122) on its first run** — so what else was hidden behind six sprints of single-architecture evidence is
> an open empirical question for SP-100.

> 🟡 **SP-100 ACTIVATED 2026-08-19 (user-approved).** Three Tasks
> in a load-bearing order: **T-0390** (repair-matrix world conditions) → **T-0418** (live-use pass on the
> real USB rig, **new at planning**) → **T-0391** (the AC1–AC10 pass + close prep, which reads the other
> two as evidence and cannot start early).
>
> ⚠️ **Planning found that AC1 and AC10 could not be re-verified as written.** Each carries a clause the
> Epic's own §3.0 **no-migration** ruling superseded — *"legacy 5-kind files load unchanged"* and
> *"existing projects open unchanged"* — so verifying either would have meant asserting a behaviour EP-031
> deliberately chose **not** to build. **Both are amended (R1/R2, user-approved) and both remain UNTICKED.**
> **AC10's amended form is stronger**, requiring both architectures **and** sanitizers — which is exactly
> the evidence base SP-106 restored, and answers the empirical question above rather than deferring it.
>
> ⚠️ **The External Change Repair Matrix has ZERO world coverage** — 578 lines, 21 conditions, and not one
> mention of `world` or `scrivworld`, because worlds landed in SP-097 three sprints after its last
> revision. **A repair pass written against the current document could destroy a writer's entire
> relationship graph and satisfy every rule in it.** That is T-0390's subject.
>
> ⚠️ **The live pass (R4) is required evidence, not polish.** Four of EP-031's eleven sprints were
> unplanned and **every one came from USE**; SP-102's live runs alone produced eight Issues, one of which
> (**I-0129**) *was* the AC23 clause and was invisible to every fixture.

Epic-level status — acceptance criteria, the AC1 re-verification requirement, the Linux gap — belongs to
EP-031 in [`../Epics/Epic-active.md`](../Epics/Epic-active.md) and is not duplicated here.

**EP-019 and EP-030 both ✅ CLOSED 2026-08-11** (Human-approved, double Epic close) — sprints SP-092, SP-093,
SP-094, SP-101 archived. **EP-029** `[Cross]` ✅ fully closed 2026-08-03. *(Next-available Sprint ID is recorded in Statistics — **SP-107**.)*

Prior: **SP-089** (EP-029 `[Cross]` — **`[Apple]` editor wiring**) — ✅ **closed 2026-08-03
(Human-approved).** The final EP-029 sprint, which **closed the Epic**. Wired the manuscript editor's
Cut/Copy/Paste + copy buffers through the ScriviCore fragment ops so the manuscript behaves as one monolithic
document: **T-0354** (Pass A system-clipboard ⌘C/⌘X/⌘V, Verified 2026-07-29), **T-0357** (title-capture + chapter
promotion, 2026-07-29), **T-0355** (Pass B structured copy buffers, 2026-08-03), **T-0356** (AC6 reversible
structured undo — app-runs-inverse, new core endpoint `scrivi_fragment_uncut_paste`, 2026-08-03). AC1–AC7 all
met. ScriviCore `ctest` **369/369**, macOS interop **45/45**, app **BUILD SUCCEEDED**.
`Closed/Sprint-SP-089.md`; EP-029 → `../Epics/Closed/Epic-EP-029.md`. **Next available Sprint SP-090.**

Prior: **SP-056** (EP-019 `[Apple]` — **Multiple copy buffers**) — ✅ **closed 2026-07-27
(Human-approved).** Delivered **AC6**: vim/emacs-register-style copy buffers, refined to **three explicit
chords** (user 2026-07-25/27) — **⌘1–9 copy · ⌃1–9 paste · ⌥1–9 cut** into 9 per-project persistent slots
(`history/buffers.json`), buffer 0 = the untouched system pasteboard; an app-global Copy Buffers palette
(View ▸ Show Buffers / ⌥⌘B, reloads to the frontmost project, modifier-sensitive action button + clear);
Edit-menu Copy/Paste/Cut To Buffer + Scene/Chapter New/Merge menu items. Trade T3: copy = no event, paste =
`paste` event, cut = `cut` event **tagged with bufferID** (backend `scrivi_history_record_event` + history-node
schema extended). T-0213 + T-0214 both Verified live; ctest **327** + macOS interop **43** green. A cross-boundary
Cut/Copy/Paste gap surfaced during verify → new **EP-029**. ✅ `Closed/Sprint-SP-056.md` **archived 2026-08-05**
(reconstructed from the EP-019 record — the "to be archived" note above was never actioned at close time).

Prior: **SP-084** (EP-025 `[Linux]` — **Clustering + persistence verify + full verify & Epic close**) — ✅
**closed 2026-07-24 (Human-approved).** Co-located project-row dots collapse into a single **aggregate dot**
(larger core + count + segmented arc ring + selection arc, Apple T-0174); **hover fans** the members into the
hexagonal ring; **zoom resolves** the cluster. Then a persistence sweep + full EP-025 verify. **AC6b Verified**
live over VNC; four clustering findings fixed + re-verified same session (incl. a phantom double-draw defect).
Container green (216/216) + new `timeline_cluster_smoke` + 14 regression smokes PASS; `scrivi.h` untouched.
Tasks T-0346–T-0349 all Verified. **This closed EP-025** (all ACs AC1–AC6b; `../Epics/Closed/Epic-EP-025.md`).
`Closed/Sprint-SP-084.md`.

Prior: **SP-082** (EP-025 `[Linux]` — **Historical events + imported timelines + export**, 4th sprint) —
✅ **closed 2026-07-24 (Human-approved).** A writer authors/edits/deletes **historical events** (own `#C8A97A`
draggable dots + context menus); **imports** external `.scrivi-timeline.json` files as **distinct grey rows
below** the project row (per-source shade, window-clipped, `EpochOffsetDialog` on import, hide/show, edit-offset,
remove); **exports** the project timeline; and File ▸ Import/Export Timeline… menu items (T-0345). **AC5
Verified** live over VNC; two findings fixed + re-verified same day (I-0090 imported row behind the scrollbar,
I-0091 dialogs defaulted to `/root`). Container green (211/211) + new `timeline_events_smoke` + 11 regression
smokes PASS; `scrivi.h` untouched. Tasks T-0340–T-0345 all Verified. `Closed/Sprint-SP-082.md`.

Prior: **SP-081 + SP-083** (EP-025 `[Linux]` — **Timeline Panel**, 3rd sprint + brought-forward zoom/pan)
— ✅ **both closed 2026-07-23 (Human-approved).** **SP-081** delivered story-structure bands (structure selector
→ colored proportional bands + labels; border-drag re-proportion; scene→band drag-up / "Assign to Act…"
assignment with a colored ring; remove keeps offsets + assignments) — **AC4**; T-0329–T-0332 all Verified;
`story_structure_smoke` PASS (caught the `{"bands":[…]}` shape); I-0089 mode-latch fixed. **SP-083** (brought
forward to fix **I-0087** dot-crowding) delivered zoom (`Ctrl`+wheel zoom-about-pointer + `+`/`−` control) + pan
(background drag), keeping the linear axis — **AC6a**; T-0333–T-0339 all Verified (+ follow-ons: any-scene
anchor, storyline-wrapped bands, per-project zoom/pan persistence, elided anchor titles); closed I-0087/I-0088.
`Closed/Sprint-SP-081.md`, `Closed/Sprint-SP-083.md`.

Prior: **SP-080** (EP-025 `[Linux]` — **Timeline Panel**, 2nd sprint) — ✅ **closed 2026-07-22
(Human-approved).** Interactive scene dots: drag a dot → **Time Delta Picker** on release → commit via
`scrivi_set_scene_story_time` (manual placement) + **chain-propagate** subsequent offsets; also a dot
context-menu "Set Time Delta…". **EP-025 AC3 Verified** live over VNC; container build green (193/193,
0 warnings) + new `timeline_story_time_smoke` + all Linux smokes PASS; `scrivi.h` untouched (endpoint from
EP-016). Tasks T-0325–T-0328 all Verified. `Closed/Sprint-SP-080.md`.

Prior: **SP-079** (EP-025 `[Linux]` — **Timeline Panel**, opened the Epic) — ✅ **closed 2026-07-22
(Human-approved).** The read + layout + select core: a hideable **bottom** strip (`TimelinePanel`) with one dot
per scene in **story-time** order (chain-computed from new `ScriviBridge::getTimeline` + `getSceneStoryTime`
invokables), a **View ▸ Show Timeline** toggle (Ctrl+Alt+T) on the SP-077 menu bar, and bidirectional
**dot↔navigator selection** + tooltip. **EP-025 AC1 + AC2 Verified** live over VNC; container build green
(185/185, 0 warnings) + all Linux smokes PASS; `scrivi.h` untouched (timeline C ABI complete from EP-016).
Tasks T-0321–T-0324 all Verified. `Closed/Sprint-SP-079.md`.

Prior: **SP-078** (EP-024 `[Linux]` — **Scene Inspector Panel**, opened AND closed the Epic) — ✅
**closed 2026-07-22 (Human-approved).** A hideable right-side tabbed panel mirroring Apple EP-014 — a
`SceneInspector` (`QTabWidget`, one stub "Scene Entities" tab: title + "No entities yet." + disabled Add
Entity) docked as `EditorShell`'s **third splitter pane**, toggled from **View ▸ Show Inspector** (Ctrl+Alt+I,
checkable, editor-only) on the SP-077 menu bar; session-scoped, default **shown**, **120/200 width** (user
pref); window enlarged to **1020×760**. Verified live over VNC (T-0318–T-0320); container build green
(184/184, 0 warnings) + all Linux smokes PASS. Qt/C++ UI only — no `scrivi_*`/`scrivi.h`/Apple change; no new
headless smoke (pure UI). **This closed EP-024** in a single sprint, like Apple EP-014/SP-037.
`Closed/Sprint-SP-078.md`.

Prior: **SP-077** (`[Linux]` — **Native menu bar**) — ✅ **closed 2026-07-22 (Human-approved).** Built
a native `QMenuBar` on `ScriviWindow` (File/Edit/Scene/Chapter/Project) so every core op has a mouse-driven
trigger + shortcut hint. **This unblocked EP-028 AC5/AC6 Linux verification over VNC** — the macOS→VNC input
path swallows `Ctrl-Shift-Backspace` (Shift held; proven via key-log, not an app bug), so chapter-merge was
verified live via **Chapter ▸ Merge**. Also: File ▸ New/Open route to the landing page's create/open UI
(T-0314/T-0315); **flush-safe editor-leaving paths** closed a data-loss hole (Close/New/Open now flush first,
T-0316); Ctrl+W closes to landing (T-0317); TEMP diagnostics stripped (T-0313). Container build + smokes green;
full VNC walkthrough user-verified. Tasks T-0310–T-0317 all Verified. `Closed/Sprint-SP-077.md`.

Prior: **SP-076** (EP-028 `[Linux]` — **Scene & chapter merge parity**, the final EP-028 leg) — ✅ **closed
2026-07-22 (Human-approved).** Brought Linux to macOS parity: `Ctrl-Backspace` (scene) / `Ctrl-Shift-Backspace`
(chapter) merges backed by SP-074's `scrivi_merge_scene` / `scrivi_merge_chapter`, no confirmation, no-op at
manuscript start, reload-from-disk (I-0081-safe). Scene-merge verified live over VNC; chapter-merge verified
live via the SP-077 menu. Plus two Apple-parity extras surfaced in VNC testing: between-scene separator rule
(T-0308) + `Ctrl+Return` mid-scene split (T-0309). New `scene_merge_smoke` in CI; container build + smokes
green. Linux-only (Qt/C++); `scrivi.h`/ScriviCore untouched. Covered **AC5, AC6 (Linux), AC7** — **closed
EP-028**. Tasks T-0304–T-0309 all Verified. `Closed/Sprint-SP-076.md`.

Prior: **SP-075** (EP-028 `[Apple]` — **Adopt the merge endpoints**) — ✅ **closed 2026-07-21
(Human-approved).** Pointed the macOS `⌘-Backspace` / `⇧⌘-Backspace` merge commands at SP-074's endpoints
(retiring the `deleteChapter`-composed chapter-merge = the I-0083 cause); new `ScriviEngine.mergeScene`/
`mergeChapter` wrappers + `MergeSceneResult`/`MergeChapterResult`; `xcodebuild build`+`test` green (36/36
interop, incl. 4 new merge tests). Scene-merge join changed to blank-line (user-approved). **I-0083 & I-0084
Verified** (user GUI); **I-0085** closed Not-a-Bug (env. duplicate-instance + macOS 27.0 beta panel-XPC
timeout); **I-0086** `try?`-unused build warnings fixed. EP-028 AC1–AC4 complete. Tasks T-0302–T-0303.
`Closed/Sprint-SP-075.md`.

Prior: **SP-074** (EP-028 `[ScriviCore]` — **Merge endpoints + filesystem-coherence fix**) — ✅
**closed 2026-07-21 (Human-approved).** Two new atomic C ABI endpoints `scrivi_merge_scene` (`SceneMerger`) /
`scrivi_merge_chapter` (`ChapterMerger` — the atomic I-0083 fix: relocates scene files into the predecessor
BEFORE removing the emptied chapter). 11 merge tests incl. the I-0083 loss guard on the old
`deleteChapter`-composed path; ctest **macOS 317/317 + Linux 324/324**; both C symbols exported in
`libScriviCore.a`; `scrivi.h` boundary stayed pure C ABI. Fixed stale `devops/docker/linux/Dockerfile`
(missing `libssl-dev`). Tasks T-0298–T-0301. `Closed/Sprint-SP-074.md`.

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
| SP-100 | EP-031 verification & Epic close (⚠️ owned the **AC1 re-verification**) | EP-031 | 2026-08-19 | 2026-08-19 | ✅ Closed |

⚠️ **This table is a copy of [`Sprint-backlog.md`](Sprint-backlog.md)'s, and copies go stale.** It was
refreshed 2026-08-12 (SP-086/SP-090/SP-085/SP-056 had all closed; SP-057 was superseded into SP-094) and
again **2026-08-17**, when it still listed **SP-106 as 🔵 Planning "runs FIRST" the day it closed** and
**SP-102 as Planning after it was activated** — while the section above already said otherwise, in this same
file. **`Sprint-backlog.md` is authoritative; consult it rather than this copy.**

## All Sprints

⚠️ **This table is the single source for Sprint status (P7).** Counts are **not** restated here or in
Statistics — read the rows. Next available ID: **SP-119** — ⚠️ **SP-107–SP-114 are RESERVED to EP-032**
(see the reserved-ID note in Statistics). **Do not reissue them.** ⚠️ **SP-115 runs BEFORE them** (Q-a).

**SP-106 ✅ closed 2026-08-17 (user-approved)** — `[Cross]` test integrity & CI trust. It ran **first**, before
SP-102 and SP-100, because SP-100's Epic-verification remit rested on a suite that had not run clean on
x86-64 since 2026-07-30. **All 3 Tasks and both Issues (I-0121, I-0122) Verified.** Record:
[`Closed/Sprint-SP-106.md`](Closed/Sprint-SP-106.md). ✅ **Its All-Sprints row was added in the same step as
the close** — the step the 2026-08-12 and 2026-08-16 audits both found missing.

**SP-102 activated 2026-08-17; planning completed the same day** → [`Sprint-active.md`](Sprint-active.md).
⚠️ **The planning audit found most of T-0389's staged scope already shipped** in SP-099/SP-104 and cut it.
What remains: the card-level §7.2 footer, the warning view (`listPendingEdges` has zero call sites), the AC24
volume refinement, and the `sources` card — plus **T-0415**, the AC23 live disk-image verification, split out
because a fixture cannot establish it. Two rulings (**R1** warning-view anchoring, **R2** AC24 via volume
keys); **Doc 3 §4.6 amended at planning**, before any code.

> ✅ **SP-099, SP-104 and SP-105 all closed 2026-08-15 (user-approved).** Together they delivered EP-031's
> `[Apple]` half and cleaned up after the T-0409 scope ruling — Issues **I-0114–I-0119** carried, of which
> **I-0114–I-0119 are ✅ ALL Verified** — I-0118/I-0119 on 2026-08-14/15, and **I-0114–I-0117 on 2026-08-17**
> (user-approved, verified live during the SP-102 / T-0415 world runs); all six archived to
> [`../Issues/Verified/Issue-verified-0111-0120.md`](../Issues/Verified/Issue-verified-0111-0120.md). `ctest`
> **516/516**, macOS interop **86/86**. **SP-103 also closed** the same day — T-0410 was removed as OBE
> (user-ruled: a worldless project operates silently; the prompt answered a problem that never occurred).
> ⚠️ **EP-031 AC1 must be re-verified** against the new scope model — SP-100 owns that.

> ⚠️ **SP-104 opened UNPLANNED 2026-08-14**, during live verification of SP-103's scope change — the user
> could not create an object in the app. See [Sprint-SP-104.md](Sprint-SP-104.md). It carries four Issues
> (I-0114–I-0117), of which **I-0114 is a class issue**: the 4th and 5th occurrence of a restated kind
> list in this Epic. (SP-103 closed 2026-08-15: its 11 test realignments were finished in SP-104, and
> T-0410 was removed as OBE.)

> ⚠️ **SP-102 opened 2026-08-13 at SP-099 planning** (R2 — the SP-099 split; EP-031 became a 7-sprint Epic
> *at that date* — superseded: SP-103/104/105 have since been added, making it a **10-sprint** Epic).
> It takes the number after SP-101 because SP-101 was already used by the unplanned EP-030 AC12 sprint, so
> **SP-102 runs before SP-100 despite the higher number.** SP-100 keeps its number as the Epic close sprint.

> **Table repaired 2026-08-12.** SP-086–SP-094 and SP-101 had closed without ever being added to the table
> below, which still ended at SP-085 and read "Next available: SP-086". All eleven missing rows (including
> SP-095) were reconstructed from `Closed/` at the SP-095 close. Historical notes:
> the SP-068 **ID** was skipped when EP-027 claimed SP-069–SP-072 — its EP-023 chapter-drag scope shipped as
> **SP-073** (✅ closed 2026-07-19); **SP-083 was brought forward** ahead of SP-082, both closed.

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
| SP-056 | Copy buffers — store, ABI, palette/menu UX, ⌘/⌃/⌥1–9 chords, history integration (**AC6**) | EP-019 `[Apple]` | 2026-07-24 | 2026-07-27 | ✅ Closed |
| SP-057 | Undo/Redo — verification sprint: AC2/AC7/AC8 live verify + perf fixtures + Epic close | EP-019 `[Apple]` | — | — | ⚪ **Superseded into SP-094** (2026-08-11) — record in [`Sprint-SP-094.md`](Closed/Sprint-SP-094.md) |
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
| SP-075 | `[Apple]` Adopt the merge endpoints (regression-safe swap) — app-side I-0083 fix (AC4) | EP-028 `[Cross]` | 2026-07-21 | 2026-07-21 | ✅ Closed |
| SP-076 | `[Linux]` Scene & chapter merge parity (AC5/AC6/AC7) — final EP-028 leg | EP-028 `[Cross]` | 2026-07-21 | 2026-07-22 | ✅ Closed |
| SP-077 | `[Linux]` Native menu bar (File/Edit/Scene/Chapter/Project) — unblocked EP-028 AC5/AC6 Linux verify over VNC | — `[Linux]` | 2026-07-22 | 2026-07-22 | ✅ Closed |
| SP-078 | `[Linux]` Scene Inspector Panel — dockable tabbed panel + Scene Entities stub + View-menu toggle | EP-024 `[Linux]` | 2026-07-22 | 2026-07-22 | ✅ Closed |
| SP-079 | `[Linux]` Timeline panel scaffold + scene dots (story-time) + show/hide + dot↔navigator selection | EP-025 `[Linux]` | 2026-07-22 | 2026-07-22 | ✅ Closed |
| SP-080 | `[Linux]` Scene-dot drag + Time Delta Picker + chain propagation | EP-025 `[Linux]` | 2026-07-22 | 2026-07-22 | ✅ Closed |
| SP-081 | `[Linux]` Story-structure bands — overlay, border drag, band assignment | EP-025 `[Linux]` | 2026-07-22 | 2026-07-23 | ✅ Closed |
| SP-083 | `[Linux]` Timeline zoom + pan (brought forward — fixes I-0087) | EP-025 `[Linux]` | 2026-07-22 | 2026-07-23 | ✅ Closed |
| SP-082 | `[Linux]` Historical events + imported timelines + export | EP-025 `[Linux]` | 2026-07-24 | 2026-07-24 | ✅ Closed |
| SP-084 | `[Linux]` Co-located dot clustering + persistence + full EP-025 verify & Epic close | EP-025 `[Linux]` | 2026-07-24 | 2026-07-24 | ✅ Closed |
| SP-085 | `[Cross]` Design doc + trade studies + fragment schema (`scrivi.fragment.v1`) | EP-029 `[Cross]` | 2026-07-27 | 2026-07-27 | ✅ Closed |
| SP-086 | `[ScriviCore]` `scrivi.fragment.v1` model + extract-fragment | EP-029 `[Cross]` | 2026-07-27 | 2026-07-27 | ✅ Closed |
| SP-087 | `[ScriviCore]` paste-splice | EP-029 `[Cross]` | 2026-07-27 | 2026-07-27 | ✅ Closed |
| SP-088 | `[ScriviCore]` cut-with-merge | EP-029 `[Cross]` | 2026-07-27 | 2026-07-27 | ✅ Closed |
| SP-089 | `[Apple]` editor wiring — clipboard, structured buffers, reversible undo (closed EP-029) | EP-029 `[Cross]` | 2026-08-03 | 2026-08-03 | ✅ Closed |
| SP-090 | `[Apple]` Scene Inspector card framework (protocol, layout schema, tabs) | EP-030 `[Apple]` | 2026-08-05 | 2026-08-05 | ✅ Closed |
| SP-091 | `[Cross]` Writing-tool cards (tags · todo · outline) | EP-030 `[Apple]` | 2026-08-05 | 2026-08-05 | ✅ Closed |
| SP-092 | `[Cross]` History card + Properties tab | EP-030 `[Apple]` | 2026-08-05 | 2026-08-11 | ✅ Closed |
| SP-093 | `[Cross]` History capture granularity + presentation | EP-019 `[Apple]` | 2026-08-10 | 2026-08-11 | ✅ Closed |
| SP-094 | EP-019 + EP-030 verification & Epic close (merged) | EP-019 + EP-030 | 2026-08-11 | 2026-08-11 | ✅ Closed |
| SP-101 | `[Apple]` EP-030 AC12 soft-failure isolation (T-0399) | EP-030 `[Apple]` | 2026-08-11 | 2026-08-11 | ✅ Closed |
| SP-095 | `[ScriviCore]` Object kinds + fields + object index | EP-031 `[ScriviCore]` | 2026-08-12 | 2026-08-12 | ✅ Closed |
| SP-096 | `[ScriviCore]` Relationship graph — canonical edges, append-log, compaction | EP-031 `[ScriviCore]` | 2026-08-12 | 2026-08-12 | ✅ Closed |
| SP-097 | `[ScriviCore]` World packages — bindings, resolution, locking, epoch chain | EP-031 `[ScriviCore]` | 2026-08-12 | 2026-08-12 | ✅ Closed |
| SP-098 | `[ScriviCore]` Graph integrity — cascade-prune, orphans, promotion, ⚠️ pending-vs-dangling | EP-031 `[ScriviCore]` | 2026-08-12 | 2026-08-12 | ✅ Closed |
| SP-099 | `[Apple]` Engine wrappers, object cards, picker, in-place create/edit, Worlds menu | EP-031 `[ScriviCore]` | 2026-08-13 | 2026-08-15 | ✅ Closed |
| SP-103 | ⚠️ `[Cross]` All worldbuilding kinds → world scope (breaking; no migration) | EP-031 `[ScriviCore]` | 2026-08-14 | 2026-08-15 | ✅ Closed |
| SP-104 | ⚠️ `[Cross]` Post-ruling fallout — world reachability + the restated-kind class (unplanned) | EP-031 `[ScriviCore]` | 2026-08-14 | 2026-08-15 | ✅ Closed |
| SP-105 | `[Cross]` World search indexing (I-0118) (unplanned) | EP-031 `[ScriviCore]` | 2026-08-14 | 2026-08-15 | ✅ Closed |
| SP-106 | ⚠️ `[Cross]` Test integrity & CI trust — I-0121 + sanitizer CI leg + macOS platform coverage (unplanned) | EP-031 `[Cross]` | 2026-08-16 | 2026-08-17 | ✅ Closed |
| SP-102 | `[Apple]` Pending presentation + warning view + `sources` card (**AC23 + AC24**) | EP-031 `[Apple]` | 2026-08-17 | 2026-08-18 | ✅ Closed |
| SP-107 | ⚠️ **RESERVED (EP-032)** — Design + trade rulings Q1–Q6, design doc; no implementation | EP-032 `[Cross]` | — | — | ⚪ **Reserved, not planned** |
| SP-108 | ⚠️ **RESERVED (EP-032)** — `[ScriviCore]` Reference syntax, parse/write, `source`→`scene` seed type (AC1, AC9) | EP-032 `[Cross]` | — | — | ⚪ **Reserved, not planned** |
| SP-109 | ⚠️ **RESERVED (EP-032)** — `[ScriviCore]` Fragment-model support + cut/copy/paste (AC2) | EP-032 `[Cross]` | — | — | ⚪ **Reserved, not planned** |
| SP-110 | ⚠️ **RESERVED (EP-032)** — `[Apple]` Rendering, resolution, atomic editing + undo (AC3, AC4) | EP-032 `[Cross]` | — | — | ⚪ **Reserved, not planned** |
| SP-111 | ⚠️ **RESERVED (EP-032)** — `[Apple]` Footnote + pull-quote presentations, citation popup (AC5) | EP-032 `[Cross]` | — | — | ⚪ **Reserved, not planned** |
| SP-112 | ⚠️ **RESERVED (EP-032)** — `[Cross]` Deleted/pending targets + repair-matrix rows (AC6) | EP-032 `[Cross]` | — | — | ⚪ **Reserved, not planned** |
| SP-113 | ⚠️ **RESERVED (EP-032)** — `[Linux]` Parity (AC8) | EP-032 `[Cross]` | — | — | ⚪ **Reserved, not planned** |
| SP-114 | ⚠️ **RESERVED (EP-032)** — AC verification + live-use pass on the real rig + Epic close prep (AC7, AC10) | EP-032 `[Cross]` | — | — | ⚪ **Reserved, not planned** |
| SP-115 | ⚠️ **The five open Issues + I-0142 (user-found)** — all ✅ Verified | EP-034 `[Cross]` | 2026-08-20 | 2026-08-20 | ✅ Closed |
| **SP-116** | ⚠️ **`[Cross]`** World assets (D6) + `assetPath` (D7) + kind-scope endpoint (D5) — retired **six Issues** | EP-034 `[Cross]` | 2026-08-21 | 2026-08-21 | ✅ Closed |
| **SP-117** | ⚠️ **`[Cross]`** Detail Sheet shell (D1, D2, D3) + **T-0440's ABI reason field** — ⚠️ **first writer-facing surface of EP-034** | EP-034 `[Cross]` | 2026-08-21 | 2026-08-21 | ✅ Closed |
| **SP-118** | ⚠️ **`[Cross]`** Related objects + relationship creation (D4) + **T-0416's reconciliation** — ⚠️ **13 Issues, ALL from the live pass, none from any suite** | EP-034 `[Cross]` | 2026-08-22 | 2026-08-23 | ✅ Closed |

## Statistics

⚠️ **The All-Sprints table above is the SINGLE SOURCE for per-sprint status and counts (P7).** This
section carries **only what the table cannot express** — per-status totals were removed 2026-08-19 under
audit ruling **R-22(④)**, because a summary that restates a table will drift from it. **To count Sprints
by status, read the table.**

- **Next available Sprint ID:** **SP-119** — ⚠️ **not SP-107**; See the reserved block below.
- **Total Sprint IDs issued:** 118 (SP-001–SP-118) — 110 executed, **0 ACTIVE**, **0 in Planning**, **8 reserved (SP-107–SP-114)**
- ⚠️ **IDs RESERVED to an Epic that is not executing them yet:**
  - **SP-107–SP-114** — reserved to **EP-032** `[Cross]` (Inline Object References) at its promotion on
    2026-08-20. ⚠️ **EP-032 was RETURNED to the Epic backlog the same day, and the reservation SURVIVED
    that return — which is exactly what it was written for.** All eight are now ⚪ **Reserved, not
    planned** (SP-107 was briefly 🔵 Planning); EP-032 has no active or planned sprint, and the later
    IDs' content depends on SP-107's Q1–Q6 rulings, which have not happened.
    > ⚠️ **These IDs must NOT be reissued to another Epic's sprints, and this reservation SURVIVES
    > EP-032 being returned to the Epic backlog.** The reservation exists precisely for that case: work
    > sequenced ahead of EP-032 takes **SP-115+**. Releasing any of these IDs is a **user ruling**, not a
    > planning convenience.
  - **Where their detail lives:** [`../Epics/Epic-backlog.md`](../Epics/Epic-backlog.md) — it **moved with
    the Epic**, as designed. ⚠️ **It is deliberately NOT recorded in `Epic-active.md`**, so that a future
    promotion/return cycle cannot strand it again.
- **IDs that are not sprints, and why:**
  - **SP-034** — ⚪ Cancelled
  - **SP-057** — ⚪ Superseded → SP-094
  - **SP-068** — ⚪ **Skipped**; EP-027 claimed SP-069–SP-072 and its scope shipped as SP-073
- **Out-of-sequence execution:** **SP-083 ran before SP-082**; **SP-102 ran before SP-100**, which keeps
  its number as EP-031's close sprint and runs last; ⚠️ **SP-115+ (EP-034) runs BEFORE SP-107–SP-114
  (EP-032)** — ✅ Q-a ruled 2026-08-20 that EP-034 precedes EP-032, because EP-032's AC5 needs the source
  creation EP-034 delivers.

> ⚠️ **This drift recurred THREE times before it was fixed structurally** — the All-Sprints table and the
> statistics disagreed in 2026-08-16, 2026-08-17 and 2026-08-19, each time because the summary restated
> the table. The correct diagnosis was written down twice (here and in `Epic-Documentation.md`) and acted
> on neither time. **Do not reintroduce per-status counts here.**

---

*Last Updated: 2026-08-23, ninth pass (**SP-118 ✅ CLOSED, user-approved** — five Tasks + **thirteen
Issues** Verified and archived in the same step; closes **AC5, AC6, AC7**. ⚠️ **Thirteen Issues, ZERO found
by any suite** — every one from the user's live click-through, which is now unambiguously this Epic's
highest-value verification step. ⚠️ **Four were one failure: an existing correct pattern the new code did
not follow.** ⚠️ **T-0443's deviation from the plan was RULED IN FAVOUR by the user.** `ctest` **561/561**
arm64 · x86-64 · ASan; Linux **565/565** non-root; interop **120/120**; app **BUILD SUCCEEDED**. Active
Sprints 1 → **0**; none in Planning. Prior note follows.)*

*Last Updated: 2026-08-22, eighth pass (**SP-118 🟡 ACTIVATED** 2026-08-22 and implemented the same day —
five Tasks ✅ Implemented — Not Verified. ⚠️ **AC5/AC6 are NOT closed**: they close on **S9**, the live
click-through, which is the user's to run. ⚠️ **T-0441 required a DRIFTED fixture** — a fresh project is
already correct, so a normally-seeded test passes vacuously; the negative control was run and both halves
fail without the fix. ⚠️ **T-0443 deviates from the plan's letter** (a second picker rather than a
mode-conditional `ObjectPickerView`) — **a ruling is owed**. ctest **559/559** arm64 · x86-64 · ASan; Linux
**563/563** non-root; interop **119/119**; app **BUILD SUCCEEDED**. Prior note follows.)*

*Last Updated: 2026-08-21, seventh pass (**SP-118 CREATED 🔵 Planning** — EP-034's fourth sprint; five
Tasks T-0441–T-0445, closes **AC5 + AC6**. ⚠️ **T-0416 confirmed on the rig and scheduled as T-0441.**
Next available Sprint ID: **SP-119**. Prior note follows.)*

*Last Updated: 2026-08-21, sixth pass (**SP-117 ✅ CLOSED, user-approved** — EP-034's third sprint and
⚠️ **the first to ship a writer-facing surface**; seven Tasks + I-0148 Verified and archived. ⚠️ **Recorded
`[Cross]`, not `[Apple]`** — T-0440 needed an ABI `statusReason` field because the reason a world is
unavailable was discarded by `resolve()`. ⚠️ **The live click-through both VALIDATED the D1-E ruling and
found I-0148** — the fifth EP-034 defect found by use. Suites: ctest **554/554** · Linux **558/9332** ·
interop **115/115** · **BUILD SUCCEEDED**. Next: **SP-118**. Prior note follows.)*

*Last Updated: 2026-08-21, fifth pass (**SP-117 → 🟠 REVIEW** — all seven Tasks ✅ Verified (user-approved)
and archived, plus I-0148. ⚠️ **Recorded as `[Cross]`, not `[Apple]`**: T-0440 required a ScriviCore
`statusReason` field, because the reason a world is unavailable was discarded by `resolve()` and carried by
no envelope. ⚠️ **Claude cannot close a Sprint** — awaiting user approval. Prior note follows.)*

*Last Updated: 2026-08-21, fourth pass (**SP-117 CREATED 🔵 Planning** — EP-034's third sprint; seven
Tasks T-0434–T-0440, no open Issues. ⚠️ **The first EP-034 sprint to ship a writer-facing surface.** Next
available Sprint ID: **SP-118**. Prior note follows.)*

*Last Updated: 2026-08-21, third pass (**SP-116 ✅ CLOSED, user-approved** — EP-034's second sprint; eight
Tasks + six Issues Verified and archived in the same step → [`Closed/Sprint-SP-116.md`](Closed/Sprint-SP-116.md).
⚠️ **Two defects found by LIVE USE** (I-0144's never-called lock, I-0146's orphaned partial on a pulled USB
drive) and **one accepted limitation** (I-0147, deferred to the network-worlds design). Suites: ctest
**552/552** (arm64 · x86-64 · ASan) · Linux **556/9300** · interop **107/107** · **BUILD SUCCEEDED**.
Next: **SP-117**. Prior note follows.)*

*Last Updated: 2026-08-21, second pass (**SP-116 🟡 ACTIVATED** — status flipped in the All-Sprints table,
the header and Statistics; it left `Sprint-backlog.md` in the same step, so that file now holds **zero**
Sprints in Planning. Five Tasks T-0426–T-0430 moved backlog → active. Next available Sprint ID: **SP-117**.
Prior note follows.)*

*Last Updated: 2026-08-21 (**SP-116 CREATED 🔵 Planning** — EP-034's second sprint; five Tasks
**T-0426–T-0430**, three Issues **I-0140, I-0141, I-0143**. Plan in
[`Sprint-backlog.md`](Sprint-backlog.md). ⚠️ **Recorded as `[Cross]`, NOT `[ScriviCore]` as the Epic table
first estimated** — D5's endpoint is adopted in Swift in the same sprint so **I-0140 is cured, not merely
made curable**. ⚠️ **I-0143 filed at planning** (`scrivi_list_assets` cannot escape its own JSON) and fixed
here, because **T-0427 is what makes it reachable**. ⚠️ **Two P7 restatements repaired in this file** — the
header carried the "Next available Sprint ID" sentence **twice**, contradicting the rule that it is
recorded once, in Statistics. Next available Sprint ID: **SP-117**. Prior note follows.)*

*Last Updated: 2026-08-20 (**SP-115 ✅ CLOSED, user-approved** — EP-034's first sprint; seven Tasks + six
Issues Verified and archived in the same step → [`Closed/Sprint-SP-115.md`](Closed/Sprint-SP-115.md).
⚠️ **I-0137 verified on the real rig, drive ejected**; ⚠️ **I-0142 found by the user, not a suite.**
⚠️ **Carried out: T-0420's missing surface (unowned)** and **I-0140/I-0141 (SP-116)**. Suites: ctest
**525/525** · interop **103/103** · app **BUILD SUCCEEDED**. Next: **SP-116**. Prior note follows.)*

*Last Updated: 2026-08-20 (**SP-115 → 🟠 REVIEW: all seven Tasks + six Issues ✅ Verified and archived in
the same step.** ⚠️ **I-0137 verified on the real rig, drive ejected** — the check a suite cannot
substitute for. ⚠️ **Carried out: T-0420 has no writer-facing surface**, and **I-0140/I-0141 stay open for
SP-116.** ⚠️ **Claude cannot close a Sprint** — awaiting user approval. Prior note follows.)*

*Last Updated: 2026-08-20 (**SP-115 🟡 ACTIVATED** — EP-034's first sprint; six Tasks T-0419–T-0424.
⚠️ Scope is the five Issues **and nothing else**; the Detail Sheet begins SP-116. ⚠️ **SP-115 executes
BEFORE the reserved SP-107–SP-114** (Q-a). Next available for other work: **SP-116**. Prior note follows.)*

*Last Updated: 2026-08-20 (**EP-034 activated → SP-115 🔵 Planning — the five open Issues I-0135–I-0139 and
nothing else** (user ruling). ⚠️ **Q-a ruled EP-034 precedes EP-032**, so **SP-115+ executes BEFORE the
reserved SP-107–SP-114** — recorded in the out-of-sequence note, alongside SP-083/SP-082 and SP-102/SP-100.
**Next available for other work: SP-116.** ⚠️ **The EP-032 reservation is untouched and still holds.**
Prior note follows.)*

*Last Updated: 2026-08-20 (**EP-032 returned to the Epic backlog — SP-107 dropped 🔵 Planning → ⚪ Reserved;
all 8 IDs SP-107–SP-114 now reserved, none planned. No Epic active, no Sprint active or planned.**
⚠️ **The reservation survived the Epic's move**, which is what it was written for; its detail moved with
EP-032 to `../Epics/Epic-backlog.md` and is deliberately not kept in `Epic-active.md`. **Next available
Sprint for other work: SP-115 — not SP-107.** Prior note follows.)*

*Last Updated: 2026-08-19, later same day (🟡 **SP-100 ACTIVATED (user-approved)** — EP-031's eleventh
and last sprint. Its row left `Sprint-backlog.md` in the same step (**that file is now empty**), per the
rule that a Sprint leaves the backlog at activation and never returns. **T-0390 is 🟡 Active**; T-0418 and
T-0391 stay 🔵 Backlog until the build order reaches them. ⚠️ **The rig is backed up** — T-0418's blocking
precondition, user-confirmed. Also corrected here: a stale *"Next available Sprint: SP-106"* line, which
contradicted the Statistics block's **SP-107** in the same file. Prior note follows.)*

*2026-08-19 (**SP-100 planned — 🔵 Planning, NOT activated.** Plan in
[`Sprint-active.md`](Sprint-active.md), banner-marked as not-yet-active. Three Tasks **T-0390 → T-0418 →
T-0391** (**T-0418 new**); four user-approved rulings — **R1/R2** amend **AC1** and **AC10**, each of
which carried a clause superseded by the §3.0 no-migration ruling and so **could not be re-verified as
written**; **R3** T-0390 documents **and** tests, filing rather than fixing; **R4** a live-use pass on the
real USB rig is **required evidence**. Both amended ACs **stay unticked**.
⚠️ **Two stale rows in this file corrected in the same pass — SP-102 was still shown 🟡 Active in both the
All-Sprints table (l.348) and the Statistics block**, though it closed 2026-08-18 and the Active-Sprints
section at the top already said so. **This is the third time this file's lower tables have lagged its own
header** (see the 2026-08-17 note below). Counts: **101 closed, 0 Active, 1 Planning (SP-100)**; next
available **SP-107**. Prior note follows.)*

*2026-08-17 (**SP-106 ✅ CLOSED (user-approved) and SP-102 activated.** All 3 SP-106 Tasks
(T-0412/T-0413/T-0414) and both Issues (**I-0121, I-0122**) ✅ Verified and archived in the same step —
`../Tasks/Verified/Task-verified-0412-0414.md` and the new `../Issues/Verified/Issue-verified-0121-0130.md`.
✅ **The All-Sprints row was added as part of closing**, which the 2026-08-12 and 2026-08-16 audits both found
had been skipped at the previous closes; a row for the newly-active **SP-102** was added at the same time.
Also corrected: **"Next available" read SP-106 in the Statistics block while the Currently line read SP-107** —
both now say **SP-107**. Counts updated: 100 closed, 1 Active (SP-102), 1 Planning (SP-100). ⚠️ **SP-102's
scope predates SP-104/SP-105/SP-106 and a planning pass is owed** before implementation. Prior note follows.)*

*2026-08-16 (**consistency audit — the All-Sprints table was missing the four Sprints that
closed 2026-08-15.** SP-099, SP-103, SP-104 and SP-105 had archives in `Closed/` but no table row; the table
ran SP-098 → SP-100 → SP-101 → SP-102. ⚠️ **This is the same defect the 2026-08-12 repair note records
(SP-086–SP-094 and SP-101 closed without ever being listed) recurring at the very next close** — adding the
row is evidently not happening as part of closing a Sprint. Also corrected: the Statistics block still read
"Total Sprints: 83 / Closed: 83" from 2026-07-27 and listed SP-057 as the only Planning sprint; **SP-057's
table row still showed 🔵 Planning under an Epic that closed 2026-08-11** and is now marked ⚪ Superseded into
SP-094; the "Currently: 99 Sprints" line conflated IDs issued with sprints closed; and the **I-0114–I-0119
verified** claim was wrong — **I-0118/I-0119 are Verified, I-0114–I-0117 are still `Resolved - Not Verified`**
per `../Issues/Issue-active.md`. The Active-Sprints section carried five stale close-notes for archived
Sprints and stated EP-031 was a six-sprint Epic; it now carries current status only. Prior note follows.)*

*2026-07-27 (**SP-085 ✅ closed (Human-approved) — EP-029 design doc approved.** The EP-029
`[Cross]` design sprint delivered + approved `Scrivi_Structured_CutCopyPaste_Design_v0_1.md`: the
`scrivi.fragment.v1` ordered-pieces schema, extract / paste-splice / cut-merge behaviour (composing EP-027
create/split + EP-028 merge), buffer-schema evolution, and a one-reversible-event history shape. Trades ruled
**T1=A · T2=A · T3=A · T4=A**; Open Questions #1–#3 resolved — no cross-window structured paste in v1;
**caret-in-heading paste = refuse + flash the screen** (user override of the drafted silent-retarget); divider-
anchored selection normalised to the adjacent scene body. Task T-0350 Verified; docs-only (`scrivi.h`/pbxproj
untouched). `Closed/Sprint-SP-085.md`. Closed 82→83, Active stays 0; **EP-029 → SP-086** (ScriviCore
extract-fragment) next, awaiting go-ahead. Next available Task **T-0351**. Prior note follows.)*

*2026-07-27 (**SP-056 ✅ closed (Human-approved) — EP-019 AC6 Verified; SP-085 planned under new
EP-029.** SP-056 delivered multiple copy buffers (T-0213 + T-0214 both Verified live): explicit ⌘1–9 copy /
⌃1–9 paste / ⌥1–9 cut chords (buffer 0 = the untouched system pasteboard), an app-global Copy Buffers palette
(follows the frontmost project), Edit-menu Copy/Paste/Cut To Buffer + Scene/Chapter New/Merge menu items, and a
bufferID-tagged `cut` history event (backend `scrivi_history_record_event` + node schema extended). ctest 327 +
interop 43 green. A cross-boundary Cut/Copy/Paste gap surfaced during verify → new Epic **EP-029** `[Cross]`; its
design sprint **SP-085** is planned (design doc `Scrivi_Structured_CutCopyPaste_Design_v0_1.md`, task T-0350) and
awaits activation. EP-019 is **held pending** its final sprint SP-057. Total Sprints 82→83; Active 1→0; next
available Sprint **SP-086**. Prior note follows.)*

*2026-07-24 (**EP-019 `[Apple]` un-deferred → SP-056 activated** (user request: lock copy buffers
on Apple before Linux). SP-056 (6th of 7 EP-019 sprints) delivers **AC6** — vim/emacs-register-style multiple
copy buffers: 9 per-project persistent slots (`history/buffers.json`), keyboard HUD + palette + Edit-menu UX
(Trade T4), copy≠event/cut=event/paste=ordinary-event (Trade T3), system pasteboard untouched. Tasks T-0213
(`BufferStore` + `scrivi_buffers_*` C ABI + engine wrappers + interop) + T-0214 (buffer UX + history
integration). Backend greenfield-but-small (peer to the built `HistoryStore`); Apple-only (Linux copy buffers =
EP-026). Active 0→1, Planning 2→1. Next available Sprint **SP-085**. Prior note follows.)*

*2026-07-24 (**SP-084 ✅ closed (Human-approved) — this closed EP-025.** The 5th and final EP-025
sprint delivered **AC6b** (co-located dot clustering — aggregate dot + hover fan-out + zoom-resolve, Apple
T-0174 — + a persistence sweep + full EP-025 verify); T-0346–T-0349 all Verified live; four clustering findings
fixed + re-verified same session (incl. a phantom double-draw defect). Container green (216/216) + new
`timeline_cluster_smoke` + 14 regression smokes PASS; `scrivi.h` untouched; no pbxproj. **EP-025 CLOSED** — all
ACs AC1–AC6b Verified across six sprints; archived `../Epics/Closed/Epic-EP-025.md`. Closed 80→81, Active 1→0
(no active sprint or Epic). Next available Sprint **SP-085**. Records: `Closed/Sprint-SP-084.md`. Prior note
follows.)*

*2026-07-24 (**SP-082 ✅ closed (Human-approved) + SP-084 planned + activated** — EP-025 `[Linux]`
Timeline Panel. **SP-082** (4th sprint) delivered **AC5** — historical events (author/edit/delete + `#C8A97A`
draggable dots + context menus), imported timelines (grey rows below, per-source shade, window-clip,
`EpochOffsetDialog`, hide/show, edit-offset, remove), export, + File ▸ Import/Export menu items (T-0345);
T-0340–T-0345 all Verified live; two findings fixed + re-verified same day (I-0090 imported row behind the
scrollbar, I-0091 dialogs defaulted to `/root`); container green (211/211) + `timeline_events_smoke` + 11
regression smokes PASS; archived `Closed/Sprint-SP-082.md`. **SP-084** now active (5th and final sprint):
co-located dot **clustering** (aggregate dot — larger core + count + segmented arc ring + selection arc, hover
fan-out, zoom-resolve; Apple T-0174) + **persistence sweep** + **full EP-025 verify** + **Epic close**; delivers
**AC6b**; tasks T-0346–T-0349; new `timeline_cluster_smoke`; `scrivi.h` untouched; no pbxproj. Closed 79→80,
Active stays 1 (SP-082→SP-084); next available **SP-085**. Prior note follows.)*

*2026-07-24 (**SP-082 planned + activated** — EP-025 `[Linux]` Timeline Panel, 4th sprint,
delivering **AC5**: historical events (author/edit/delete + `#C8A97A` draggable dots + context menus), imported
timelines (grey rows below, per-source shade, window-clip, `EpochOffsetDialog`, hide/show, edit-offset, remove),
and export (`.scrivi-timeline.json` → re-import round-trip). Tasks T-0340–T-0344; timeline-events C ABI complete
(EP-016, `scrivi.h` 270–291, untouched); new `timeline_events_smoke`; no pbxproj (Linux-only). Clustering +
panel-persistence sweep + Epic close carved out to **SP-084**. **SP-081 + SP-083 ✅ both closed 2026-07-23**
(AC4 bands + AC6a zoom/pan; T-0329–T-0339; closed I-0087/I-0088/I-0089; archived). Closed 78→79, Active 0→1;
next available **SP-084**. Records: `Closed/Sprint-SP-081.md`, `Closed/Sprint-SP-083.md`. Prior note follows.)*

*2026-07-22 (**SP-080 ✅ closed (AC3 Verified) + SP-081 activated** — EP-025 `[Linux]` Timeline
Panel. SP-080 delivered interactive scene dots (drag → Time Delta Picker → manual placement + chain
propagation; + "Set Time Delta…" context menu), T-0325–T-0328 all Verified live over VNC; container build green
(193/193) + `timeline_story_time_smoke` + all smokes PASS; `scrivi.h` untouched. **SP-081** now active (3rd
sprint): story-structure bands — structure selector paints colored proportional bands + labels behind the dots,
border-drag re-proportion (`update_band_layout`), scene→band assignment (drag-up / "Assign to Act…",
`assign_scene_to_band`) with a colored ring, remove keeps offsets+assignments. Delivers AC4. Tasks T-0329–T-0332;
full story-structure C ABI exists (EP-016), `scrivi.h` untouched; new `story_structure_smoke`. Closed 75→76,
Active stays 1 (SP-080→SP-081); next available SP-082 (historical + imported events). Records:
`Closed/Sprint-SP-080.md`. Prior note follows.)*

*2026-07-22 (**SP-079 ✅ closed with user approval** — EP-025 `[Linux]` Timeline Panel's first
sprint, planned + activated + implemented + verified + closed same day. Delivered the timeline read + layout +
select core: a hideable bottom `TimelinePanel` strip with one scene dot per scene in story-time order
(chain-computed from new `ScriviBridge::getTimeline` + `getSceneStoryTime` invokables), a View ▸ Show Timeline
toggle (Ctrl+Alt+T) on the SP-077 menu bar, and bidirectional dot↔navigator selection + tooltip. T-0321–T-0324
all ✅ Verified live over VNC; **AC1 + AC2 met**; container build green (185/185, 0 warnings) + all Linux smokes
PASS; `scrivi.h` untouched. Closed 74→75, Active 1→0 — no active sprint; **EP-025 stays Active** → SP-080
(scene-dot drag + Time Delta Picker, AC3). Next available SP-080. Record: `Closed/Sprint-SP-079.md`. Prior note
follows.)*

*2026-07-22 (**SP-078 ✅ closed with user approval** — EP-024 `[Linux]` Scene Inspector Panel,
planned + activated + implemented + verified + closed same day (mirroring Apple EP-014/SP-037). A hideable
right-side `QTabWidget` panel docked as `EditorShell`'s third splitter pane, a stub "Scene Entities" tab, and
a View ▸ Show Inspector toggle (Ctrl+Alt+I) on the SP-077 menu bar; session-scoped, default shown, 120/200
width (user pref); window enlarged to 1020×760. T-0318–T-0320 all ✅ Verified live over VNC; container build
green (184/184, 0 warnings) + all Linux smokes PASS. Qt/C++ UI only — no `scrivi_*`/`scrivi.h`/Apple change.
**This closed EP-024.** Closed 73→74, Active 1→0 — no active sprint, no Active Epic; next available SP-079.
Record: `Closed/Sprint-SP-078.md`. Prior note follows.)*

*2026-07-22 (**SP-076 + SP-077 ✅ both closed with user approval** — the final EP-028 `[Linux]`
legs. **SP-076** delivered scene & chapter merge parity (`Ctrl-Backspace`/`Ctrl-Shift-Backspace`, reload-from-disk,
no confirmation) + between-scene separator rule + `Ctrl+Return` mid-scene split; scene-merge live over VNC,
chapter-merge live via the SP-077 menu; new `scene_merge_smoke` in CI. **SP-077** delivered the native menu bar
(File/Edit/Scene/Chapter/Project) that made chapter-merge verifiable over VNC (macOS→VNC swallows
`Ctrl-Shift-Backspace`), plus flush-safe editor-leaving paths (closed a data-loss hole) + Ctrl+W. **This closed
EP-028** — AC5/AC6/AC7 Verified (AC1–AC4 already Verified from SP-074/SP-075). Closed 71→73, Active 1→0 —
**no active sprint, no Active Epic**; next available **SP-078**. Records: `Closed/Sprint-SP-076.md`,
`Closed/Sprint-SP-077.md`. Prior note follows.)*

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
