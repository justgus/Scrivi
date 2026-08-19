# Active Sprint

> ⚠️ **SP-100 is 🔵 Planning, NOT Active.** This document is the completed plan; **activating it needs
> direct user approval.** Nothing below has been started.

---

## SP-100: EP-031 verification & Epic close

**Status:** 🔵 **Planning** (planned 2026-08-19)
**Epic:** EP-031 — `[ScriviCore]` Worldbuilding Object Model & Relationship Graph
**Codebase:** `[Cross]` — documentation, ScriviCore tests, and a live `[Apple]` use pass. **No new
feature code is planned.**
**Goal:** Close out EP-031's two remaining acceptance criteria — **AC1** (re-verification under the
2026-08-14 scope model) and **AC10** (regression) — document the world-package conditions the External
Change Repair Matrix has never covered, and establish by **use** rather than by suite that the Epic is
done.
**Start Date:** TBD (on activation)
**End Date:** TBD
**Capacity:** TBD

**This is the last of EP-031's eleven sprints, and it runs last by design.**

---

## 1. Four rulings taken at planning (2026-08-19, user-approved)

### R1 — AC1's legacy clause is STRUCK, not re-verified

AC1 reads *"New kinds round-trip; **legacy 5-kind files load unchanged**; `timeline` kind retired."*

⚠️ **The middle clause contradicts a ruling the Epic already took.** §3.0 consequence 2 ruled **no
migration pass**: `objects/characters/…` became `<world>/characters/…`, and the existing test project was
*discarded and recreated by hand*. A legacy project-scoped object file is therefore **not** loaded — by
design, deliberately, and the Epic is correct to do so while no real data exists.

Re-verifying the clause as written would mean asserting a behaviour EP-031 chose **not** to build.

**Ruled: AC1 is amended.** Its verified form becomes:

> **AC1 (amended 2026-08-19)** — All **11** object kinds round-trip in their **ruled scope**: the ten
> worldbuilding kinds (`artifact`, `building`, `character`, `chronicle`, `faction`, `item`, `location`,
> `map`, `rule`, `vehicle`) in the `.scrivworld` package, and **`source` alone** project-scoped at
> `objects/sources/`. The `timeline` kind is retired. **The "legacy 5-kind files load unchanged" clause
> is struck as superseded by the §3.0 no-migration ruling.**

The 2026-08-12 evidence stays in the Epic file as history. It does not establish the amended AC — that is
T-0391's work.

> **What survives from the old clause, and what does not.** `JsonSchemaTests.cpp:812` ("a legacy object
> file without the SP-095 keys parses unchanged") **is still valid and still passes** — it is a
> *schema-level* assertion about absent optional keys, kind-agnostic and scope-agnostic. It was never
> evidence for the scope clause. Nothing else in `ScriviCore/tests/` tests legacy loading; the grep is
> otherwise empty.

### R2 — AC10 is amended to its regression half

AC10 reads *"No regression: `ctest` + interop suites green; **existing projects open unchanged**."*

The second clause fails for the same reason as R1's, and additionally has **no subject**: Scrivi has not
shipped, so no projects exist in the field. Reconstructing a pre-SP-103 project to test against would mean
rebuilding the very thing the ruling discarded.

**Ruled: AC10 is amended.** Its verified form becomes:

> **AC10 (amended 2026-08-19)** — No regression: `ctest` and the macOS interop suite are green **on both
> architectures** (arm64 + x86-64) **and under sanitizers**, per SP-106's standing practice; the app
> builds. **The "existing projects open unchanged" clause is struck** — superseded by the §3.0
> no-migration ruling, and vacuous while no field data exists.

⚠️ **This amendment is only defensible because of SP-106.** Before 2026-08-17 there was no x86-64 gate to
name. AC10 in its amended form is a **stronger** criterion than the one it replaces, not a weaker one.

### R3 — T-0390 documents, then tests against shipped behaviour; gaps are FILED, not fixed

The repair matrix (`Scrivi_External_Change_Repair_Matrix_v0_2.md`, 578 lines, conditions §6.1–§6.21)
contains ⚠️ **zero occurrences of "world" or "scrivworld"**. Every world failure mode is undocumented.

**Ruled:** T-0390 writes the new conditions **and** asserts each against shipped behaviour. Where the
document and the code disagree, the finding is **filed as an Issue** — it is not fixed inside a
verification sprint.

> **Why the middle option and not "document only".** A matrix section with no test behind it is a
> capability claim with no evidence — **the exact shape that recurred four times in this Epic**
> (`project_capability_without_surface`). Documenting world conditions without exercising them would add a
> fifth instance in the sprint whose job is to certify the Epic.
>
> **Why not "fix what you find".** SP-099 doubled at R4 because a verification pass started writing code.
> A close sprint that fixes defects cannot also be the sprint that certifies them.

### R4 — a live-use pass is REQUIRED evidence, not optional polish

⚠️ **Four of EP-031's eleven sprints were unplanned, and every one came from USE:** SP-104 and SP-105 from
the scope ruling's fallout, SP-106 from a CI error the user noticed, and SP-099 doubled at R4 when live
verification found a shipped surface with no way to create data. SP-102's live runs produced **eight
Issues (I-0123–I-0130)**, of which **I-0129 was the AC23 no-intervention clause itself** — invisible to
every fixture, because ejecting a drive normally forces a focus change that masked it.

**Ruled: SP-100 includes a live pass on the real rig** (**T-0418**, new). Closing this Epic on suite
evidence alone would rest the close on precisely the standard the Epic itself disproved four times.

⚠️ **The rig is real writing work** (`project_test_rig_tintagael_eskandar`):
`~/Desktop/the-stairs-of-tintagael.scrivi` bound to `/Volumes/Scrivi Worlds/Eskandar.scrivworld`.
**Back it up before the pass begins** — this is an explicit exit-criterion, not a reminder.

---

## 2. Assigned Tasks

| ID | Title | Priority | Status |
| -- | ----- | -------- | ------ |
| **T-0390** | External Change Repair Matrix — **world-package conditions** (§6.22–§6.28) + tests against shipped behaviour | High | 🔵 Backlog |
| **T-0418** | ⚠️ **Live-use pass on the real rig** — all 10 world kinds, relate, eject/reattach, reopen (**new at planning, R4**) | High | 🔵 Backlog |
| **T-0391** | **EP-031 verification (AC1–AC10) + Epic close prep** — includes the **AC1 re-verification** | High | 🔵 Backlog |

### Assigned Issues

| ID | Title | Severity | Status |
| -- | ----- | -------- | ------ |
| — | *None at planning.* | — | — |

> **I-0133 and I-0134 are not SP-100 work — both are CLOSED OUT as of 2026-08-19.**
> **I-0133** (`restoredScrollFraction` dead state) is ✅ **Verified** and archived to
> [`Issue-verified-0131-0140.md`](../Issues/Verified/Issue-verified-0131-0140.md).
> **I-0134** is ⚪ **Closed as a non-issue** → [`Issue-closed-0134.md`](../Issues/Closed/Issue-closed-0134.md):
> it was filed as an Apple/Linux parity defect, but ⚠️ **parity is directional — Apple is the source of
> truth and Linux conforms**, so a difference between the platforms is the expected state, not a defect.
> **Neither blocks this Epic's close, and neither owes any work.**
>
> ⚠️ **This note previously gave a WRONG reason** — that I-0133 was `Resolved - Not Verified` and I-0134
> 🔴 Open and awaiting a ruling. Both statements were read out of stale rows in `Issue-active.md`
> (audit finding **F-01**) and propagated into this plan. Corrected under audit rulings **§0** and
> **R-01**; the conclusion (neither is EP-031 work) stands, on these grounds instead.

---

## 3. Build order

**T-0390 → T-0418 → T-0391.** The order is load-bearing:

1. **T-0390 first** because it is the only task that can still surface a *core* defect. If a world
   condition behaves wrongly, the Epic must know before it certifies itself, and an Issue filed on day one
   can be scheduled; one filed at close cannot.
2. **T-0418 second** because the live pass is the sprint's real evidence, and it must run against a build
   that already has T-0390's findings recorded — otherwise a live symptom and a documented gap get
   discovered twice and reconciled once.
3. **T-0391 last** because it is the only task that reads all the others as evidence. ⚠️ **It cannot start
   early**, and a partial AC pass is worth nothing.

---

## 4. Task detail

### T-0390 — Repair matrix: world-package conditions

**Codebase:** `[ScriviCore]` (doc + tests) · **Status:** 🔵 Backlog

The matrix documents 21 conditions and **not one of them involves a world**. Worlds landed in SP-097,
three sprints after the matrix was last revised.

**Conditions to write** (numbering continues from §6.21), each in the established format —
*Classification / Behavior / Automatic / Suggested user actions / Do not*:

| § | Condition | Shipped behaviour to assert against |
| - | --------- | ----------------------------------- |
| **6.22** | **World package missing entirely** (volume unmounted, package moved/deleted) | `WorldStore.cpp:308-312` — `missing` **only** when positively established, else `unavailable` |
| **6.23** | ⚠️ **`worldID` mismatch** — a same-named package with different identity | `WorldStore.cpp:338` — refused with `detail == "worldIDMismatch"`, **never substituted** |
| **6.24** | **`world.json` corrupt or unreadable** | — |
| **6.25** | **Binding exists, world never resolvable** (project bound to a world that is gone for good) | Edges hold **pending**, never pruned (SP-098/T-0380) |
| **6.26** | **Stale write lock** in the world package | 60 s stale recovery + unparseable-lock recovery (SP-097/T-0383) |
| **6.27** | **World object file present but absent from the world index** | Index rebuild-from-scan (SP-095/T-0372, AC2) |
| **6.28** | ⚠️ **Worldless project** — a worldbuilding-object operation with no world bound | `kindDirFor` refuses with `detail == "worldRequired"`; **operate silently until an operation needs a world** (user ruling 2026-08-15, the ruling that removed T-0410 as OBE) |

⚠️ **§6.22 and §6.25 must state the Epic's highest-risk property in the document's own voice:**
*absence is never deletion.* The matrix is where a future maintainer looks when deciding what a repair
pass may delete, and today it says nothing about worlds at all. **A repair pass written against the
current document could destroy a writer's entire relationship graph and satisfy every rule in it.**

**Also update §5 (External Change States)** — the state vocabulary has no world-related state. Add at
minimum `world unavailable` and `world identity mismatch`.

**Tests:** each condition gets an assertion in `ScriviCore/tests/integration/WorldTests.cpp` (or
`ExternalChangeTests.cpp` where the shape fits). **Where behaviour already has coverage, cite the existing
test rather than duplicating it** — the deliverable is that every documented condition is *backed*, not
that every condition gets a new `TEST_CASE`.

**Exit:** every new §6.x names either a new test or an existing one. Disagreements between doc and code
are **filed as Issues** (R3), not fixed here.

### T-0418 — ⚠️ Live-use pass on the real rig (new at planning)

**Codebase:** `[Apple]` · **Status:** 🔵 Backlog

⚠️ **BACK UP THE RIG FIRST.** `~/Desktop/the-stairs-of-tintagael.scrivi` and
`/Volumes/Scrivi Worlds/Eskandar.scrivworld` contain **real writing work**.

**The pass, against the AC list rather than against a script:**

1. **Create an object in each of the ten world kinds** — `artifact`, `building`, `character`, `chronicle`,
   `faction`, `item`, `location`, `map`, `rule`, `vehicle` — and a `source` in project scope. **This is
   AC1's re-verification performed by use**, and it is the reason the kind list is written out here in
   full rather than referenced.
2. **Relate them** — at least one edge from each entrance (object card and picker), including one
   **symmetric** type and one **cross-partition** `cites` edge.
3. **Eject the drive.** Confirm pending presentation, the warning view, the §7.2 footer, and that
   `offline`/`unmounted` resolve rather than degrading to `unavailable`.
4. **Reattach.** ⚠️ **Confirm restoration with NO writer intervention** — this is I-0129's exact defect
   and the AC23 clause itself.
5. **Quit and reopen** with the drive both present and absent. Confirm the graph is unchanged **and the
   edge log is byte-identical** across an absent-world save (AC-A2's "verbatim" reading).

⚠️ **Findings are filed as Issues and triaged, not silently fixed.** If the pass produces defects on the
scale SP-102's did (eight), **that is a finding about the Epic's readiness to close**, and the close
decision is the user's. **A live pass that finds nothing is a valid result; a live pass that is skipped is
not.**

### T-0391 — EP-031 verification + Epic close prep

**Codebase:** `[Cross]` · **Status:** 🔵 Backlog · ⚠️ **Owns the AC1 re-verification.**

**Per-AC pass over all ten**, each with named evidence:

| AC | State entering SP-100 | What T-0391 does |
| -- | --------------------- | ---------------- |
| **AC1** | ⚠️ **Unticked** — untick 2026-08-14 | **Re-verify under R1's amended text.** Amend the Epic file first, then verify. |
| AC2–AC9 | ✅ Met | **Confirm the evidence still stands** under the shipped model — do not re-derive. AC9 was met 2026-08-18. |
| **AC10** | ⚠️ **Open** | **Verify under R2's amended text** — both architectures + sanitizers + app build. |

**Also required:**

- **Amend `Epic-active.md`** with R1's and R2's AC text **before** verifying against it. ⚠️ Code and
  evidence must not run ahead of the spec — the same discipline SP-103 applied when it amended Doc 1 §3.0
  before changing the model.
- **Amend Doc 1** (`Scrivi_Worldbuilding_Object_Model_v0_2.md`) §3.0 consequence 4, which currently says
  AC1 "must be re-verified" — it becomes *amended and re-verified*, with R1's reasoning recorded.
- **Resolve the T-0369 open question.** `Task-backlog.md:92-95` flags that EP-030 is closed but no
  Task-level verification record was written for T-0369, and asks that it be *"resolved at the next audit
  rather than assuming either way."* **SP-100 is that audit.**
- **Epic close prep** — retrospective, sprint table final states, archive plan.

⚠️ **Claude cannot close EP-031.** Close requires **direct user approval**, and so does marking any AC
Verified.

---

## 5. Carried into this sprint — do NOT read as delivered

| Item | Owed to | Note |
| ---- | ------- | ---- |
| **Source creation** — nothing in the app creates a `source` or attaches a `cites` edge | **EP-034** | ⚠️ **The `sources` card is NOT evidence for anything in SP-100.** It renders only its empty state. T-0365 closed as an explicit **PARTIAL** delivery. |
| §3.1.1's **second popup entry point** — object cards surfacing their own sources | **EP-034** | `CitationPopover` is built; `ObjectCard.swift` references it **0** times. |
| **Key equivalents** for Scene/Chapter Start-End | unassigned | T-0417 shipped as menu items; the macOS keyspace is exhausted. |
| **T-0416** — seeded relation-type vocabulary does not reach existing projects | EP-031, **unscheduled** | Deferred by user ruling 2026-08-17. ⚠️ **A live-pass finding may make this urgent** — it is how I-0125 blocked real chronicle creation. |
| **T-0400** — history log-segment rotation | EP-019, deferred | 🟢 Nice-to-have, user-ruled. Not EP-031 work. |

⚠️ **T-0418 may surface T-0416 as a blocker.** The user's real project holds a *valid* `relation-types.json`
written before the vocabulary widened, so it is loaded verbatim and **never re-seeded**. His file was
patched **by hand** to unblock I-0125. If step 2 of the live pass fails on a relation type, this is why —
and it is a scheduling decision, not a bug to fix mid-pass.

---

## 6. Exit criteria

1. **T-0390** — §6.22–§6.28 written; §5 state vocabulary extended; every condition names a backing test;
   doc/code disagreements filed as Issues.
2. **T-0418** — the live pass has **run**, all five steps, on a **backed-up** rig; findings filed.
3. **T-0391** — all ten ACs have a per-AC evidence line; **AC1 and AC10 amended in `Epic-active.md`
   before** being verified against.
4. ⚠️ **`ctest` and interop figures NAME THEIR ARCHITECTURE**, and both architectures are green
   (SP-106's standing practice — the habit whose absence let a divide-by-zero run red for 17 days).
5. **The app builds**, and ⚠️ **Scrivi is not running when `xcodebuild test` is invoked** (a live instance
   blocks the runner with a LaunchServices error — `pgrep` first).
6. **Epic close prep drafted** — retrospective, archive plan, sprint table.
7. ⚠️ **Nothing is marked Verified, and no Epic or Sprint is closed, without direct user approval.**

---

## 7. What this sprint must not do

- **Not fix defects it finds.** R3 and R4 both file rather than fix. A close sprint that writes fixes
  stops being a close sprint — SP-099's R4 is the precedent.
- **Not read a green suite as a finished Epic.** ⚠️ **Every defect that mattered in
  SP-099/SP-103/SP-104/SP-105 was found by use while the suites were green throughout**, and SP-106's
  sanitizer leg found an **unknown** defect (I-0122) on its **first** run after weeks of green.
- **Not treat the `sources` card as evidence.**
- **Not close EP-031.** Claude may mark the Epic **Complete** and draft the close; **closing it is the
  user's call.**

---

## 8. Sprint Notes

**Numbering.** SP-100 keeps its number from the 2026-08-09 renumbering and has always been scheduled to
run last. Four sprints (SP-102, SP-103, SP-104, SP-105, SP-106) were opened *after* it and closed *before*
it — all unplanned, all from use.

**Next available IDs:** Sprint **SP-107**, Task **T-0419**, Issue **I-0135**.

**The empirical question SP-100 inherits from SP-106**, stated plainly so it is answered rather than
gestured at: six EP-031 sprints (SP-093, SP-095–SP-099) were **closed on single-architecture evidence**
while x86-64 CI was red. The gate is restored, and the first sanitizer run found a real, previously unknown
defect. **"What else is latent" now has one confirmed answer and no reason to assume it is the last.**
T-0418 exists because the suites are not the place that question gets settled.

### Retrospective

*Not started — the Sprint is in Planning.*

---

*Last Updated: 2026-08-19 (**SP-100 planned — 🔵 Planning, NOT activated.** Four rulings taken, all
user-approved: **R1** AC1's "legacy 5-kind files load unchanged" clause is **struck** as superseded by the
§3.0 no-migration ruling and AC1 is **amended** to the 11-kind ruled-scope round-trip; **R2** AC10 is
**amended** to its regression half, strengthened to require **both architectures + sanitizers**, its
"existing projects open unchanged" clause struck as vacuous while no field data exists; **R3** T-0390
documents the world conditions **and** tests them against shipped behaviour, **filing** rather than fixing
disagreements; **R4** a **live-use pass on the real rig is required evidence** — ⚠️ **new Task T-0418** —
because four of this Epic's eleven sprints were unplanned and every one came from use. ⚠️ **The repair
matrix contains ZERO world coverage** across its 578 lines and 21 conditions. **Activating SP-100 needs
direct user approval.**)*
