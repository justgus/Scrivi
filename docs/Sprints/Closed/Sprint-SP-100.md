# Active Sprint

> ✅ **SP-100 CLOSED 2026-08-19 (user-approved).** EP-031's eleventh and last sprint — **its close closed
> the Epic.** All three Tasks ✅ Verified → [`Task-verified-0390-0418-0391.md`](../Tasks/Verified/Task-verified-0390-0418-0391.md).

---

## SP-100: EP-031 verification & Epic close

**Status:** ✅ **CLOSED 2026-08-19 (user-approved)** — planned, activated and closed the same day
**Epic:** EP-031 — `[ScriviCore]` Worldbuilding Object Model & Relationship Graph
**Codebase:** `[Cross]` — documentation, ScriviCore tests, and a live `[Apple]` use pass. **No new
feature code is planned.**
**Goal:** Close out EP-031's two remaining acceptance criteria — **AC1** (re-verification under the
2026-08-14 scope model) and **AC10** (regression) — document the world-package conditions the External
Change Repair Matrix has never covered, and establish by **use** rather than by suite that the Epic is
done.
**Start Date:** 2026-08-19
**End Date:** 2026-08-19
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
| **T-0390** | External Change Repair Matrix — **world-package conditions** (§6a) + tests against shipped behaviour | High | ✅ **Verified (2026-08-19)** |
| **T-0418** | ⚠️ **Live-use pass on the real rig** — all 10 world kinds, relate, eject/reattach, reopen (**new at planning, R4**) | High | ✅ **Verified (2026-08-19)** — ⚠️ steps 1–2 partially blocked, recorded |
| **T-0391** | **EP-031 verification (AC1–AC10) + Epic close prep** — includes the **AC1 re-verification** | High | ✅ **Verified (2026-08-19)** |

### Assigned Issues

| ID | Title | Severity | Status |
| -- | ----- | -------- | ------ |
| **I-0135** | A corrupt/unparseable `world.json` has no test coverage | Low | 🔴 **Open** — T-0390, **not fixed** (R3) |
| **I-0136** | ⚠️ `world.json`'s `formatVersion` is **read but never compared** | **Medium** | 🔴 **Open** — T-0390, **not fixed** (R3) |
| **I-0137** | ⚠️ **AC24's refinement can NEVER FIRE** — `packagePath` empty for exactly the worlds it must diagnose | **High** | 🔴 **Open** — T-0418, **not fixed** (R4) |
| **I-0138** | "Remove from scene" disabled for a pending object but **not explained** | Low | 🔴 **Open** — T-0418, **not fixed** (R4) |
| **I-0139** | Clicking an object title opens the editor with no evident way back to viewing | Medium | 🔴 **Open** — T-0418, **not fixed** (R4) |

⚠️ **Both were FILED, not fixed — ruling R3 working as intended.** Neither blocks SP-100, and **neither
is a regression**: both were found by writing §6a against shipped behaviour rather than against
assumption. **I-0136 is a behaviour gap**, not merely a coverage gap.

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

## 2a. Delivered — T-0390 (2026-08-19) ✅ Verified

**`docs/Scrivi_External_Change_Repair_Matrix_v0_2.md` — 578 → 809 lines. World mentions: 0 → 63.**

New **§6a — World-package conditions**, seven conditions in the matrix's established format
(*Classification / Behavior / Automatic / Suggested user actions / Do not*), each **asserted against
shipped code** and each naming the test that backs it:

| § | Condition | Backed by |
| - | --------- | --------- |
| **6a.0** | ⚠️ **The governing principle: ABSENCE IS NEVER DELETION** — three rules every condition obeys | — |
| 6a.1 | World package missing entirely | `WorldTests.cpp:218,234,277` |
| 6a.2 | ⚠️ `worldID` mismatch — a same-named package | `WorldTests.cpp:194,306` |
| 6a.3 | `world.json` corrupt or unparseable | `WorldTests.cpp:234` ⚠️ *(gap — I-0135)* |
| 6a.4 | Binding exists, world permanently unresolvable | `ObjectCApiTests.cpp:420,436,493,518,555` · `WorldTests.cpp:326` |
| 6a.5 | Stale write lock | `WorldTests.cpp:108,356,376,405` |
| 6a.6 | Object present but absent from the world index | `ObjectIndexTests.cpp:277,295,323,363,390` |
| 6a.7 | Worldless project — ⚠️ **an expected state, not a defect** | `WorldTests.cpp:338` · `ObjectCApiTests.cpp:580` |

**§5 state vocabulary extended** with `world unavailable`, `world missing`, `world identity mismatch`,
`world locked` — and a note that ⚠️ **`unavailable` and `missing` must never be collapsed**, since
guessing `missing` invites destructive remedies for an intact world on an unreachable volume.

**Evidence:** ⚠️ **all 23 test citations were verified to resolve** to real `TEST_CASE`s with matching
descriptions — a citation that did not resolve would be the very defect §6a documents. `ctest`
**520/520 macOS arm64**, 0 failed.

**Gaps found → FILED, not fixed (R3):** **I-0135** (no coverage for a corrupt `world.json`) and
⚠️ **I-0136** (`formatVersion` read but **never compared** — a newer world package parses as current;
`grep "formatVersion >"` returns nothing). **I-0136 is a behaviour gap and the more serious**: forward
compatibility cannot be retrofitted, because by the time a newer package exists in the wild the readers
that mis-parsed it have shipped — and world packages are **shared between projects and carried across
machines**, which is exactly where version skew arises.

✅ **T-0390 Verified 2026-08-19 (user-approved).**

---

## 2b. Run record — T-0418 (2026-08-19) ✅ Verified

**Run by the user on the real rig**, drive backed up beforehand.

| Step | Result |
| ---- | ------ |
| **1. Create in all 10 world kinds** | ✅ **PASS** — all ten round-trip. ⚠️ **`source` NOT created — no UI exists** (known EP-034 gap) |
| **2. Relate from both entrances** | ⛔️ **BLOCKED** — clicking a title opens the editor with no evident exit (**I-0139**); no relate UI reachable |
| **3. Eject** | ✅ **PASS** — warnings appear, footers name the world, pending rows show **names**, removal disabled. ⚠️ Two defects: **I-0138**, **I-0137** |
| **4. Reattach** | ✅ **PASS** — ⚠️ **AC23's no-intervention clause HELD.** All warnings cleared and items re-enabled **with no writer action** |
| **5. Quit / reopen ×2** | ✅ **PASS** — restarts behave as expected, drive present and absent |

### ✅ What this establishes

⚠️ **AC23 is verified by use, on hardware, and it is the clause that could not be established any other
way.** Step 4 is **I-0129's exact defect** — refresh keyed to app focus rather than the mount event — and
it held: reattaching restored the cards **with no click, no menu, no relaunch.**

**Step 1 is AC1's re-verification performed by use.** All ten world-scoped kinds round-tripped into the
`.scrivworld` package. ⚠️ **Four directories (`buildings`, `items`, `maps`, `vehicles`) were absent
beforehand** — the world was created 2026-08-14, one day before SP-104 fixed the skeleton — **and were
created on demand** (`ObjectStore.cpp:194`). **The pre-SP-104 gap is cosmetic, not blocking, and this
world is better evidence than a fresh one would have been.**

### ⚠️ What it does NOT establish

- **`source` round-trip** — no creation UI (EP-034). **AC1's eleventh kind is unverified by use**, though
  it is covered at ScriviCore level (SP-098/T-0406).
- **Step 2 entirely** — the two-entrance edge test, the symmetric type, and the cross-partition `cites`
  edge **were not exercised.** ⚠️ **AC3's and AC9's from-either-entrance clauses therefore have no live
  evidence from this pass**; they rest on their SP-096/SP-099 test coverage.

### Findings — five open, filed not fixed

**I-0137 (High)** is the one that bears on the Epic's close: ⚠️ **AC24's `unmounted`/`offline` refinement
cannot fire on real hardware.** The refinement is correct, unit-tested and correctly wired — and
`WorldStore::listWorlds` supplies `packagePath` **only when the world is `available`**, so the single
input it needs is guaranteed absent in the only case it exists for. **Every warning surface said
"unavailable"; `unmounted` appeared nowhere.**

⚠️ **AC24 was marked Verified 2026-08-17.** Whether that verification stands is **T-0391's** question.

---

## 2c. AC pass — T-0391 (2026-08-19) ✅ Verified

### Per-AC result

| AC | State | Evidence |
| -- | ----- | -------- |
| **AC1** | ✅ **Verified 2026-08-19 (user)** | **9/9** targeted tests (arm64) **+ live**: all ten world kinds round-tripped on the real rig. ⚠️ `source` core-only — no creation UI (EP-034) |
| AC2–AC8 | ✅ Met — **evidence confirmed, not re-derived** | Suites green; no change since their sprints |
| **AC9** | ✅ Met — ⚠️ **with a recorded finding** | AC23 **held live**; ⚠️ **AC24's refinement cannot fire on real hardware (I-0137)** |
| **AC10** | ✅ **Verified 2026-08-19 (user)** | arm64 **520/520** · x86-64 CI ✅ · sanitizers ✅ (2×2) · **BUILD SUCCEEDED** · **interop 99/99 in 10 suites, TEST SUCCEEDED** |

### ✅ The outstanding leg — closed 2026-08-19

The user quit Scrivi and the interop suite ran: **99/99 in 10 suites, macOS arm64, TEST SUCCEEDED.**
**AC10's four legs are all green.**

⚠️ **One observation from that run is worth more than the pass itself.** The suite contains
*"World volume status refinement (EP-031 AC24)"* — **and it passes.** That does not contradict
**I-0137**; it *is* I-0137. The refinement is correct and its tests prove it. ⚠️ **What no unit test can
see is that its caller never supplies the input it needs.** *The tests pass and the feature does not reach
the writer.*

### ✅ User ruling on AC9 (2026-08-19)

> *"The Task was verified. What was not is due to unimplemented software features."*

⚠️ **T-0389 delivered its scope and its verification stands.** I-0137's cause is in
`WorldStore::listWorlds` — a component T-0389 does not own. **Re-opening a verified Task because a
different component starves it would make verification unfalsifiable.**
**Ledger: T-0389 ✅ · AC24 ✅ · AC9 ✅ · I-0137 🔴 Open**, scheduled on its own terms.

### Also completed under T-0391

- ✅ **Doc 1 §3.0 consequence 4 amended** — records that AC1 was **amended then re-verified**, why
  re-verification alone was impossible, and what survives of the struck clause.
- ✅ **T-0369 RESOLVED** — ⚠️ **without a user ruling, because the archive answered it.**
  `Epic-EP-030.md:8` states *"All acceptance criteria AC1–AC7 Verified"* with a dated per-AC table, and
  `Sprint-SP-094.md:5` names T-0369 as their vehicle. **The record was missing, not the verification** →
  [`Task-verified-0369.md`](../Tasks/Verified/Task-verified-0369.md). *(Contrast T-0298–T-0301, where the
  archives genuinely did not answer and a user ruling was required.)*
- ✅ **Two stale rows corrected in `Epic-active.md`** — SP-102 shown 🟡 Active (closed 2026-08-18) and
  T-0389 shown 🟠 Not Verified (Verified 2026-08-17). ⚠️ **Both were found by this pass, not by the audit.**
- ✅ **AUDIT CHECK run** (exit criterion 7) — **all seven checks pass.** One apparent failure was a **false
  positive**: the grep matched R-23's *explanatory note about* the AC9 error, not the error itself.

---

## 2d. EP-031 close prep (T-0391) — ✅ APPROVED 2026-08-19

### Where the Epic stands

**11 sprints, 10 closed. SP-100 is the eleventh and is 🟡 Active with all three Tasks implemented.**

| AC | State |
| -- | ----- |
| **AC1** | ✅ **Verified 2026-08-19 (user)** — amended (R1), then verified by suite **and** by use |
| AC2–AC8 | ✅ Met |
| AC9 | ✅ Met — ⚠️ with **I-0137** recorded against it, user-ruled not to unseat it |
| **AC10** | ✅ **Verified 2026-08-19 (user)** — amended (R2, **stronger**), all four legs green |

✅ **All ten ACs verified. EP-031 close approved 2026-08-19.**

### Suites at close prep

`ctest` **520/520 macOS arm64** · x86-64 + sanitizers ✅ (CI 2×2, ScriviCore byte-identical) ·
macOS interop **99/99 in 10 suites** · app **BUILD SUCCEEDED**.

### ⚠️ Carried OUT of EP-031 — do not read as delivered

| Item | Owed to |
| ---- | ------- |
| **Source creation** — nothing in the app creates a `source` or attaches a `cites` edge | **EP-034** |
| §3.1.1's **second popup entry point** — object cards surfacing their own sources | **EP-034** |
| **I-0137** — AC24's refinement starved of `packagePath` | unassigned |
| **I-0135 / I-0136** — corrupt-`world.json` coverage; **`formatVersion` never compared** | unassigned |
| **I-0138 / I-0139** — unexplained disabled removal; editor exit discoverability | unassigned |
| **T-0416** — seeded relation types never reach existing projects | unscheduled |
| **Key equivalents** for Scene/Chapter Start-End | unassigned |

⚠️ **AC1's eleventh kind (`source`) is proven at the core and boundary but NOT by use** — no creation UI
exists. **Stated plainly rather than glossed.**

⚠️ **AC3's and AC9's from-either-entrance clauses have no live evidence from this Epic's final pass** —
T-0418 step 2 was blocked by I-0139. They rest on SP-096/SP-099 suite coverage, which is green.

### Retrospective — draft

**What this sprint was for, and whether it worked.** SP-100 was scoped as verification, and ⚠️ **it found
five defects — none of which the suites could see.** T-0390 found two by writing documentation against
shipped code; T-0418 found three by using the app on real hardware. **The suites were green throughout,
as they have been for every one of this Epic's unplanned sprints.**

⚠️ **The Epic's signature defect appeared a fifth time, in its subtlest form.** I-0137 is
*capability shipped, surface never built* — but here the capability, its unit tests **and** its call site
all exist. **Only the data path is missing.** A suite named for the very AC passes while the feature
cannot reach the writer.

**What the rulings bought.** **R3** (document *and* test, file don't fix) and **R4** (the live pass is
required evidence) were both contested at planning as possibly excessive. **Between them they produced
every finding in this sprint.** R1/R2 — amending AC1 and AC10 rather than verifying them as written —
prevented the Epic from certifying itself against clauses its own §3.0 ruling had superseded.

**What to carry forward.** ⚠️ **A passing test suite named after an acceptance criterion is not evidence
the criterion is met in the product.** I-0137 is the cleanest example this project has produced, and it
was found by a writer ejecting a drive, not by CI.

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
- **Resolve the T-0369 open question.** EP-030 is ✅ Closed and SP-094 is closed, yet **T-0369 is still
  🔵 Backlog** with no Task-level verification record. Its own note asks that it be *"resolved at the next
  audit rather than assuming either way."*
  ⚠️ **The 2026-08-19 audit did NOT resolve it** — it was not among the 31 findings, so no ruling covers
  it. **It remains T-0391's, and it is the same shape as T-0298–T-0301**: a Task left unverified inside a
  closed Epic, where the archive does not state whether the Epic's close verified it.
  ⚠️ **Claude cannot decide this. Put it to the user**, as T-0298–T-0301 was.
- **Epic close prep** — retrospective, sprint table final states, archive plan.

> ⚠️ **Evidence note added 2026-08-19 (post-audit):** **I-0118's archive entry did not exist** when this
> plan was written — it was Verified in three files with **no record anywhere** (audit finding F-03). It
> has since been **reconstructed** into batch 12 from primary sources under QA observation.
> `Epic-active.md` names it as *"available as evidence for SP-100's AC pass"*, so **T-0391 may now rely on
> it** — but should read the reconstruction banner and treat it as what it is: a record rebuilt after the
> fact, not a contemporaneous one.

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
7. ⚠️ **An AUDIT CHECK has been run** before the close prep is considered complete — now **required by
   `Epic-GUIDELINES.md` step 1** (added 2026-08-19). It is **not** a full Audit: a read-only, mechanical
   sweep (AC status agreement · evidence exists · sprint status agreement · counts re-derived ·
   table/entry parity · orphan files · ID continuity). **Its findings are ruled as part of the Epic
   close**, not in a separate session.
   > **Why this is an exit criterion and not a nicety:** the 2026-08-19 audit found **two** defects that
   > would have corrupted this Epic's close — `Epic-Documentation.md` claiming **AC9 was outstanding**
   > when it had been met, and **I-0118 having no archive entry** while being named as evidence for this
   > very AC pass. **Both were greps.**
8. ⚠️ **Nothing is marked Verified, and no Epic or Sprint is closed, without direct user approval.**

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

*Last Updated: 2026-08-19, later same day (**post-audit reconciliation — the plan is unchanged in
substance; four things were corrected or added.** (1) ⚠️ The I-0133/I-0134 exclusion note gave a **wrong
reason**, read out of stale rows (audit F-01) — corrected. (2) **T-0369's framing was stale**: it said
*"SP-100 is that audit"*, but an audit has since run and **did not resolve it** — it stays T-0391's, is
the same shape as T-0298–T-0301, and ⚠️ **must be put to the user**. (3) **I-0118's archive entry did not
exist** when this plan was written and has since been **reconstructed** — T-0391 may now rely on it,
reading its reconstruction banner. (4) ⚠️ **New exit criterion 7: an AUDIT CHECK**, now required by
`Epic-GUIDELINES.md` before an Epic close. **SP-100 remains 🔵 Planning; activating it still needs direct
user approval.** Prior note follows.)*

*2026-08-19 (**SP-100 planned — 🔵 Planning, NOT activated.** Four rulings taken, all
user-approved: **R1** AC1's "legacy 5-kind files load unchanged" clause is **struck** as superseded by the
§3.0 no-migration ruling and AC1 is **amended** to the 11-kind ruled-scope round-trip; **R2** AC10 is
**amended** to its regression half, strengthened to require **both architectures + sanitizers**, its
"existing projects open unchanged" clause struck as vacuous while no field data exists; **R3** T-0390
documents the world conditions **and** tests them against shipped behaviour, **filing** rather than fixing
disagreements; **R4** a **live-use pass on the real rig is required evidence** — ⚠️ **new Task T-0418** —
because four of this Epic's eleven sprints were unplanned and every one came from use. ⚠️ **The repair
matrix contains ZERO world coverage** across its 578 lines and 21 conditions. **Activating SP-100 needs
direct user approval.**)*
