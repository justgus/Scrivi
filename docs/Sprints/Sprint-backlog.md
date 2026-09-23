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

## ⚠️ RESERVED Sprint IDs — SP-145–SP-148 ([EP-043] `[Linux]` The Session)

⛔ **These are NOT in Planning, and none is ready to activate.** ✅ **[EP-043] was created 2026-09-21
into the Epic backlog** (🔵 Draft) → [`../Epics/Epic-EP-043.md`](../Epics/Epic-EP-043.md).
⚠️ **The IDs are recorded here ONLY so they are not reissued** — ✅ **the same precaution taken for
EP-032's SP-107–SP-114 above**, and for the same reason the user gave when ordering that one.

| Sprint | Step | Provisional scope | ACs |
| ------ | ---- | ----------------- | --- |
| **SP-145** | S1 | ✅ **The session split** — per-project state out of the 2,783-line `EditorShell`; ⚠️ **behaviour-preserving**; registry introduced | — (foundation) |
| **SP-146** | S2 | ✅ **The windows** — one window per project; R3 focus-existing; quit flushes ALL; per-window `scrivi_close_project` | R1 R2 R3 R7 R8 |
| **SP-147** | S3 | ✅ **The persistence** — open-session manifest + launch restore; per-project geometry **and splitters**; ⚠️ **the test guard lands HERE** | R4 R5 R6 |
| **SP-148** | S4 | ✅ **Verification** — AC sweep, Docker `ctest` as non-root, ⚠️ **live pass on the real rig**, close prep | AC-build AC-live |

⚠️ **THE CHAIN IS SERIAL** — ✅ **for a real reason (⛔ you cannot persist per-window state before there
are per-window identities)** — ⚠️ **but a stall in SP-145 stalls the whole Epic.** ✅ **[EP-018]
delivered the Apple equivalent in THREE Sprints; ⚠️ the fourth here is SP-145**, because ⚠️ **Apple
extracted `ProjectSession` from an `AppEnvironment` that was ALREADY a separate object, whereas Linux's
per-project state lives inside the widget itself.**

⚠️ **FOUR RULINGS ARE OWED before SP-145 can activate** — ✅ **listed in the Epic record, not restated
here** (P7: a second copy is what goes stale).

⚠️ **A reserved ID becomes a real 🔵 Planning entry only when its Epic is active and its turn comes**,
then leaves this file at activation and never returns.

---

## ✅ SP-129 — **CLOSED 2026-09-15 (user-approved).** ⛔ **NO LONGER A BACKLOG SPRINT.**

✅ **Record: [`Closed/Sprint-SP-129.md`](Closed/Sprint-SP-129.md).** ⚠️ **Its planning detail lived
here; ✅ the closed record supersedes it** — ⛔ **a closed Sprint must not keep a planning block in the
backlog** (`feedback_sprint_backlog_cleanup`).
✅ **T-0502 / T-0503 / T-0506 + [I-0214] VERIFIED;** ⛔ **T-0504 / T-0505 DEFERRED as design questions.**

---

## ✅ SP-130 — **CLOSED 2026-09-18 (user-approved).** ⛔ **NOT A BACKLOG SPRINT.**

✅ **Record: [`Closed/Sprint-SP-130.md`](Closed/Sprint-SP-130.md).** ✅ **T-0508 Verified; ⛔ T-0509 struck.**
⚠️ **[I-0197] is NOT closed by it** — ✅ **Class C only; the chain continues in [SP-140]–[SP-143].**
⚠️ **A Sprint leaves this file at activation and never returns** (`feedback_sprint_backlog_cleanup`).

---

## ✅ SP-149 — **CLOSED 2026-09-22 (user-approved).** ⛔ **NO LONGER A BACKLOG SPRINT.**

✅ **Record: [`Closed/Sprint-SP-149.md`](Closed/Sprint-SP-149.md).** ⚠️ **Its planning detail lived
here; ✅ the closed record supersedes it** (`feedback_sprint_backlog_cleanup`).
✅ **T-0541 VERIFIED by a green GitHub run.** ⚠️ **Found 2026-09-22 by [EP-041]'s Audit Check.**

⚠️ **Superseded planning text follows.**

### (as planned)

✅ **Plan: [`Sprint-SP-149.md`](Sprint-SP-149.md).** ⛔ **Not activated.** ✅ **Blocks on NOTHING** —
⚠️ **it is design work and can run in parallel with [SP-142].**

⚠️ **Split out of [SP-143] 2026-09-21**, ✅ **because the guard's DESIGN half blocks on nothing while
its VERIFY half blocks on two Sprints** — ⛔ **bundled, the design would not begin until the blockers
cleared**, ⚠️ **which is exactly how [EP-041]'s own record predicted AC4 would slip: *"an Epic that
promises a guard without specifying it will close with a prose paragraph instead of a check."***

⚠️ **IT WAS DRAFTED AS SP-145 AND RENUMBERED** — ✅ **SP-145–SP-148 are reserved for [EP-043] above**,
⛔ **the exact collision that reservation block exists to prevent.**

⚠️ **Its central finding is not a lint question:** ⛔ **no CI workflow covers `Scrivi/` at all**, ✅ **and
`check-textkit2.sh` — the precedent [EP-041] AC4 names — is wired into nothing.**

✅ **FULLY RULED 2026-09-21 (user) — ⚠️ AC1 is MET BEFORE ACTIVATION**, ⛔ **which is the whole reason
it was split out of [SP-143]:** ✅ **narrow API pattern (⛔ not `fileExists` — Class C is ruled) ·
a NEW lint-only `scrivi-apple-ci.yml` on `Scrivi/**` (that tree's FIRST CI) with `check-textkit2.sh`
wired in beside it · ONE script, TWO pattern sets, BOTH workflows · [SP-149] rules AND builds, with
[SP-143] as the independent witness.**

✅ **[I-0241] IS NOW ASSIGNED to [SP-142] as T-0542** (user ruling 2026-09-21), ⚠️ **its own Task, NOT
folded into T-0537.** ⚠️ **Detail below; ⛔ [SP-149] still only ALLOW-LISTS it.**

⚠️ **PLANNING [SP-149] FOUND A LIVE BYPASS NOTHING ELSE HAD — [I-0241]:** ⛔ **`EditorShell.cpp:2469` reads
`objects/historical-events/*.json` straight off disk, scanning every file per lookup** — ✅ **a pattern
Apple RETIRED in [SP-129], still live on Linux.** ⛔ **Not fixed here; allow-listed with its Issue
number, because the fix needs a CORE change.**

---

## ✅ SP-134 — **CLOSED 2026-09-22 (user-approved).** ⛔ **NO LONGER A BACKLOG SPRINT.**

✅ **Record: [`Closed/Sprint-SP-134.md`](Closed/Sprint-SP-134.md).** ⚠️ **Its planning detail lived
here; ✅ the closed record supersedes it** (`feedback_sprint_backlog_cleanup`).
✅ **T-0543 VERIFIED by the user's live pass; ✅ [EP-040] AC2/AC3 met.**
⚠️ **It produced [I-0243]** (⛔ **the project title renders THREE times**) — ✅ **[SP-135]'s.**

---


## ✅ SP-135 — **CLOSED 2026-09-22 (user-approved).** ⛔ **NO LONGER A BACKLOG SPRINT.**

✅ **Record: [`Closed/Sprint-SP-135.md`](Closed/Sprint-SP-135.md).** ⚠️ **Its planning detail lived
here; ✅ the closed record supersedes it** (`feedback_sprint_backlog_cleanup`).
✅ **All 8 ACs met; T-0545 VERIFIED; [I-0203] and [I-0205] resolved.**

## 🔵 SP-137 — **PLANNED 2026-09-23** ([EP-040]) — ⚠️ **the Object Detail Sheet's real chrome**

✅ **Plan: [`Sprint-SP-137.md`](Sprint-SP-137.md).** ⛔ **Not activated.**
⚠️ **[EP-040]'s LAST Sprint.** ✅ **Serves EP-040 AC8** (*the sheet no longer hand-builds window chrome*).

✅ **THREE RULINGS TAKEN 2026-09-23, so the Sprint starts decided:** ⚠️ **Q1 hosting — KEEP THE MODAL
SHEET** (⛔ **§4.4's option (a) is VOID: [I-0245] proved D1-E's beside-pane cannot coexist with the
Inspector column, and the app crashed**); ⚠️ **Q2 scope — ALL the chrome** (back/forward + close become
real `.toolbar` items; ✅ **Save/Cancel STAY as content-level actions — they are not window chrome**);
⚠️ **Q3 — a layout-convergence GUARD, its own Task** ([SP-149]'s precedent).

⚠️ **ONE QUESTION REMAINS OWED (Q4): what SHAPE the guard takes.** ✅ **Recommendation in the plan:
a static `scripts/` check + a debug assertion.**

✅ **Tasks: T-0547** (real sheet chrome) · **T-0548** (the guard). ✅ **8 ACs.**

⚠️ **CLOSES [I-0247]**, which was raised deliberately at [SP-136]'s close so D1-E's superseded intent
would be RULED, not absorbed silently.

---

**Next available Sprint ID: SP-150.** ✅ **SP-149 CLOSED 2026-09-22; ⚠️ SP-145–SP-148 reserved for [EP-043].**

⚠️ **Prior line, kept for provenance:** **Next available Sprint ID: SP-149.** ⚠️ **CORRECTED 2026-09-21 — this line read `SP-134` and was
STALE:** ✅ **SP-134–SP-137 belong to [EP-040], SP-140–SP-143 to [EP-041], and SP-144 has CLOSED.**
⚠️ **SP-145–SP-148 are now RESERVED for [EP-043]** (see the reservation block below).

✅ **SP-115 through SP-128 are ALL CLOSED and ARCHIVED**
(→ [`Closed/`](Closed/)). ⚠️ **SP-129 and SP-130 above MOVED TO [EP-040] 2026-09-15** — ✅ **their status is owned by EP-040's Sprint table, not by this file.**
🟡 **SP-132 and SP-133 are ACTIVE** and live in [`Sprint-active.md`](Sprint-active.md).
🟡 **SP-131 is ACTIVE** and lives in [`Sprint-active.md`](Sprint-active.md) — ⚠️ **it left this file at
activation and never returns.** ⚠️ **SP-129/SP-130 sequence BEHIND it and behind the `[Apple]` async
sprint.** ⚠️ **Next available Task: T-0544.** ✅ **T-0543 was issued to [SP-134] and is now VERIFIED and archived** (2026-09-22). ⛔ **This line read `T-0532` and was corrected 2026-09-21.**
⚠️ **T-0541 is RESERVED for [SP-149]; ✅ T-0542 for [SP-142] ([I-0241], assigned by user ruling
2026-09-21).** ⛔ **Neither is issued until its Sprint activates.**
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
