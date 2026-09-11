# Sprint Backlog

Sprints listed here are in 🔵 Planning status — defined and ready to activate, but not yet started.

---

## ⚠️ RESERVED Sprint IDs — SP-107–SP-114 (EP-032)

**These are NOT in Planning, and none is ready to activate.** ⚠️ **EP-032 was returned to the Epic backlog
on 2026-08-20 by user ruling**, so it has no active or planned sprint. These eight IDs are recorded here so
they are **not reissued** to sprints executed before EP-032 — the exact collision the user identified when
ordering the reservation.

> ⚠️ **SP-107 was a 🔵 Planning entry earlier today and is now RESERVED.** Its Epic left the active file,
> and a Sprint in Planning for a backlogged Epic would misstate what is ready to run. **Its planning
> content is not lost** — the Q1–Q6 questions it exists to rule live in EP-032's backlog entry
> ([`../Epics/Epic-backlog.md`](../Epics/Epic-backlog.md)), along with AC1–AC10.

| Sprint | Provisional scope | ACs |
| ------ | ----------------- | --- |
| **SP-107** | ⚠️ **Design + trade rulings** — Q1–Q6, design doc; **no implementation** | — (gates all others) |
| SP-108 | `[ScriviCore]` Reference syntax, parse/write, `source`→`scene` seed type | AC1, AC9 |
| SP-109 | `[ScriviCore]` Fragment-model support + cut/copy/paste | AC2 |
| SP-110 | `[Apple]` Rendering, resolution, atomic editing + undo | AC3, AC4 |
| SP-111 | `[Apple]` Footnote + pull-quote presentations, citation popup | AC5 |
| SP-112 | `[Cross]` Deleted/pending targets + repair-matrix rows | AC6 |
| SP-113 | `[Linux]` Parity | AC8 |
| SP-114 | ⚠️ AC verification + **live-use pass on the real rig** + Epic close prep | AC7, AC10 |

⚠️ **Sequencing stays provisional until SP-107 actually rules Q1–Q6**, and eight is an estimate made
*before* that ruling. EP-031 planned 6 and delivered 11 — four of the five additions came from **use**, not
planning. If EP-032 ends up needing fewer, surplus IDs are released **by user ruling**, not silently reused.

⚠️ **Two rulings EP-032 owes before it can be re-promoted**, both recorded in its backlog entry: **Q5** —
manuscript export has no existing path to attach to (`scrivi_export_project_timeline` is the timeline, not
the manuscript), so **AC7 is unverifiable as written**; and the **EP-034 sequencing question** — without
source *creation*, AC5's footnotes are verifiable only on hand-authored fixtures.

> ✅ **SP-124 was ACTIVATED 2026-08-31 and left this file in the same step**, per the standing rule that
> a Sprint leaves the backlog at activation and never returns. Plan and status now live in
> [`Sprint-active.md`](Sprint-active.md). ⚠️ **No closure note will be written here.**

⚠️ **Its scope was WIDENED at activation by user ruling** — three drive-loss scenarios rather than one.
✅ **The widening is recorded in the ACTIVE file, not here**, because a backlog entry that no longer
governs anything is a second source of truth.

---

## SP-129 — `[Apple]` ⚠️ **The four unbuilt surfaces** — and the direct-filesystem bypass they exposed

**Status:** 🔵 **PLANNING — not activated.**
**Epic:** [EP-039](../Epics/Epic-backlog.md) — `[Cross]` Project Load Performance · ⚠️ **sprint 1 of N**
**Codebase:** `[Apple]` — ⚠️ **Swift/SwiftUI only.** ✅ **No ScriviCore change expected** (the endpoints
all exist and are live on Linux).
**Date Created:** 2026-09-10
**Tasks:** **T-0502 – T-0506** (five) · **Next available:** T-0507

### ⚠️ Why this is in the PERFORMANCE Epic, not a parity Epic

⚠️ **User ruling 2026-09-10: integrate this with the performance work.** ✅ **The reason is not
scheduling — it is that ONE of these four is ALREADY a performance defect of exactly the kind EP-039
exists to fix.**

⚠️ **`TimelineViewModel.loadImportedTimelines` (`TimelineStripView.swift:395-420`) DOES NOT CALL THE CORE
AT ALL.** ⚠️ **It reads `objects/imported-timelines/` with `FileManager.contentsOfDirectory`, then
`Data(contentsOf:)` and `JSONDecoder` per file, IN SWIFT** — ⚠️ **while `scrivi_list_imported_timelines`
exists, is bound in `ScriviEngine` (`:924`), and is what Linux calls.**

⚠️ **THIS VIOLATES A STANDING ARCHITECTURAL RULE** (CLAUDE.md): *"No backend logic is reimplemented in
Swift. Swift is responsible for UI only."* ⚠️ **It also runs on the timeline's load path — the SAME path
measured at `251 s` in [I-0196]** — ✅ **so it is squarely EP-039's business.**

✅ **MEASURED SCOPE: only 3 direct-filesystem call sites exist in `Scrivi/Views` + `Scrivi/App`.**
⚠️ **Contained, not systemic — which is exactly why it is worth closing NOW, before it spreads.**

### ⚠️ The four endpoints are NOT equivalent — they split two ways

✅ **AUDITED 2026-09-10.** ⚠️ **Two have a REAL, FINISHED Linux surface to mirror; two have NO UI on
either platform.** ⚠️ **Treating all four the same would re-earn
`feedback_mirror_the_finished_surface_not_the_placeholder`.**

| endpoint | Linux UI | ⚠️ What Apple needs |
| -------- | -------- | ------------------ |
| `listImportedTimelines` | ✅ **YES** — `EditorShell::reloadImportedTimelines` | ⚠️ **Apple has a surface but BYPASSES the core to feed it** |
| `updateImportedTimelineOffset` | ✅ **YES** — `EpochOffsetDialog` (`EditorShell.cpp:2606`) | ⚠️ **No Apple surface at all** |
| `setTimelineEpochLabel` | ⚠️ **NO** — bridge method only | ⚠️ **NEITHER platform has a surface** |
| `promoteObject` | ⚠️ **NO** — bridge method only | ⚠️ **NEITHER platform has a surface** |

⚠️ **THE LAST TWO ARE NOT A PARITY GAP.** ✅ **They are `project_capability_without_surface`: a shipped
core capability NO platform ever surfaced.** ⚠️ **Building an Apple UI for them means DESIGNING one, not
mirroring one** — ⚠️ **and a design invented to close a checkbox is how placeholder surfaces get built.**

### Tasks

| ID | Task | Priority | Status |
| -- | ---- | -------- | ------ |
| **T-0502** | ⚠️ **`[Apple]` Route `loadImportedTimelines` THROUGH `scrivi_list_imported_timelines`** — ⚠️ **delete the `FileManager`/`JSONDecoder` bypass.** ✅ **The endpoint is already bound at `ScriviEngine:924`** | **High** | 🔵 Not started |
| **T-0503** | ⚠️ **`[Apple]` Imported-timeline OFFSET editing** — mirror Linux's `EpochOffsetDialog`; calls `updateImportedTimelineOffset` | **Medium** | 🔵 Not started |
| **T-0504** | ⚠️ **`[Apple]` Epoch-label editing** — `setTimelineEpochLabel`. ⚠️ **DESIGN REQUIRED: no platform has this surface** | **Low** | 🔵 Not started — ⚠️ **needs a design ruling first** |
| **T-0505** | ⚠️ **`[Apple]` Object promotion** — `promoteObject` (project-scoped → world-scoped). ⚠️ **DESIGN REQUIRED: no platform has this surface**, ⚠️ **and it MOVES a writer's object between packages — the failure modes need ruling BEFORE a button exists** | **Low** | 🔵 Not started — ⚠️ **needs a design ruling first** |
| **T-0506** | ⚠️ **Audit the remaining 2 direct-filesystem call sites** in `Scrivi/Views` + `Scrivi/App`; ✅ **route through the core or record WHY not** | **Medium** | 🔵 Not started |

### Definition of Done

- [ ] ⚠️ **No Swift code reads project files directly** where a `scrivi_*` endpoint exists — ✅ **or the
      exception is RECORDED with its reason.**
- [ ] ⚠️ **T-0504/T-0505 are either BUILT from a ruled design, or DEFERRED with the design question
      written down** — ⚠️ **NOT built as placeholders to close a checkbox.**
- [ ] ⚠️ **A LIVE PASS** — ⚠️ **each new surface is USED by a writer, not just compiled**
      (`feedback_live_pass_finds_what_suites_cannot`).
- [ ] ⚠️ **`xcodebuild` green for macOS, iOS AND visionOS** — ⚠️ **iOS/visionOS regressed once already
      because a view was written macOS-first** (2026-09-10).

### ⚠️ Risks

| Risk | ⚠️ Mitigation |
| ---- | ------------ |
| ⚠️ **T-0504/T-0505 get built as placeholders** | ✅ **They are explicitly gated on a DESIGN RULING.** ⚠️ **A surface invented to close a checkbox is the defect `feedback_mirror_the_finished_surface_not_the_placeholder` names** |
| ⚠️ **`promoteObject` moves data between packages** | ⚠️ **It relocates a writer's object.** ✅ **Rule the failure modes (world unavailable mid-promote, duplicate identity) BEFORE any button exists** |
| ⚠️ **T-0502 changes the timeline load path** | ⚠️ **That path is [I-0196]'s `251 s`.** ✅ **Measure before AND after with `ScriviDiag`, so a "fix" cannot quietly make it slower** |

---

## SP-130 — `[Apple]`+`[ScriviCore]` ⚠️ **Close the ScriviCore bypasses** — [I-0197]

**Status:** 🔵 **PLANNING — not activated.**
**Epic:** [EP-039](../Epics/Epic-backlog.md) — `[Cross]` Project Load Performance
**Codebase:** `[Apple]` ⚠️ **+ `[ScriviCore]`** — ⚠️ **Class B needs a NEW ENDPOINT; it is not an app-only sprint**
**Date Created:** 2026-09-10 · **Issues:** ✅ **[I-0197]**
**Tasks:** **T-0507 – T-0510** (four) · **Next available:** T-0511
**Depends on:** ⚠️ **[SP-129]'s T-0502 already closes CLASS A** — ⚠️ **do not do it twice**

### ⚠️ The rule being enforced

⚠️ **CLAUDE.md, non-negotiable:** *"No backend logic is reimplemented in Swift. Swift is responsible for
UI only."* ⚠️ **Every bypass is a place the CORE'S GUARANTEES DO NOT APPLY** — ✅ no atomic-write
discipline, no repair path, no external-change detection, no `soft`-mount timeout, ⚠️ **and no
Linux/Windows equivalent.**

✅ **MEASURED: 35 hits, 11 files, THREE CLASSES.** ⚠️ **They are NOT equally wrong and must not be fixed
uniformly.**

### Tasks

| ID | Task | Priority | Status |
| -- | ---- | -------- | ------ |
| **T-0507** | ⚠️ **CLASS B — `inspector-layout.json`.** ⚠️ **`InspectorLayoutStore` READS AND ATOMICALLY WRITES a file INSIDE the project package** (`:156`, `:173`, `:304`) ⚠️ **and the core has ZERO endpoints for it.** ✅ **Add them; route Swift through them.** ⚠️ **THE CORE OWNS PACKAGE LAYOUT, VALIDATION AND REPAIR — today it cannot even SEE this file** | **High** | 🔵 Not started |
| **T-0508** | ⚠️ **CLASS C — RULE the `fileExists` asset checks** (`ObjectCard:980,984`, `ObjectImageSection:106,137`, `ExistingAssetPicker:130`). ⚠️ **Decide: legitimate UI presence-check, or a core question?** ⚠️ **On an unreachable volume a bare `fileExists` is the BLOCKING-STAT pattern that cost [I-0193] 102 s** — ✅ **whatever is ruled, it must not block the UI thread** | **Medium** | 🔵 Not started |
| **T-0509** | ⚠️ **Sweep the remaining hits** in the 11 files; ✅ **each is ROUTED or RECORDED with its reason.** ⚠️ **A recorded exception is acceptable; an unexamined one is not** | **Medium** | 🔵 Not started |
| **T-0510** | ⚠️ **A REGRESSION GUARD.** ✅ **A test or CI grep that FAILS when Swift touches a project-package path directly**, ⚠️ **with an explicit allow-list carrying T-0509's recorded reasons** — ⚠️ **otherwise this class returns the moment someone is in a hurry** | **Medium** | 🔵 Not started |

### Definition of Done

- [ ] ⚠️ **No Swift code reads or writes a PROJECT-PACKAGE file** where the core owns that file — ✅ **or the
      exception is recorded WITH ITS REASON.**
- [ ] ⚠️ **`inspector-layout.json` is CORE-OWNED** — ✅ **readable and repairable by the core, and reachable
      by Linux/Windows**, ⚠️ **which cannot read it today.**
- [ ] ⚠️ **The Class C ruling is WRITTEN DOWN**, ⚠️ **not left implicit in whatever the code ends up doing.**
- [ ] ⚠️ **A guard exists** so the class cannot silently return.
- [ ] ⚠️ **`xcodebuild` green for macOS, iOS AND visionOS.**

### ⚠️ Risks

| Risk | ⚠️ Mitigation |
| ---- | ------------ |
| ⚠️ **Treating all 35 hits as one defect** | ✅ **Three classes, three treatments.** ⚠️ **Most hits are app-support/bookmark paths and are NOT violations at all** |
| ⚠️ **`inspector-layout.json` is Git-visible project state** | ⚠️ **Changing who writes it touches a file already on disk in real projects.** ✅ **The core must READ the existing format, not require a migration** |
| ⚠️ **Class C "fixed" by adding a blocking core call** | ⚠️ **That would trade an architectural defect for a FREEZE** ([I-0193]). ✅ **Rule the question first** |
| ⚠️ **Overlap with [SP-129] T-0502** | ✅ **Class A is SP-129's.** ⚠️ **This sprint must not redo it** |

---

**Next available Sprint ID: SP-131** — ⚠️ **SP-115–SP-123 and SP-125–SP-127 are ALL CLOSED**; ✅ **SP-124** (EP-038) is **COMPLETE 2026-09-10 and awaiting close approval**; 🟡 **SP-128** (EP-038) is **ACTIVE — activated 2026-09-10** and lives in [`Sprint-active.md`](Sprint-active.md). ⚠️ **SP-128 never entered this file**, per the standing rule that a Sprint leaves the backlog at activation and never returns.
and ⚠️ **the LAST sprint of EP-038.** ⚠️ **SP-122 never appeared in this file** — defined and activated
in one step on 2026-08-25, like SP-120 and SP-121; ⚠️ **it was EP-034's last.**
⚠️ **Zero Sprints in Planning.**

⚠️ **SP-121 never appeared in this file** — defined and activated in one step on 2026-08-24, like SP-120.

⚠️ **SP-120 never appeared in this file.** It was defined and activated in one step on 2026-08-24, so
there is no Planning row to remove — recorded here because an ID vanishing from the sequence with no
backlog trace otherwise reads as an omission.

**8 IDs reserved (SP-107–SP-114)** for EP-032 — ⚠️ **reserved, not planned.**

---

---

---

---

> ✅ **SP-119 was ACTIVATED 2026-08-23 and left this file in the same step**, per the standing rule that a
> Sprint leaves the backlog at activation and never returns. Plan and status now live in
> [`Sprint-active.md`](Sprint-active.md). ⚠️ **No closure note will be written here.**

> ✅ **SP-118 was ACTIVATED 2026-08-22 and left this file in the same step**, per the standing rule that a
> Sprint leaves the backlog at activation and never returns. Plan and status now live in
> [`Sprint-active.md`](Sprint-active.md). ⚠️ **No closure note will be written here.**

> ✅ **SP-117 was ACTIVATED 2026-08-21 and left this file in the same step**, per the standing rule that a
> Sprint leaves the backlog at activation and never returns. Plan and status now live in
> [`Sprint-active.md`](Sprint-active.md).

> ✅ **SP-116 was ACTIVATED 2026-08-21 and left this file in the same step**, exactly as SP-115 did — per
> the standing rule that **a Sprint leaves the backlog at ACTIVATION and never returns.** Its plan and
> status now live in [`Sprint-active.md`](Sprint-active.md). ⚠️ **No closure note will be written here.**

> ✅ **SP-115 was ACTIVATED 2026-08-20 and left this file in the same step** — per the standing rule that
> **a Sprint leaves the backlog at ACTIVATION and never returns.** Its plan and status live in
> [`Sprint-active.md`](Sprint-active.md). ⚠️ **No closure note will be written here.**

⚠️ **SP-115 and SP-116 both execute BEFORE SP-107–SP-114.** ✅ **Q-a ruled 2026-08-20: EP-034 precedes
EP-032**, so sprint IDs run **out of sequence** — precedented (SP-083 ran before SP-082; SP-102 before
SP-100). ⚠️ **Next available is recorded ONCE, at the top of this file** — restating it here is what let it
go stale (P7).

⚠️ **The standing rule still holds:** a Sprint leaves this file at **activation** and never returns.
SP-100 activated and left on 2026-08-19; it closed the same day (`Closed/Sprint-SP-100.md`). ⚠️ **The
reserved block above is an exception in KIND, not to that rule** — it holds **IDs, not sprints**. A reserved
ID becomes a real 🔵 Planning entry only when its Epic is active and its turn comes, then leaves at
activation like any other.

---

⚠️ **No closure notes are kept in this file.** A Sprint leaves the backlog at **activation** and never
returns; whether it later closed is recorded in [`Closed/`](Closed/) and in
[`Sprint-Documentation.md`](Sprint-Documentation.md)'s All-Sprints table.

> **Removed 2026-08-19 under audit ruling R-28:** ~230 lines of closure records for **SP-052 → SP-106**.
> Every one duplicated its `Closed/Sprint-SP-XXX.md` archive, and the file had accreted so far that its
> single Planning row sat under a year of history.
>
> ⚠️ **Before deletion, the planning rulings inside those notes were checked against their archives**
> (the R-28 safeguard) — SP-095's gating ruling, SP-096's ID-prefix finding, SP-103's scope ruling and
> T-0410's OBE removal, and the SP-097/098/099/102/105/106 rulings. **All present in `Closed/`.
> Nothing was reconstructed because nothing was lost.**

*Last Updated: 2026-08-23, ninth pass (**SP-119 🟡 ACTIVATED and REMOVED from this file** in the same step —
its plan moved to [`Sprint-active.md`](Sprint-active.md). ⚠️ **Zero Sprints in Planning.** Next available
Sprint: **SP-120**; Task: **T-0451**; Issue: **I-0162**. Prior note follows.)*

*Last Updated: 2026-08-23, eighth pass (**SP-119 REVISED after user review.** ⚠️ **The
`capability_without_surface` framing was WRONG and was corrected** — ScriviCore is the capability layer and
the App is one surface; **core-before-UI is the CORRECT sequence**, and the real risk is designing the
surface to the "lowest common denominator". ✅ **New criterion S11** makes that a checkable step.
⚠️ **T-0446 corrected twice**: carry the **resolved image path**, not `imageAssetID` alone (*"the writer is
not going to care what the computer calls the image on the inside"*); and ⚠️ **the "two rebuild paths"
claim was imprecise** — `scanDir()` already IS the unified scan and `rebuild()` simply does not call it, so
unification is small and comes FIRST. ✅ **T-0450 NARROWED — the user already tested it live:**
recover-and-save works; ⚠️ **the real gap is no warning when unsaved edits are at risk**, and persisting
them across quit is **ruled out of scope**. Prior note follows.)*

*Last Updated: 2026-08-23, seventh pass (**SP-119 PLANNED 🔵** under EP-034 — five Tasks **T-0446–T-0450**,
no open Issues; **closes AC2, AC3, AC4 and AC9's second half**. ⚠️ **Three user rulings at planning:**
scope is images **+** tags **+** AC9 (not images alone); ⚠️ **`image` is carried in the OBJECT INDEX**
rather than read per row, which makes this `[Cross]`; and ⚠️ **the user is creating the second Eskandar
project** for AC3 — a precondition, not sprint work. ⚠️ **The asset stack has ZERO view callers** — the
fourth and largest `capability_without_surface` in this Epic. Next available Sprint: **SP-120**; Task:
**T-0451**; Issue: **I-0162**. Prior note follows.)*

*Last Updated: 2026-08-22, sixth pass (**SP-118 🟡 ACTIVATED and REMOVED from this file** in the same step —
its plan moved to [`Sprint-active.md`](Sprint-active.md). ⚠️ **This file is back to zero Sprints in
Planning.** Next available Sprint: **SP-119**; Task: **T-0446**; Issue: **I-0149**. Prior note follows.)*

*Last Updated: 2026-08-21, fifth pass (**SP-118 PLANNED 🔵** under EP-034 — five Tasks **T-0441–T-0445**,
no open Issues; **closes AC5 + AC6**. ⚠️ **T-0416 IS SCHEDULED (T-0441) AND WAS CONFIRMED ON THE RIG** —
`the-twisted-remains-of-myself.scrivi` still carries the pre-I-0125 `appears-in`, and the same operation
was proven to succeed on a fresh seed and fail on the drifted one. ✅ **User ruled: reconcile on open,
seeded types only** — ⚠️ **accepted consequence: it overwrites a seeded type a writer edited.**
⚠️ **`tintagael` is CLEAN, so the rig will NOT reproduce T-0416 by default** — S9 requires a deliberately
drifted fixture or the sprint passes vacuously. Next available Sprint: **SP-119**; Task: **T-0446**;
Issue: **I-0149**. Prior note follows.)*

*Last Updated: 2026-08-21, fourth pass (**SP-117 🟡 ACTIVATED and REMOVED** — plan moved to
`Sprint-active.md`. Zero Sprints in Planning. Next available: **SP-118**. Prior note follows.)*

*Last Updated: 2026-08-21, third pass (**SP-117 PLANNED 🔵** under EP-034 — seven Tasks **T-0434–T-0440**,
**no open Issues**. ⚠️ **The first sprint of EP-034 that ships anything a writer can click** — two sprints
have closed without a reachable surface, which is the very defect the Epic exists to cure. ⚠️ **Two
planning rulings**: `tags` deferred to SP-119 (no chip-editor precedent in the app), so **R2/AC2 cannot
close here**; and **both** R7 entry points ship (double-click *and* right-click). ⚠️ **T-0440 finally takes
T-0420's owed surface** — unowned across two sprints because neither shipped UI. ⚠️ **S4 (patch-don't-
reconstruct) is the likeliest silent defect** and **S8 (host-independence) protects D1's successor path**.
Next available Sprint: **SP-118**; Task: **T-0441**; Issue: **I-0148**. Prior note follows.)*

*Last Updated: 2026-08-21, second pass (**SP-116 🟡 ACTIVATED and REMOVED from this file** in the same
step — its plan moved verbatim to [`Sprint-active.md`](Sprint-active.md). ⚠️ **This file is back to zero
Sprints in Planning**; the 8 reserved EP-032 IDs are untouched. Next available Sprint ID: **SP-117**.
Prior note follows.)*

*Last Updated: 2026-08-21 (**SP-116 PLANNED 🔵** under EP-034 — five Tasks **T-0426–T-0430**, three
Issues **I-0140, I-0141** and ⚠️ **I-0143 (filed at planning)**. ⚠️ **Scope WIDENED from the Epic table's
`[ScriviCore]` to `[Cross]`, by two user rulings**: D5 reaches into Swift so **I-0140 is actually cured**
rather than made curable — refusing to repeat `capability_without_surface` in the sprint curing occurrence
*eight* of its sibling — and the `list_assets` escaping defect found while reading D7's target code is
**filed AND fixed here**, because ⚠️ **D7 puts a filesystem path into the very array that cannot escape**.
⚠️ **No UI ships in SP-116** — S1–S9 are core-and-boundary criteria and none proves a writer can do
anything; the surfaces are SP-117–SP-120. Next available Sprint: **SP-117**; Task: **T-0431**; Issue:
**I-0144**. Prior note follows.)*

*Last Updated: 2026-08-20 (**SP-115 🟡 ACTIVATED and REMOVED from this file in the same step** — the
standing rule: a Sprint leaves at activation and never returns. Its six Tasks **T-0419–T-0424** are Active;
plan in [`Sprint-active.md`](Sprint-active.md). **0 Sprints in Planning; 8 IDs still reserved to EP-032.**
Next available for other work: **SP-116**. Prior note follows.)*

*Last Updated: 2026-08-20 (**SP-115 added 🔵 Planning for EP-034 — the five open Issues I-0135–I-0139 and
NOTHING else**, per user ruling; the Detail Sheet work starts at SP-116. ⚠️ **SP-115 executes BEFORE the
reserved SP-107–SP-114** (Q-a: EP-034 precedes EP-032), so IDs run out of sequence. ⚠️ **The EP-032
reservation is untouched.** Next available for other work: **SP-116**. Prior note follows.)*

*Last Updated: 2026-08-20 (**EP-032 returned to the Epic backlog → SP-107 dropped from 🔵 Planning to
⚠️ RESERVED**, joining SP-108–SP-114. **0 Sprints in Planning; 8 IDs reserved.** ⚠️ **The reservation
SURVIVED the Epic's move, which is precisely what it was written for** — had these IDs lived only in
`Epic-active.md`, they would now be unrecorded and reissuable. **Next available for other work stays
SP-115.** Prior note follows.)*

*2026-08-20 (**SP-107 added 🔵 Planning + SP-108–SP-114 recorded as ⚠️ RESERVED**, on the
user's instruction, for EP-032 `[Cross]`. **Reason, in the user's words:** if EP-032 is ruled inactive and
sent back to the Epic backlog, sprint IDs documented only inside `Epic-active.md` "get overwritten when
developing new Sprints that will be executed before these seven." ⚠️ **The reservation is therefore
written to SURVIVE that move** — it is recorded here AND in `Sprint-Documentation.md`'s All-Sprints table
and Statistics block, and **next available is now SP-115, not SP-107**. ⚠️ **SP-108–SP-114 are deliberately
NOT marked 🔵 Planning** — this file's rule is "defined and ready to activate", and they are neither;
their content depends on SP-107's Q1–Q6 rulings. Prior note follows.)*

*2026-08-19 (audit remediation — ruling R-28).*
