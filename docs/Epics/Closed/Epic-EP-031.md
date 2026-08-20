# Active Epics

## EP-031: [ScriviCore] Worldbuilding Object Model & Relationship Graph

**Codebase:** `[ScriviCore]` primarily (C++ model, index, graph, C ABI) with `[Apple]` object cards on top.
**Status:** ✅ **CLOSED 2026-08-19 (user-approved).** All **11 sprints closed**; all **10 ACs verified**. SP-095 ✅ (**AC2**), SP-096 ✅ (**AC5**),
SP-097 ✅ (**AC3 + AC6 + AC8**) and SP-098 ✅ (**AC4 + AC7**) closed 2026-08-12; SP-099 ✅
(**AC16/17/21/22 + AC18/19/20; AC10 struck**), SP-103 ✅ (the scope ruling), SP-104 ✅ (post-ruling
fallout, I-0114–I-0117) and SP-105 ✅ (world search indexing, I-0118) all closed 2026-08-15; **SP-106 ✅**
(⚠️ `[Cross]` test integrity & CI trust — I-0121 + I-0122) closed **2026-08-17**, and \*\*SP-102 ✅
(AC23 + AC24 → AC9) closed 2026-08-18\*\* — each with user approval.
**Remaining: SP-100** 🔵 Planning (verification & Epic close, **runs last**; ⚠️ owns the \*\*AC1
re-verification**). ✅ **SP-102 closed 2026-08-18\*\* → [`Sprint-SP-102.md`][1],
delivering **AC23 + AC24** and therefore **AC9**. **10 of 11 sprints are closed.**

⚠️ **AC1 must be RE-VERIFIED, and it is now AMENDED.** It was UNTICKED 2026-08-14 when the object scope
model changed under it (Doc 1 §3.0), and although SP-103 delivered that change, \*\*AC1 was ticked against
the pre-T-0409 scope table, so its evidence no longer describes the shipped model.\*\* Re-verification is
**SP-100's** to own — *not* SP-103's, which is closed.

⚠️ **SP-100 planning (2026-08-19) found that AC1 and AC10 could not be re-verified AS WRITTEN** — each
carries a clause the Epic's own §3.0 no-migration ruling superseded (\*"legacy 5-kind files load
unchanged"\* and *"existing projects open unchanged"*). Verifying either would have meant asserting a
behaviour EP-031 deliberately chose not to build. \*\*Both are amended (rulings R1/R2, user-approved) and
both remain UNTICKED\*\* — the amendment is not the verification. **AC10's amendment makes it stronger**,
requiring both architectures and sanitizers. See the AC entries below and
[`../Sprints/Sprint-active.md`][2].

⚠️ SP-097 and SP-098 had their **content swapped** — worlds landed first, because two integrity tasks were
verified unbuildable without them; integrity was then built once, against real worlds.

⚠️ **The `[ScriviCore]` half REOPENED 2026-08-14** — it was complete, and a user ruling changed the object
scope model, which is core work. That reopening is now resolved: SP-103 made the change, and SP-104/SP-105
cleaned up its fallout.

⚠️ **Every defect that mattered in SP-099/SP-103/SP-104/SP-105 was found by USE, not by the test suites**,
which were green throughout. Worth remembering when SP-100's evidence is "all tests pass."

**State at the 2026-08-15 close:** `ctest` **516/516** · macOS interop **86/86** · app **BUILD SUCCEEDED**.
Issues carried across these sprints: **I-0114–I-0119 — ✅ ALL SIX NOW VERIFIED.** I-0118 and I-0119 were
verified 2026-08-14/15; **I-0114–I-0117 were verified 2026-08-17 (user-approved)** during the SP-102 / T-0415
world-availability runs, which exercised those exact surfaces end to end. All six are archived to
[`../Issues/Verified/Issue-verified-0111-0120.md`][3].
✅ **They are now available as evidence for SP-100's AC pass** — previously they were not.

**Linux is untouched** by SP-104/SP-105. The `[ScriviCore]` fixes are cross-platform and covered by `ctest`;
the `[Apple]` halves (Worlds menu, bookmarks, Spotlight donation) have no Qt counterpart, and the Linux app
donates to no search index at all.

**Spawned:** **EP-033** `[Cross]` — World Lifecycle Management (Epic backlog, 🔵 Proposed), opened from the
I-0118 Q1 ruling. Nothing in Scrivi can delete a world or its search entries, and a world with no project
bound is unreachable — Scrivi opens *projects*. Its first deliverable is the product fork: \*\*a view inside
Scrivi, or a dedicated world-management application.\*\*
⚠️ **EP-031 is now an 11-sprint Epic** (SP-095–SP-100 as planned, plus SP-102, SP-103, SP-104, SP-105,
SP-106). It grew four times, each time from use rather than from planning: SP-099 was **split** at planning
(an entire unwrapped engine layer was hidden inside the staged tasks), then **grew T-0388 + T-0408 at R4**
when live verification found the sprint had shipped a surface with no way to create data or see world context;
**SP-103** was opened when the writer's follow-up question exposed that \*no path existed for cross-project
character reuse at all\*; **SP-104 + SP-105** were both unplanned, opened from the fallout of SP-103's ruling;
and **SP-106** was opened 2026-08-16 when the user asked why GitHub had been reporting a CI error — a defect
that had been red since 2026-07-30 and invisible to every local run. ⚠️ \*\*Earlier notes in this file calling
EP-031 a "6-", "7-", "8-" or "10-sprint Epic" are superseded\*\* — they were written before those sprints
existed and are retained only as history.

⚠️ **Four of the eleven sprints were unplanned, and every one came from USE.** SP-104, SP-105 and SP-106 were
not foreseen at any planning session; SP-099 doubled at R4. That is the Epic's most durable lesson, and it
applies directly to SP-100's evidence standard.

> ⚠️ **The scope ruling (2026-08-14) is the Epic's second model change, and it is breaking.** All ten
> worldbuilding kinds move into the `.scrivworld` package; **`source` alone stays project-scoped**. A project
> becomes a *manuscript* that references world objects through the graph. **No migration pass** — Doc 3 §7 is
> retained and the test project is discarded, which is only defensible while no real data exists. See Doc 1
> **§3.0**.

> ⚠️ **The Epic's highest-risk criterion is met.** AC7 — *absence is never deletion* — is the one failure
> Doc 3 §4.6 calls **silent and unrecoverable**: a prune pass that reads "world unmounted" as "object deleted"
> destroys every relationship into that world, errors nothing, and may go unnoticed for weeks. SP-098's T-0380
> makes *pending* an explicit state distinct from *dangling*, freezes the graph toward an unavailable world in
> both directions, and tests **both branches** — world present + endpoint missing prunes; world absent does
> not, survives repeated opens, and returns on reattach.

**Goal:** Implement the approved object model — new kinds, the object index, the canonical relationship graph,
world packages — then the worldbuilding-object cards on top of EP-030's framework.
**Design:** `docs/Scrivi_Worldbuilding_Object_Model_v0_2.md` ✅ **Approved 2026-08-05** (T1–T6 ruled) +
`docs/Scrivi_World_Data_Separation_v0_1.md` ✅ **Approved 2026-08-05** (W1–W6 ruled).
**Date Created:** 2026-08-05
**Target Close Date:** TBD (6 sprints)

### Acceptance Criteria

- [x] AC1 — ✅ **VERIFIED 2026-08-19 (user-approved).** ⚠️ **AMENDED first** at SP-100 planning (ruling R1). Its verified form is now:
	  \> **All 11 object kinds round-trip in their RULED SCOPE** — the ten worldbuilding kinds (`artifact`,
	  \> `building`, `character`, `chronicle`, `faction`, `item`, `location`, `map`, `rule`, `vehicle`) in the
	  \> `.scrivworld` package, and **`source` alone** project-scoped at `objects/sources/`. \*\*`timeline`
	  \> retired.\*\*
	  \>
	  \> ⚠️ **The "legacy 5-kind files load unchanged" clause is STRUCK** — it contradicts a ruling the Epic
	  \> already took. §3.0 consequence 2 ruled **no migration pass**: `objects/characters/…` became
	  \> `<world>/characters/…` and the test project was *discarded and recreated by hand*. A legacy
	  \> project-scoped object file is therefore **not** loaded, **by design**. Re-verifying the clause as
	  \> written would mean asserting a behaviour EP-031 deliberately chose not to build.
	  \>
	  \> **What survives:** `JsonSchemaTests.cpp:812` ("a legacy object file without the SP-095 keys parses
	  \> unchanged") is still valid and still passes — a *schema-level*, kind- and scope-agnostic assertion
	  \> about absent optional keys. It was never evidence for the scope clause. Nothing else in
	  \> `ScriviCore/tests/` tests legacy loading.

	  \#\## ✅ T-0391 EVIDENCE (2026-08-19) — AC1 is satisfiable and ticks on user approval

	  **Suite evidence — 9/9 passing, macOS arm64:**
	  `serializeWorldObject / parseWorldObject round-trip for every kind` · \`world-scoped kinds are
	  identified as such`·`world-scoped kinds are refused when NO world is supplied``· ````source\` is the
	  sole kind creatable with NO world\``·`C ABI: a world-scoped object round-trips create → open →
	  delete (I-0113)`·`a source round-trips through the ABI into objects/sources/``· ``a``cites\` edge
	  relates a source to any object kind\``·`world-scoped kinds round-trip IN WORLD SCOPE\` ·
	  ```rule` is world-scoped and no longer lives under objects/``

	  **⚠️ Live evidence — T-0418 step 1, on the real USB rig (2026-08-19):** \*\*all ten world-scoped kinds
	  were created by hand and round-tripped\*\* into `Eskandar.scrivworld`. ⚠️ \*\*Four kind directories
	  (`buildings`, `items`, `maps`, `vehicles`) were ABSENT beforehand\*\* — that world was created
	  2026-08-14, one day before SP-104 fixed the skeleton — **and were created on demand**
	  (`ObjectStore.cpp:194`). **The pre-SP-104 gap is cosmetic, not blocking**, and this world was
	  stronger evidence than a fresh one would have been.

	  ⚠️ **What the live pass did NOT establish:** `source` was **not** created live — \*\*no creation UI
	  exists\*\* (the known EP-034 gap). AC1's eleventh kind rests on **ScriviCore evidence only**
	  (SP-098/T-0406, plus the two ABI tests above). **Stated rather than glossed:** every kind is proven
	  at the core and boundary; ten of eleven are additionally proven by use.

	  **Still UNTICKED pending user approval — Claude cannot mark an AC Verified.**

	  *Original text and its 2026-08-14 untick follow, retained for the record.*
	  \~\~New kinds (`building`, `vehicle`, `artifact`, `map`, `chronicle`, `faction`, `world`) round-trip;
	  legacy 5-kind files load unchanged; `timeline` kind retired. (Doc 1 AC1)\~\~
	  ⚠️ **UNTICKED 2026-08-14 — must be RE-VERIFIED under the new scope model.** It was Verified 2026-08-12
	  against Doc 1 §3's old scope table, where `character`/`location`/`item`/`building`/`vehicle`/`map` were
	  **project**-scoped. The user ruled 2026-08-14 that **all ten worldbuilding kinds are world-scoped**
	  (`source` alone stays project-scoped) — see Doc 1 **§3.0**. The round-trip assertions therefore now
	  exercise the wrong partition, and "legacy 5-kind files load unchanged" needs restating: with \*\*no
	  migration\*\* (Doc 3 §7 retained, user-ruled), legacy project-scoped files are **not** carried forward.
	  **Re-verification is T-0411 in SP-103.** The 2026-08-12 evidence below is retained for the record but no
	  longer establishes this AC.
	  \~\~✅ **Met — SP-095 + SP-097 + SP-098 (T-0406), Verified 2026-08-12.**\~\~
	  \> **Amended 2026-08-12 at SP-095 planning (user-ruled).** Two changes. **(a) `source` removed from AC1** —
	  \> sources are a *writing aid*, not worldbuilding, and belong in the Writing tab. \*\*Updated 2026-08-12:
	  \> OQ-1 is closed and `source` is back in EP-031's scope\*\* — as a full object kind with a
	  \> **`cites`/`documented-by`** relation type (Doc 1 §3.4), scheduled as a split across SP-096/097
	  \> (ScriviCore) and SP-099 (the aggregate card). AC1 should count `source` among the kinds when those
	  \> sprints land. Source→scene is **out of EP-031** → EP-032. **(b) Round-trip is staged, not simultaneous:** the
	  \> world-scoped kinds (`artifact`, `chronicle`, `faction`) are *declared* in SP-095 but gated until
	  \> a world package supplies somewhere to hold them, because Doc 3 §7 forbids the relocation pass that
	  \> creating them in `objects/` would require. **AC1 is assessable after SP-097** (world packages — the
	  \> sprint that was SP-098 before the 2026-08-12 content swap).
	  \>
	  \> **SP-095 (Verified 2026-08-12):** enum complete at 11 kinds; `building`/`vehicle`/`map` round-trip;
	  \> legacy 5-kind files load unchanged; **`timeline` retired** with `objects/timelines/timeline.meta.json`
	  \> asserted intact.
	  \> **SP-097 (✅ Verified 2026-08-12):** the 3 gated kinds — `artifact`/`chronicle`/`faction`
	  \> — now **round-trip in world scope** (T-0385), and `rule` relocated to `worlds/<worldID>/rules/`
	  \> (T-0404). **AC1's staged round-trip is complete**; `source` remained outstanding (T-0365's ScriviCore
	  \> half, SP-098), which was the only reason AC1 stayed unticked.
	  \> **SP-098 (✅ Verified 2026-08-12) — T-0406:** `source` ships as a **project-scoped**
	  \> kind at `objects/sources/` with its own schema tag and index participation, round-tripping through the
	  \> C ABI, and a `cites` edge relates it to any kind across both partitions. \*\*AC1 is now fully satisfiable
	  \> — it ticks on verification.\*\*
	  \> ⚠️ En route, `source` exposed a **duplicated kind list**: `scrivi_c_api.cpp` carried its own copy of the
	  \> kind table, so the new kind passed every enum, schema, and index site and was still rejected at the
	  \> boundary. It now delegates to `objectKindFromName` — the same shape of defect as I-0113, a boundary
	  \> re-stating what the core already knows.
- [x] AC2 — `objects/index.json` is built on open, updated atomically, and **rebuilt from a scan** when
	  missing/stale/corrupt; `findByID` resolves via the index. (Doc 1 AC2–AC3)
	  ✅ **Met — SP-095 (T-0372 + T-0401), Verified 2026-08-12.** All three rebuild triggers covered (missing;
	  corrupt ×5 branches; stale ×2), plus idempotence and skip-not-fatal. The zero-scan property is proven
	  behaviorally by a `listDirectory`-counting filesystem decorator, not merely asserted.
- [x] AC3 — **One canonical edge** per relationship, created from either endpoint, with the inverse as a
	  read-time label projection. Duplicate rejection tested for **asymmetric and symmetric** (faction↔faction)
	  types. (Doc 1 AC4–AC5) ✅ **Met — SP-096 (T-0375) + SP-097 (T-0385), Verified 2026-08-12.**
	  \> **SP-096 (✅ Verified 2026-08-12):** canonical edges, both-direction label projection
	  \> from one record, and duplicate rejection from either creation order all shipped and tested.
	  \> ⚠️ **AC3 stays unticked on ONE clause only:** it names **faction↔faction** as the symmetric case, but
	  \> `faction` was world-scoped and uncreatable until worlds existed. SP-096 covered the same-kind symmetric
	  \> *shape* with `sibling-of` (character↔character, lexical).
	  \> ✅ **SP-097 closed the clause (Implemented, Not Verified 2026-08-12):** with `faction` creatable, the
	  \> **faction↔faction "at war with"** duplicate test now exists and passes — created from both ends, one
	  \> canonical edge, `detail == "duplicateEdge"`.
- [x] AC4 — Cascade-prune on delete; **orphans survive** and are findable; `objectID` preserved across
	  `item`→`artifact` promotion with **zero edges rewritten**. (Doc 1 AC6–AC8)
	  ✅ **Met — SP-098 (T-0377 + T-0378 + T-0379), Verified 2026-08-12.**
	  \> **SP-098 (✅ Verified 2026-08-12) — T-0377 + T-0378 + T-0379.** All three clauses:
	  \> cascade-prune fires on object **and** scene **and** chapter delete (the chapter path collects its scene
	  \> IDs *before* `remove_all`, since afterwards there is nothing left to read them from), with a load-time
	  \> repair pass behind it for the crash-between-write-and-tombstone case. Orphans are **retained**, not
	  \> swept — deleting an object prunes the edge and leaves the far endpoint intact and findable via
	  \> `scrivi_list_orphaned_objects`.
	  \> **The promotion proof is asserted the strong way:** the test compares the relationship log
	  \> **byte-for-byte** across an `item`→`artifact` promotion, not merely that edges still resolve. A rewrite
	  \> that happened to produce working edges would still have falsified the bare-endpoint premise (§5.2).
	  \> EP-027's existing scene/chapter delete suites stayed green throughout — the sprint's stated gate.
- [x] AC5 — `relationships.jsonl` compacts at **30% or 1,000 tombstones**, whichever first; torn final line
	  detected and truncated. (Doc 1 AC9) ✅ **Met — SP-096 (T-0374 + T-0376), Verified 2026-08-12.**
	  \> **SP-096 (✅ Verified 2026-08-12):** both triggers tested **independently** — a 6-record
	  \> graph at 33% (under the absolute bound) and a 5,001-record log at 20% with 1,001 tombstones (under the
	  \> ratio) — plus a below-both-thresholds case asserting **no** compaction, torn-final-line truncation, and
	  \> crash-safety (a stray `.tmp` never displaces the real log).
- [x] AC6 — The **epoch chain** resolves (event → timeline → world → project); rebinding a world changes exactly
	  one number; two timelines in one world relate without any project. (Doc 1 AC13–AC15)
	  ✅ **Met — SP-097 (T-0384), Verified 2026-08-12.**
	  \> **SP-097 (✅ Verified 2026-08-12) — T-0384.** All three clauses tested: the chain resolves
	  \> as `timeline.epochOffsetMs + binding.epochOffsetMs`; two timelines in one world relate by pure
	  \> world-relative arithmetic **with no project involved**; rebinding changes **one** number and rewrites no
	  \> timeline offset. Plus: editing a binding **never mutates `world.json`**, asserted by byte-comparing the
	  \> file — the world's epoch is intrinsic and travels with it when shared.
- [x] AC7 — ⚠️ **Absence is never deletion:** an unavailable world holds edges **pending** — never pruned, never
	  modified, surviving save, restored on reattach. Status reports offline/unmounted/missing where
	  determinable, else generic unavailable. (Doc 1 AC16–AC17, Doc 3 AC-A1–A7)
	  ✅ **Met — SP-098 (T-0380), Verified 2026-08-12.**
	  \> **SP-098 (✅ Verified 2026-08-12) — T-0380.** `ResolvedEndpoint` now answers
	  \> `pending()` and `dangling()` as **distinct** states rather than leaving callers to infer from `found`
	  \> alone, and every prune path consults it first. AC-A1–A7 each have their own test, and \*\*both branches of
	  \> AC-A5 are tested explicitly**: world present + endpoint missing **prunes**; world absent does **not\*\*,
	  \> survives repeated opens, and returns on reattach with no repair pass.
	  \> AC-A2 is asserted as "**verbatim**" — the edge log is byte-compared across a save with the world away,
	  \> because a tombstone that was later compacted out would satisfy a weaker reading and still have lost the
	  \> writer's data. AC-A4's freeze is symmetric: `scrivi_create_edge` **and** `scrivi_delete_edge` refuse with
	  \> `detail == "worldPending:<status>"`, never a silent drop. AC-A7's names come from `binding.cachedIndex`.
	  \> **Build order honoured (R2):** T-0380's guard was written, built, and tested *before* any prune code
	  \> existed — T-0377's dangling-branch test failed until cascade-prune landed on top of it.
- [x] AC8 — World packages: `worldID`-verified resolution, no search/registry, platform-neutral bindings,
	  lock→write→unlock with stale-lock recovery. (Doc 3 AC-P1–P4, AC-L1–L5)
	  ✅ **Met — SP-097 (T-0381/T-0382/T-0383 + T-0403), Verified 2026-08-12.**
	  \> **SP-097 (✅ Verified 2026-08-12) — T-0381/T-0382/T-0383 + T-0403.** Self-contained
	  \> packages; **identity-verified** resolution (a same-named package with a different `worldID` is refused,
	  \> not substituted — tested); no search, no registry; platform-neutral references (no bookmarks in the
	  \> model); and lock→write→unlock with **60 s stale recovery**, unparseable-lock recovery, and
	  \> contention that reports rather than hangs. ⚠️ Required a new **`createFileExclusive`** primitive
	  \> (**T-0403**) — Doc 3 §6.5 assumed an `AtomicWrite` exclusive-create path that does not exist, and
	  \> `rename` overwrites, so a lock built on it would have let two writers both win.
- [x] AC9 — Worldbuilding-object cards on EP-030's framework: unfiltered picker, in-stack creation with no modal,
	  "Remove from scene" deletes the edge only. (Doc 2 AC16–AC24)

	  \> ⚠️ \*\*T-0391 FINDING (2026-08-19) — AC9 stays MET, but AC24's diagnostic half does not work on real
	  \> hardware, and that must not be recorded silently.\*\*
	  \>
	  \> **What T-0418's live pass established:** with the drive ejected, **every** warning surface reported
	  \> *"unavailable"*. **`unmounted` appeared nowhere.** Root cause traced to **I-0137**:
	  \> `WorldVolumeStatus.refine` is correct, unit-tested and correctly wired, but guards on
	  \> `packagePath.isEmpty` — and `WorldStore::listWorlds` populates `packagePath` \*\*only when
	  \> `status == available`**. ⚠️ **The one input the refinement needs is guaranteed absent in the only
	  \> case it exists for.\*\*
	  \>
	  \> **Why AC9 nonetheless stays ticked, stated explicitly so the reasoning can be challenged:**
	  \> AC9 is \*"worldbuilding-object cards on EP-030's framework: unfiltered picker, in-stack creation with
	  \> no modal, Remove-from-scene deletes the edge only."\* \*\*Every clause of that sentence is met and was
	  \> confirmed live.\*\* AC24's contribution to AC9 is that pending state is **presented** — and it is:
	  \> cards show the §7.2 footer naming the world, pending rows show **names not bare IDs**, removal is
	  \> disabled, and ⚠️ **AC23's no-intervention reattach HELD**. What fails is the *specificity* of one
	  \> status word.
	  \>
	  \> ⚠️ **The honest statement of the residue:** Doc 2 AC24 asks for `offline`/`unmounted` \*\*where
	  \> determinable**, degrading to generic `unavailable` otherwise. The shipped behaviour **always\*\*
	  \> degrades — so it satisfies the letter of the fallback while never delivering the refinement the
	  \> clause was written to add. **That is a defect (I-0137), not a failed AC** — but a reader who sees
	  \> only "AC9 ✅" would not know it, which is why it is written here.
	  \>
	  \> ⚠️ **AC24 was marked Verified 2026-08-17 on fixture evidence that could not have caught this** — a
	  \> fixture supplying a `packagePath` passes where the real rig cannot. \*\*Recorded as a lesson about the
	  \> evidence, not only about the code.\*\*
	  \>
	  \> ### ✅ USER RULING 2026-08-19 — the verification STANDS
	  \>
	  \> \> *"I agree with the judgement call for AC9. *\*The Task was verified. What was not is due to
	  \> \> unimplemented software features.\**"*
	  \>
	  \> ⚠️ **This is a sharper framing than the one it settles, and it generalises.** T-0389 delivered what
	  \> it was scoped to deliver, and \*\*its verification is not retroactively falsified by a defect in a
	  \> dependency it does not own.\*\* I-0137's cause is in **`WorldStore::listWorlds`** — it withholds
	  \> `packagePath` for unavailable worlds — **not in the refinement T-0389 built.** Re-opening a verified
	  \> Task because a *different* component starves it would make verification unfalsifiable: no Task could
	  \> ever stay verified.
	  \>
	  \> **So the ledger reads:** **T-0389 ✅ Verified** · **AC24 ✅ Met** · **AC9 ✅ Met** ·
	  \> **I-0137 🔴 Open** — an unimplemented data path, owned by nobody yet, scheduled on its own terms.
	  \>
	  \> ⚠️ **The interop suite makes the point concrete.** It contains a suite literally named \*"World
	  \> volume status refinement (EP-031 AC24)"\* — and **it passes**. The refinement is correct and proven.
	  \> **What no unit test can see is that its caller never supplies the input it needs.** \*The tests pass
	  \> and the feature does not reach the writer\* — which is this Epic's signature defect
	  \> (`project_capability_without_surface`) appearing for the fifth time, in its subtlest costume yet.

	  ✅ **MET — SP-102 closed 2026-08-18 (user-approved).** AC23 + AC24, its last two clauses, were
	  ✅ Verified 2026-08-17 on the real USB rig; the runs produced **I-0123–I-0130, all Verified**, of
	  which **I-0129 is the AC23 no-intervention clause itself**. Record:
	  [`Sprint-SP-102.md`][4].
	  ⚠️ **One clause of Doc 2 §3.1.1 is carried out, not delivered:** the `sources` card ships able to
	  render only its **empty state**, because nothing in the app creates a `source` object — owed to
	  **EP-034 `[Cross]` Object Detail & Media**. This does not affect AC9 (object cards), but
	  **SP-100 must not read the `sources` card as evidence of anything.**
	  **How it was delivered:** split across two sprints at SP-099 planning (2026-08-13, R2). **SP-099**
	  delivered Doc 2 **AC16/17/21/22** (object cards, unfiltered picker, one canonical edge from either
	  entrance, "Remove from scene" deletes the edge only) — **and also AC18/19/20** (no-modal in-stack
	  create/edit, edit-state visuals, complete-or-discard), which landed early as T-0388 at R4.
	  **SP-102 then delivered AC23/24**: pending presentation, the warning view, and the
	  `offline`/`unmounted` status refinement.
	  \> ⚠️ **The Worlds menu is NOT outstanding.** It is listed under T-0389 throughout this file's older
	  \> notes, but it **shipped in SP-099 as T-0408** (added at R4, when live verification found the sprint had
	  \> no way to create data or see world context) and was extended in **SP-104/I-0117** with per-row remove
	  \> and "Locate…". Those notes predate R4 and are superseded.
	  \> ⚠️ **AC24 has unbuilt work under it that no staged task named.** `WorldStatus` declares five states, but
	  \> **`offline` and `unmounted` are produced nowhere in `ScriviCore/src`** (`WorldTypes.hpp:67-68` comments
	  \> them "platform-layer refinement") — the core emits only `missing`/`unavailable`. Since Doc 3 §4.4.1
	  \> forbids a platform-specific *model*, the refinement is Apple-layer work feeding the neutral enum. It is
	  \> scoped into **SP-102/T-0389** rather than discovered mid-sprint. The core's fallback is the honest
	  \> behavior the design mandates — this is a gap in the *diagnostic*, not a defect.
	  \> ✅ **Confirmed still unbuilt at SP-102 planning (2026-08-17)**, and **ruled (R2)**: refined via
	  \> `URLResourceValues` volume keys against `WorldEntry.packagePath` — **both** states, degrading to
	  \> `unavailable` whenever inspection is inconclusive, **never guessing `missing`** (I-0115's rule).
	  \> **No ABI change required** — `packagePath` is already exposed (`ScriviEngineGraph.swift:416`).
	  \>
	  \> ⚠️ **SP-102 planning audit (2026-08-17) — most of AC9's remaining surface was already built.** Pending
	  \> rows shown-not-hidden, **named** cached entries, the ⚠ badge, disabled-and-explained removal, the
	  \> typed `worldPending:<status>` decode, and the Worlds menu all shipped in **SP-099** (T-0386/T-0407/
	  \> T-0408) and **SP-104** (I-0117). What remains is the card-level **§7.2 footer** (the card names no
	  \> world today — only a hover tooltip), the **warning view** (`listPendingEdges` has **zero call sites**),
	  \> the AC24 refinement, and the `sources` card.
	  \>
	  \> ⚠️ \*\*AC23 is therefore close to met in code and entirely unverified — and verification IS the
	  \> criterion.\*\* AC23 is not "pending rows render"; it is \*"reattaching restores the card with no writer
	  \> intervention"\*. Split out as **T-0415** so it cannot be reported done on a fixture.
	  \>
	  \> ✅ **Pending presentation itself needs no core work:** `scrivi_list_edges_for` already returns
	  \> `otherPending`, `otherDisplayName`, and `otherWorldStatus` per row (`scrivi_c_api.cpp:891`), so Doc 2
	  \> §7.2's named-not-bare-ID requirement is satisfiable from the shipped payload.
	  \>
	  \> **AC23 verification ruled at planning (R3):** a real `.scrivworld` on a mounted **disk image**, bound
	  \> and related to a scene, then **ejected** — because AC23 requires that reattaching restores the card
	  \> *with no writer intervention*, which a fixture cannot demonstrate. Moving the package aside covers the
	  \> `missing` branch specifically; both branches need coverage since they report different statuses.
	  \> ✅ **Carried into SP-102 as its own Task, T-0415** (2026-08-17) — and **no fixture is needed**: the
	  \> rig already exists as the user's real setup, `~/Desktop/the-stairs-of-tintagael.scrivi` bound to
	  \> `/Volumes/Scrivi Worlds/Eskandar.scrivworld` on a **931 GB USB external drive** (7 cached world
	  \> entries, 9 edges). ⚠️ **The real drive is the harder test:** a `.dmg` reports
	  \> `volumeIsEjectable == true`, but this drive reports **`false`** — so the disk-image fixture would have
	  \> passed a status-detection rule that fails on the user's own hardware.
- [x] AC10 — ✅ **VERIFIED 2026-08-19 (user-approved).** ⚠️ **AMENDED first** at SP-100 planning (ruling R2) — the amended form is **stronger**. Its verified form is now:
	  \> **No regression:** `ctest` and the macOS interop suite are green \*\*on BOTH architectures (arm64 +
	  \> x86-64)\*\* and **under sanitizers**, per SP-106's standing practice; the app builds.
	  \>
	  \> ⚠️ **The "existing projects open unchanged" clause is STRUCK** — it fails for the same reason as
	  \> AC1's legacy clause (the §3.0 no-migration ruling), and additionally **has no subject**: Scrivi has
	  \> not shipped, so no field projects exist. Reconstructing a pre-SP-103 project to test against would
	  \> mean rebuilding the very thing the ruling discarded.
	  \>
	  \> ⚠️ **This amendment is only defensible because of SP-106**, which restored the x86-64 gate on
	  \> 2026-08-17. Before that date there was no second architecture to name. \*\*The amended AC10 is a
	  \> STRONGER criterion than the one it replaces, not a weaker one.\*\*

	  \#\## ✅ T-0391 EVIDENCE (2026-08-19) — all four legs green

	  | Requirement | Evidence | Status |
	  | ----------- | -------- | ------ |
	  | `ctest` green — **arm64** | **520/520**, 0 failed, macOS arm64, run 2026-08-19 | ✅ |
	  | `ctest` green — **x86-64** | CI run `32077560293`, `ubuntu-latest`, ✅ success | ✅ |
	  | **Sanitizers** | Same run — the 2×2 matrix (`macos-latest`/`ubuntu-latest` × `SCRIVI_ENABLE_SANITIZERS` ON/OFF), all legs ✅ | ✅ |
	  | App **BUILD SUCCEEDED** | `xcodebuild -scheme ScriviApp -destination 'platform=macOS'`, 2026-08-19 | ✅ |
	  | macOS **interop suite** | **99/99 in 10 suites**, macOS arm64, **TEST SUCCEEDED**, run 2026-08-19 | ✅ |

	  ⚠️ **Why the CI evidence still applies to a newer commit, checked rather than assumed:**
	  `git diff --stat ac80466b HEAD -- ScriviCore/ CMakeLists.txt .github/` returns **empty** —
	  **ScriviCore is byte-identical to the last green 2×2 run.** Every change since is Swift under
	  `Scrivi/`, which ScriviCore CI does not build. \*\*The C++ half's two-architecture + sanitizer evidence
	  is therefore current, not stale.\*\*

	  ⚠️ **BUT that is precisely why the interop suite is REQUIRED and not a formality:** all 13 changed
	  files are Swift, and **the interop suite is the only thing that exercises them.** The one leg still
	  outstanding is the one covering everything that actually changed.

	  ✅ **RESOLVED 2026-08-19.** The suite was blocked by a running Scrivi (the documented LaunchServices
	  trap); the user quit the app and the suite ran clean. **All four legs are now green.**

	  ⚠️ \**Worth recording: the interop run includes a suite named *"World volume status refinement
	  (EP-031 AC24)"*, and it PASSES.*\* That is not a contradiction of **I-0137** — it is the shape of it.
	  `WorldVolumeStatus.refine` is correct and its unit tests prove it; \*\*what no unit test can see is that
	  the caller never supplies the input it needs.\*\* The tests pass and the feature does not reach the
	  writer.

	  **AC10 is satisfiable and ticks on user approval.** ⚠️ Only the user may mark an AC Verified.
	  *Original text: \~\~No regression: `ctest` + interop suites green; existing projects open unchanged.\~\~*

### Sprints

| Sprint     | Title                                                                                                        | Status                                                                         | Dates                                           |
| ---------- | ------------------------------------------------------------------------------------------------------------ | ------------------------------------------------------------------------------ | ----------------------------------------------- |
| SP-095     | Object kinds + fields (`subtitle`/`image`/`worldID`) + object index                                          | ✅ **Closed (Human-approved)** → `../Sprints/Closed/Sprint-SP-095.md`           | 2026-08-12                                      |
| SP-096     | Relationship graph: canonical edges, relation types, append-log, compaction                                  | ✅ **Closed (Human-approved)** → `../Sprints/Closed/Sprint-SP-096.md`           | 2026-08-12                                      |
| SP-097     | **World packages: `.scrivworld`, bindings, resolution, locking, epoch chain**                                | ✅ **Closed (Human-approved)** → `../Sprints/Closed/Sprint-SP-097.md`           | 2026-08-12                                      |
| SP-098     | **Integrity: cascade-prune, orphans, promotion, ⚠️ pending-vs-dangling**                                     | ✅ **Closed 2026-08-12 (Human-approved)**                                       | **AC1, AC4, AC7 met**                           |
| SP-099     | **`[Apple]` Engine wrappers (T-0407) + object cards + picker**                                               | ✅ **Closed 2026-08-15 (user-approved)** → `../Sprints/Closed/Sprint-SP-099.md` | **AC16/17/21/22 + AC18/19/20 met; AC10 struck** |
| SP-103     | ⚠️ **`[Cross]` All worldbuilding kinds → world scope** (breaking; no migration)                              | ✅ **Closed 2026-08-15 (user-approved)** → `../Sprints/Closed/Sprint-SP-103.md` | T-0409 + T-0411; **T-0410 removed OBE**         |
| SP-104     | ⚠️ **`[Cross]` Post-ruling fallout: world reachability + the restated-kind class** (unplanned)               | ✅ **Closed 2026-08-15 (user-approved)** → `../Sprints/Closed/Sprint-SP-104.md` | I-0114–I-0117                                   |
| SP-105     | **`[Cross]` World search indexing** (unplanned)                                                              | ✅ **Closed 2026-08-15 (user-approved)** → `../Sprints/Closed/Sprint-SP-105.md` | I-0118                                          |
| SP-106     | ⚠️ **`[Cross]` Test integrity & CI trust** — I-0121 + sanitizer CI leg + macOS platform coverage (unplanned) | ✅ **Closed 2026-08-17 (user-approved)** → `../Sprints/Closed/Sprint-SP-106.md` | T-0412/T-0413/T-0414 + I-0121/I-0122            |
| SP-102     | **`[Apple]` Pending presentation + warning view + `sources` card**                                           | ✅ **Closed 2026-08-18 (user-approved)** → [`Sprint-SP-102.md`][5]              | **AC23 + AC24** — closed AC9                    |
| **SP-100** | **Verification & Epic close** — ⚠️ owns the **AC1 re-verification** (T-0390 → **T-0418** → T-0391)           | 🟡 **ACTIVE (2026-08-19)** — the Epic's **last** sprint                        | 2026-08-19 – —                                  |

> ⚠️ \**SP-099 SPLIT at planning, 2026-08-13 (R2, user-approved) — EP-031 became a 7-sprint Epic *at that
> date**\* (superseded: it is a **10-sprint\*\* Epic as of 2026-08-15 — see the note above). The staged
> SP-099 carried 5 tasks *plus* an entire unplanned engine layer, mixing plumbing, CRUD UI, and failure-surface
> work into one verification pass. **SP-099** takes the engine wrappers + object cards + picker
> (**AC16/17/21/22** of AC9); **SP-102** takes in-stack create/edit, edit-state visuals, complete-or-discard,
> pending presentation, the Worlds menu, the warning view, and T-0365's aggregate `sources` card
> (**AC18/19/20/23/24**). **SP-100 keeps its number** and runs last. SP-102 was chosen because SP-101 is
> already taken — the unplanned EP-030 AC12 sprint, closed 2026-08-11.
> 
> **The finding that forced the split:** all 12 graph/world endpoints (`create_edge`, `list_edges_for`,
> `list_worlds`, …) are exported in `scrivi.h` and **grep to zero call sites in `ScriviEngine.swift`**, which
> has object CRUD only. The graph has never been reachable from Swift. Now **T-0407**, done first, tested
> through `scrivi_*`. ⚠️ **This is SP-098's carried-forward lesson pointing the other way** — there Swift
> wrapped *more* than assumed, here *less*; the sweep, not the hypothesis, produced the truth both times.

> **⚠️ Renumbered 2026-08-09 (SP-094–SP-099 → SP-095–SP-100).** EP-031's sprints were planned before the
> 2026-08-07 ruling that made **SP-094** the merged *"EP-019 + EP-030 verification & Epic close"* sprint,
> which collided with EP-031's first sprint. `Sprint-active.md:357` already sequenced EP-031 as
> **SP-095–SP-100**; this table and the task assignments below are realigned to match. \~\~T-0365's deferral
> target moves with it: **EP-031 SP-095**.\~\~ **Superseded 2026-08-12:** T-0365 is deferred with **no sprint**
> at all (user ruling) — see the AC1 amendment above and `../Sprints/Sprint-active.md` §R2.

> **⚠️ SP-097 ⇄ SP-098 CONTENT SWAPPED (ruled 2026-08-12).** SP-097 was to be the integrity sprint and SP-098
> world packages. Planning verified that **two of the five integrity tasks were unbuildable**: **T-0379**
> (`promote_object`) has **no destination** — `artifact` is refused by `ObjectStore::checkKindStorable` until
> worlds exist — and **T-0380** (pending-vs-dangling) has **no world plumbing to interrogate**, so the Epic's
> highest-risk branch could not even be reached, let alone tested. \*\*Worlds now land in SP-097; the whole
> integrity set moves to SP-098\*\* and is built once, against real worlds. T-0377/T-0378 were not themselves
> blocked, but shipping cascade-prune *before* the pending guard that protects it is the exact ordering Doc 3
> §4.6 warns against. **Sprint IDs stay in sequence — only the content order changed.** Three deferred items
> travel with the worlds they need and land in **SP-097**: the `rule` relocation, the faction↔faction
> symmetric test (AC3's last clause), and AC1's three gated kinds.

### Tasks

| ID     | Title                                                                                                                                                | Sprint            | Status                                                                                                    |
| ------ | ---------------------------------------------------------------------------------------------------------------------------------------------------- | ----------------- | --------------------------------------------------------------------------------------------------------- |
| T-0370 | `ObjectKind` additions + `objectKindSubdir` + schema table; retire `timeline`                                                                        | SP-095            | ✅ **Verified 2026-08-12**                                                                                 |
| T-0371 | `WorldObjectFields` extensions: `subtitle`, `image`, `worldID`                                                                                       | SP-095            | ✅ **Verified 2026-08-12**                                                                                 |
| T-0372 | `objects/index.json` — build, atomic update, scan-rebuild; `findByID` over index                                                                     | SP-095            | ✅ **Verified 2026-08-12**                                                                                 |
| T-0401 | Index rebuild + corruption coverage (missing / corrupt / stale) — AC2                                                                                | SP-095            | ✅ **Verified 2026-08-12**                                                                                 |
| T-0402 | ⚠️ Endpoint-kind resolution via the object index (replaces §5.2's ID-prefix rule) + Doc 1 amendment                                                  | SP-096            | ✅ **Verified (2026-08-12)**                                                                               |
| T-0373 | `relation-types.json` + `canonicalDirection` + `symmetric`                                                                                           | SP-096            | ✅ **Verified (2026-08-12)**                                                                               |
| T-0374 | `relationships.jsonl` append-log: create/delete/list, tombstones, torn-line recovery                                                                 | SP-096            | ✅ **Verified (2026-08-12)**                                                                               |
| T-0375 | Canonical normalization + duplicate rejection (asymmetric **and** symmetric)                                                                         | SP-096            | ✅ **Verified (2026-08-12)**                                                                               |
| T-0376 | Compaction at 30% / 1,000 tombstones                                                                                                                 | SP-096            | ✅ **Verified (2026-08-12)**                                                                               |
| T-0405 | ⚠️ **I-0113** — `worldID` on `scrivi_create/open/delete_object`; world objects reachable through the ABI                                             | SP-098            | ✅ **Verified (2026-08-12)**                                                                               |
| T-0377 | Cascade-prune on object **and** scene delete + load-time repair                                                                                      | SP-098            | ✅ **Verified (2026-08-12)**                                                                               |
| T-0378 | `scrivi_list_objects` / `scrivi_list_orphaned_objects`                                                                                               | SP-098            | ✅ **Verified (2026-08-12)**                                                                               |
| T-0379 | `scrivi_promote_object` (item↔artifact), `objectID`-preserving                                                                                       | SP-098            | ✅ **Verified (2026-08-12)**                                                                               |
| T-0380 | ⚠️ Pending-vs-dangling loader distinction + frozen graph toward unavailable worlds                                                                   | SP-098            | ✅ **Verified (2026-08-12)**                                                                               |
| T-0406 | `source` object kind (T-0365 ScriviCore half) — **closes AC1**                                                                                       | SP-098            | ✅ **Verified (2026-08-12)**                                                                               |
| T-0403 | ⚠️ `FileSystem::createFileExclusive` — the exclusive-create primitive Doc 3 §6.5 assumes but that does not exist                                     | SP-097            | ✅ **Verified (2026-08-12)**                                                                               |
| T-0381 | `.scrivworld` package + `world.json` + world index                                                                                                   | SP-097            | ✅ **Verified (2026-08-12)**                                                                               |
| T-0382 | `binding.json` + `worldID`-verified resolution + relink                                                                                              | SP-097            | ✅ **Verified (2026-08-12)**                                                                               |
| T-0383 | Lock→write→unlock + heartbeat + stale-lock recovery                                                                                                  | SP-097            | ✅ **Verified (2026-08-12)**                                                                               |
| T-0384 | Epoch chain: world/timeline/binding offsets + resolve endpoint                                                                                       | SP-097            | ✅ **Verified (2026-08-12)**                                                                               |
| T-0385 | Cached world index entries → named pending entries; **world-scoped kinds become creatable** (closes AC1's gated half + AC3's faction↔faction clause) | SP-097            | ✅ **Verified (2026-08-12)**                                                                               |
| T-0404 | `rule` relocation to world scope + Package Structure §11 correction (deferred from SP-095)                                                           | SP-097            | ✅ **Verified (2026-08-12)**                                                                               |
| T-0407 | ⚠️ **`ScriviEngine` graph + world wrappers** — 14 endpoints + `ScriviError.detail` + null-result decode                                              | SP-099            | ✅ **Verified (2026-08-15)**                                                                               |
| T-0386 | Object cards (ONE implementation, ten per-kind configurations) on EP-030's framework                                                                 | SP-099            | ✅ **Verified (2026-08-15)**                                                                               |
| T-0387 | Object picker (unfiltered, all worlds) + "Create new…" entry point                                                                                   | SP-099            | ✅ **Verified (2026-08-15)**                                                                               |
| T-0408 | **Worlds menu — list / create / bind + world context** (new at R4)                                                                                   | SP-099            | ✅ **Verified (2026-08-15)**                                                                               |
| T-0409 | ⚠️ **All ten kinds → world-scoped** (`source` excluded); `ObjectStore` + C ABI                                                                       | SP-103            | ✅ **Verified (2026-08-15)**                                                                               |
| T-0411 | Test realignment under the new scope model                                                                                                           | SP-103            | ✅ **Verified (2026-08-15)** — ⚠️ **AC1 re-verification split out to SP-100**                              |
| T-0388 | In-stack create/edit, edit-state visuals, scene-change complete-or-discard                                                                           | **SP-099** (⬅ R4) | ✅ **Verified (2026-08-15)**                                                                               |
| T-0412 | Confirm the I-0121 fix on **x86-64** — the platform that actually traps the defect                                                                   | SP-106            | ✅ **Verified (2026-08-17)**                                                                               |
| T-0413 | ⚠️ **Sanitizer CI leg** — `SCRIVI_ENABLE_SANITIZERS` + 2×2 CI matrix (**found I-0122 on its first run**)                                             | SP-106            | ✅ **Verified (2026-08-17)**                                                                               |
| T-0414 | **macOS platform coverage** — `platformDefault`'s Apple branch (Linux had 7 tests, macOS zero)                                                       | SP-106            | ✅ **Verified (2026-08-17)**                                                                               |
| T-0389 | Pending **footer** (the §7.2 world-named sentence) + **warning view** + the `offline`/`unmounted` refinement (**AC24**)                              | SP-102            | ✅ **Verified (2026-08-17)** — ⚠️ but see **I-0137**: the refinement half **cannot fire on real hardware** |
| T-0365 | Aggregate `sources` card + citation popup (final third; kind + relation type already ✅)                                                              | **SP-102**        | 🟡 **Active (2026-08-17)**                                                                                |
| T-0415 | ⚠️ **AC23 live verification on the real USB world rig** — both branches (**new at SP-102 planning**)                                                 | **SP-102**        | 🟡 **Active (2026-08-17)**                                                                                |
| T-0390 | External Change Repair Matrix — world-package conditions (⚠️ matrix had **ZERO** world coverage)                                                     | SP-100            | ✅ **Verified (2026-08-19)**                                                            |
| T-0418 | ⚠️ **Live-use pass on the real rig** — 10 world kinds, relate, eject/reattach, reopen (**new at SP-100 planning, R4**)                               | SP-100            | ✅ **Verified (2026-08-19)** — ⚠️ AC23 **held**; steps 1–2 partially blocked                      |
| T-0391 | EP-031 verification (AC1–AC10) + Epic close prep — ⚠️ **amends AC1 + AC10 before verifying them**                                                    | SP-100            | ✅ **Verified (2026-08-19)**                                                            |

### Assigned Issues

| ID     | Title                                                                                                                                                                                                                                                                                                                                              | Severity | Sprint     | Status                                                                                         |
| ------ | -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | -------- | ---------- | ---------------------------------------------------------------------------------------------- |
| I-0121 | ⚠️ `[ScriviCore]` **`rebalancedKeys(1)` divides by zero — ScriviCore CI was red on every commit since 2026-07-30 (1c42838).** The `n == 0` ternary guard is unreachable while the divisor is `n - 1`; UB that **traps on x86-64 and is silent on arm64**, so local `ctest` stayed green while `ubuntu-latest` crashed.                             | High     | **SP-106** | ✅ **Resolved - Verified (2026-08-17)** → [`../Issues/Verified/Issue-verified-0121-0130.md`][6] |
| I-0122 | ⚠️ `[ScriviCore]` **`stack-use-after-scope` — a test iterated a destroyed temporary `Result`** (`Result::value()` returns a reference *into* the Result; lifetime extension does not reach it). Test code only. **Found by SP-106's own sanitizer leg on its first run**, after weeks passing green. Same arm64/x86-64 visibility split as I-0121. | Medium   | **SP-106** | ✅ **Resolved - Verified (2026-08-17)** → [`../Issues/Verified/Issue-verified-0121-0130.md`][7] |

> ⚠️ **I-0121 undermined the evidence behind several EP-031 sprint closes.** SP-093, SP-095, SP-096, SP-097,
> SP-098 and SP-099 all pushed with ScriviCore CI failing; their "ctest N/N green" figures are \*\*local arm64
> results\*\*, and the suite gated nothing on x86-64 for that window. This does not invalidate the work — the CI
> log showed **522/523 passing**, a single crashing test, not a rotten suite — but it means *"all tests pass"*
> meant *"all tests pass on one architecture"* from 2026-07-30 to 2026-08-16.
> 
> ✅ **SP-106 restored the gate, and closed 2026-08-17 (user-approved).** CI is green on x86-64 again, a
> sanitizer matrix leg now makes UB fail by diagnosis rather than by which instruction set happens to trap,
> and every leg reports its architecture. \*\*SP-100 can now rest its verification on a suite that runs clean on
> both architectures\*\* — which it could not have done before.
> 
> ⚠️ **What SP-100 must still weigh, and SP-106 sharpened rather than settled.** The six sprints above were
> *closed* on single-architecture evidence. SP-106's sanitizer leg found a **second, previously unknown**
> defect (**I-0122**) on its very first run, after weeks of green — so "what else was hidden" is now an
> **empirical** question with one confirmed answer already, not a rhetorical caution. Re-running those suites
> on both architectures is the only way to know.
> 
> **Standing practice adopted from SP-106's exit criterion 4:** ⚠️ \*\*`ctest` figures must name their
> architecture.\*\* "516/516" without a platform is the habit that let I-0121 run red for 17 days.

> ✅ **Highest-risk task: T-0380 — done and Verified (SP-098, 2026-08-12).** The pending-vs-dangling
> distinction was the one failure in this Epic that is *silent and unrecoverable* — a loader that reads "world
> unavailable" as "endpoint deleted" destroys every relationship into that world with no error shown. Both
> branches have explicit coverage, and the guard was built and tested **before** any cascade-prune code
> existed: T-0377's dangling-branch test failed until the prune landed on top of it.

---

*Last Updated: 2026-08-19 (*\*SP-100 PLANNED — 🔵 Planning, NOT activated; activating it needs direct user
approval.\*\* The last of EP-031's eleven sprints. **Four rulings, all user-approved.** ⚠️ \*\*R1 + R2 are AC
amendments, and they were forced by a finding:\*\* AC1 and AC10 **could not be re-verified as written** —
each carries a clause superseded by the Epic's own §3.0 no-migration ruling (\*"legacy 5-kind files load
unchanged"*, *"existing projects open unchanged"\*), so verifying either would have asserted a behaviour
EP-031 deliberately chose not to build. \*\*AC1 is amended to the 11-kind ruled-scope round-trip; AC10 to
its regression half, STRENGTHENED to require both architectures + sanitizers\*\* (only defensible because
SP-106 restored the x86-64 gate). ⚠️ **Both stay UNTICKED — amendment is not verification.**
**R3:** T-0390 documents the world conditions **and** tests them against shipped behaviour, **filing**
rather than fixing disagreements — ⚠️ the repair matrix has **ZERO world coverage** across 578 lines and
21 conditions, and *"absence is never deletion"* appears in it nowhere. \*\*R4: a live-use pass on the real
USB rig is REQUIRED evidence\*\* — ⚠️ **new Task T-0418** — because \*\*four of this Epic's eleven sprints
were unplanned and every one came from USE**, and SP-102's live runs alone produced eight Issues. **No AC
state changed\*\* beyond the two amendments. Prior note follows.)\*

\*2026-08-18 (**SP-102 ✅ CLOSED (user-approved) — AC9 is MET**, its last two clauses
(AC23 + AC24) Verified 2026-08-17 on the real USB rig. **10 of 11 sprints closed; only SP-100 remains**,
which owns the **AC1 re-verification** and the **AC10** regression pass — the two ACs still unchecked.
Suites at SP-102's close: `ctest` **520/520 macOS arm64**, interop **99/99 macOS arm64**.
⚠️ **One clause is carried OUT of this Epic, not delivered:** the `sources` card (T-0365) renders only
its empty state because **nothing in the app creates a `source`** — creation and §3.1.1's second popup
entry point are owed to **EP-034 `[Cross]` Object Detail & Media**, opened 2026-08-18. \*\*SP-100 must
not read the `sources` card as evidence of anything.\*\* Prior note follows.)\*

*Last Updated: 2026-08-17, later same day (*\*SP-102 planning completed — the sprint is smaller than its
staged scope.\*\* ⚠️ **A code audit found most of AC9's remaining surface already shipped** in SP-099
(T-0386/T-0407/T-0408) and SP-104 (I-0117): pending rows shown-not-hidden, named cached entries, the ⚠ badge,
disabled-and-explained removal, the typed status decode, and the Worlds menu. \*\*AC18/19/20 also already
landed\*\* as T-0388 — the older notes in this file assigning them to SP-102 are superseded. **What remains:**
the card-level §7.2 footer (the card names no world today), the warning view (`listPendingEdges` has zero call
sites), the AC24 volume refinement, and the `sources` card.
⚠️ **AC23 is close to met in code and entirely unverified — and verification is the criterion**, so it is
split out as **T-0415**, a live ejectable-disk-image run. Two rulings: **R1** the warning view gets its own
toggle rather than being anchored to the hidden-by-default Timeline (**Doc 3 §4.6 to be amended**); **R2**
AC24 refined via URL volume keys for both states, degrading to `unavailable` and never guessing `missing` —
**no ABI change required**. **No AC state changed.** Prior note follows.)\*

\*2026-08-17 (**SP-106 ✅ CLOSED (user-approved) — 9 of 11 sprints closed; SP-102 activated.**
All 3 Tasks (T-0412/T-0413/T-0414) and both Issues (**I-0121, I-0122**) ✅ Verified and archived —
`../Tasks/Verified/Task-verified-0412-0414.md` and the new `../Issues/Verified/Issue-verified-0121-0130.md`.
**ScriviCore CI is green on x86-64 for the first time since 2026-07-30**, a 2×2 sanitizer matrix now makes UB
fail by diagnosis rather than architecture accident, and macOS gained its first `platformDefault` coverage.
⚠️ **The sanitizer leg found a second unknown defect (I-0122) on its first run** — SP-100's evidence question
is now empirical, with one confirmed answer.
**SP-102 is now 🟡 Active**, delivering **AC23 + AC24**, the last two clauses of **AC9**. ⚠️ \*\*Scope
correction recorded in the AC9 note:\*\* the Worlds menu is **not** outstanding under T-0389 — it shipped as
SP-099/T-0408 and was extended by SP-104/I-0117; the older notes in this file predate SP-099's R4.
**No AC state changed** — AC1 stays unticked and owned by SP-100, AC9 in progress, AC10 open. Prior note
follows.)\*

\*2026-08-16 (**Consistency audit — three corrections to this file.** (1) The close summary said
*"Issues verified across these sprints: I-0114–I-0119"* — \*\*only I-0118 and I-0119 are Verified; I-0114–I-0117
remain `Resolved - Not Verified`\*\* and are not evidence for any AC until the user verifies them. ⚠️ \*\*A closed
Sprint was being read as verifying its Issues.\*\* (2) The file called EP-031 both an **8-sprint** Epic (top) and
a **7-sprint** Epic (SP-099 split note) while its own sprint table listed **10** — the header now states 10 and
the split note is date-scoped as history. (3) The same wrong I-0114–I-0119 claim was corrected in
`../Sprints/Sprint-active.md` and `../Sprints/Sprint-Documentation.md`. **No AC state changed** — AC1 stays
unticked and owned by SP-100, AC9/AC10 stay open. Prior note follows.)\*

\*2026-08-13 (**SP-099 planned — EP-031's first `[Apple]` sprint; the Epic is now 7 sprints.**
⚠️ **The audit found an entire engine layer hidden inside the staged tasks:** all 12 graph/world endpoints —
`create_edge`, `delete_edge`, `list_edges_for`, `list_pending_edges`, `list_objects`,
`list_orphaned_objects`, `promote_object`, `list_worlds`, `get_world_status`, `relink_world`,
`remove_world_reference`, `list_relation_types` — are exported in `scrivi.h` and \*\*grep to zero call sites in
`ScriviEngine.swift`\*\*, which carries object CRUD only. The graph has never been reachable from Swift. Issued
as **T-0407**, done first, tested through `scrivi_*` per the I-0113 habit. \*\*This is SP-098's carried-forward
lesson pointing the other way\*\* — there Swift wrapped *more* than R1 assumed, here *less*.
⚠️ **Second finding: `WorldStatus::offline` and `::unmounted` are declared but produced nowhere** in
`ScriviCore/src`, so **AC24 rests on unbuilt Apple-layer work** no staged task named; scoped into SP-102.
✅ Recorded the other way: pending *presentation* needs **no** core work — `list_edges_for` already returns
`otherPending`/`otherDisplayName`/`otherWorldStatus`. Three rulings: **R1** engine wrappers as their own first
task; **R2** SP-099 **split**, **SP-102 created** (SP-101 was taken), SP-100 keeps its number and runs last;
**R3** AC23 verified live via an **ejectable disk image**, ruled now so SP-102 does not stall. Prior note
follows.)\*

*2026-08-12 (*\*SP-098 ✅ CLOSED (Human-approved) — the graph is self-consistent under deletion,
and the `[ScriviCore]` half of EP-031 is complete.\*\* Archived to `../Sprints/Closed/Sprint-SP-098.md`;
`Sprint-active.md` reset. **4 of 6 sprints closed, 8 of 10 ACs met** — only AC9/AC10 (the Apple cards) remain.
**Next: SP-099**, the Epic's first `[Apple]` sprint. All 6 tasks ✅ Verified: **T-0405** closes ⚠️ **I-0113** (world objects were unreachable through the
C ABI); **T-0380** makes ⚠️ **pending ≠ dangling** an explicit state and freezes the graph toward an
unavailable world in both directions; **T-0377** cascade-prunes on object, scene, and chapter delete with a
load-time repair pass behind it; **T-0378** adds the object/orphan queries; **T-0379** proves promotion leaves
the edge log **byte-identical**; **T-0406** lands `source`. \*\*AC1, AC4, and AC7 are now satisfiable and tick on
verification — 8 of 10.\*\* ctest **510/510 macOS** (+33, all through `scrivi_*`) and \*\*517/517 Linux
(GCC 14.2, zero warnings)**, interop **56/56\*\*. Two findings the plan missed: `ScriviEngine.swift` wrapped all three widened endpoints (R1 assumed only tests
did — the Swift wrappers took a defaulted `worldID`), and a **duplicated kind list** in `scrivi_c_api.cpp`
rejected `source` after every other site accepted it. Both are the I-0113 shape: a boundary restating what the
core already knows. Prior note follows.)\*

\*2026-08-12 (**SP-097 ✅ CLOSED (Human-approved) — worlds exist.** Archived to
`../Sprints/Closed/Sprint-SP-097.md`; `Sprint-active.md` reset. **3 of 6 sprints closed, 5 of 10 ACs met.**
**Next: SP-098** — integrity, including ⚠️ **T-0380 pending-vs-dangling**, the one failure Doc 3 §4.6 calls
*silent and unrecoverable*; it is now fully buildable. Delivery detail follows.
**SP-097's 7 tasks ✅ Verified.**
**AC6 and AC8 are now MET**, and **AC3 is MET** — its faction↔faction clause closed once `faction` became
creatable. **AC1's staged round-trip is complete**; it stays unticked only because `source` (T-0365's
ScriviCore half) is still outstanding for SP-098. EP-031 now has **AC2, AC3, AC5, AC6, AC8 met** — 5 of 10.
Prior note follows.)\*

\*2026-08-12 (**SP-097 implemented — worlds exist.** All 7 tasks
(T-0381–T-0385, **T-0403**, **T-0404**) 🟠 Implemented, Not Verified. `.scrivworld` packages with their own
index; project-side bindings with platform-neutral references; **identity-verified resolution** (a same-named
package with a different `worldID` is refused, never substituted); lock→write→unlock with 60 s stale recovery;
and the three-layer epoch chain. **AC6 and AC8 are substantially met**; \*\*AC1 and AC3's outstanding clauses are
CLOSED\*\* — the 3 gated kinds round-trip in world scope and the faction↔faction symmetric duplicate test now
exists. ⚠️ \*\*T-0403 was needed because Doc 3 §6.5 assumed an `AtomicWrite` exclusive-create path that does not
exist\*\* (`rename` overwrites, so two writers would both "win" the lock). Three real gaps surfaced during
implementation and were fixed: world objects were being written to the *project* index rather than the world's
own; cross-partition edges did not resolve (AC10); and Package Structure §11 claimed `objects/timelines/` was
"removed" when it holds the live project timeline. ⚠️ **10 additive `scrivi.h` endpoints**, all exported.
Suites: ctest **477/477 macOS** + **484/484 Linux (GCC 14, zero warnings)**; interop **59 passed / 0 failed**.
Prior note follows.)\*

\*2026-08-12 (**SP-096 ✅ CLOSED (Human-approved) — the relationship graph is in.**
Archived to `../Sprints/Closed/Sprint-SP-096.md`; `Sprint-active.md` reset; \*\*2 of 6 EP-031 sprints now
closed**. **Next: SP-097\*\* — integrity (cascade-prune, orphans, promotion, and ⚠️ \*\*T-0380
pending-vs-dangling**, the Epic's highest-risk task). Carried forward deliberately: the **faction↔faction
symmetric duplicate test**, the last clause blocking **AC3\*\*, which needs SP-098's world packages.
Delivery detail: all 5 tasks
(T-0373/T-0374/T-0375/T-0376/**T-0402**) ✅ **Verified 2026-08-12 (user-approved)**; sprint 🟠 Review awaiting
close approval. **AC5 met**; **AC3 met but for its faction↔faction clause** (needs SP-098).
One canonical edge per relationship with
the inverse as a read-time label projection; duplicate rejection from either creation order for both an
asymmetric cross-kind type and a symmetric same-kind type; append-only `relationships.jsonl` with torn-line
recovery; compaction on both triggers independently. **AC3 and AC5 are substantially met** — see their notes;
AC3 keeps one open clause because its named faction↔faction case needs SP-098's world packages.
**T-0402 landed first and amends Doc 1 §5.2**: the ID-prefix endpoint rule was verified **broken** against the
shipped generators (`newObjectID()` mints `character_…` for every kind; the test mock uses `obj-`/`scene-`) and
is formally withdrawn in favour of index-lookup resolution. ⚠️ **First `scrivi.h` change since EP-029** — 5
additive endpoints, all exported and confirmed via `nm`. Suites: ctest **455/455 macOS** + \*\*462/462 Linux
(GCC 14, zero warnings)**; interop **59 passed / 0 failed\**. Prior note follows.)*

*2026-08-12 (*\*Design amended — `source` relates to OBJECTS, not scenes (user ruling); OQ-1
closed; EP-032 opened.\*\* The 2026-08-05 specification that `source` is "related to scenes by ordinary edges"
was **withdrawn** as mis-specified: a citation documents an object, not a passage of text. Doc 1 gains **§3.4**
(+ `source` promoted into the §3 kinds table, `cites`/`documented-by` added to the §5.1 vocabulary as the first
type with `null` on **both** kind constraints); Doc 2 gains **§3.1.1** — the `sources` card is now \*\*ONE
aggregate card\*\* listing sources reached via *this scene's* objects with a click-through citation popup,
**not a card per source** (an aggregate can be shown/hidden as a unit in the picker). Worldbuilding-object
cards surface their own sources with the same popup. **T-0365 is unblocked**, to be scheduled as a split:
ScriviCore (kind + relation type) → SP-096/097, card → SP-099. ⏸ \*\*Source-in-manuscript — footnotes and pull
quotes — deferred to new `EP-032` `[Cross]`\*\* (Epic backlog, 🔵 Proposed): it requires rendering an object
inside scene text, which touches the Markdown scene body, `scrivi.fragment.v1`, both editors, the renderer,
export, and undo — genuinely epic-sized. Adding a source→scene type later is **additive**. Same-day doc fixes:
Doc 2's Worldbuilding tab row was missing `factions`, and its Doc 3 status was stale. Prior note follows.)\*

\*2026-08-12 (**SP-095 ✅ CLOSED (Human-approved) — EP-031's first sprint delivered.**
Archived to `../Sprints/Closed/Sprint-SP-095.md`; `Sprint-active.md` reset; the Sprint index's All-Sprints
table repaired (SP-086–SP-094 and SP-101 had closed without ever being listed — it still ended at SP-085 and
read "Next available: SP-086"; eleven rows reconstructed from `Closed/`, next available now **SP-102**).
**Next: SP-096** — relationship graph. Delivery detail: all 4 tasks ✅ Verified,
T-0370/T-0371/T-0372/T-0401. **AC2 is fully MET** — the object index builds,
updates atomically, and rebuilds from a scan when missing/corrupt/stale, with the zero-scan property proven
behaviorally by a `listDirectory`-counting filesystem decorator rather than asserted. **AC1 partially met**
(enum complete; the 3 gated kinds + the `rule` relocation remain for SP-098). Suites: ctest **432/432 macOS**
+ **439/439 Linux (GCC 14, clean compile)**, interop **59 passed / 0 failed**, app **BUILD SUCCEEDED**.
Two planning-time findings were both real and both landed in T-0370: `objects/timelines/` is **shared** with
the live project timeline (deleting it would have broken every new project — Doc 1 §3.2's "legacy overlap"
phrasing conceals this), and `ObjectStore.cpp:169` inferred kind **positionally**, now an exhaustive chain
whose final `else` is a **`static_assert`**. `scrivi.h` untouched; no pbxproj change. **Next: SP-096**
(relation types + `relationships.jsonl` + canonical edges + compaction). Prior note follows.)\*

\*2026-08-12 (**EP-031 opened 🟡 Active; SP-095 planned.** Three scope rulings taken at planning,
all user-approved. **R1 — world-scoped kinds declared but gated:** Doc 1 §3 / Doc 3 §7.2 put `artifact`,
`chronicle`, `faction`, and `rule` at `worlds/<worldID>/…`, but world packages don't land until SP-098, and
Doc 3 §7 forbids a later relocation pass ("created in world scope from the start"; Scrivi hasn't shipped, so
**no migration code is written**). SP-095 therefore declares all 8 kinds but gives a working path to only the
project-scoped ones; the rest error until SP-098. **`rule` is the live disagreement** — it ships project-scoped
at `objects/rules/` today, contradicting the design; left untouched in SP-095, relocated by T-0381.
**R2 — `source`/T-0365 removed from EP-031's near-term scope** (dropped from AC1, no sprint assigned): sources
are a writing aid, not worldbuilding. Blocked on new **OQ-1** — \*worldbuilding objects should carry multiple
sources\*, a requirement no doc states and no relation type covers; user is reviewing the existing `source`
language (Doc 1 §3 l.98–100, §11 Q2; Doc 2 l.28/92/302/585). **R3 — T-0401 added** for index rebuild/corruption
coverage, since Doc 1 §4.2 calls the rebuild path "a correctness requirement, not a convenience." Also fixed:
`Task-backlog.md` rows T-0373–T-0391 still carried pre-renumbering sprint IDs from the 2026-08-09 realignment.
Deferred to SP-098 and noted so it isn't lost: **Package Structure v0.1 §11 still documents `objects/rules/`**
and must be corrected (Doc 3 §7.2). Prior note follows.)\*

\*2026-08-11 (**EP-019 ✅ CLOSED + EP-030 ✅ CLOSED (both Human-approved) — double Epic close.**
**EP-019** (Custom Undo/Redo History & Multiple Copy Buffers): AC1–AC8 all Verified across 7 sprints.
AC2 was **amended** (auto-save retired as a commit trigger; 45 s idle-session boundary added) and design
**§4.d relaxed** — disk may lead history by at most one open typing session — both explicitly user-approved
as a design change, separately from the close. T-0217 landed the documentation in design §4.a/§4.a.1/§4.d/
§12.2/§12.8/§14/§15, package-structure §16a, and repair-matrix §6.21. **EP-030** (Scene Inspector Card
Framework): AC1–AC7 all Verified across 4 sprints. **AC12 was rescoped to soft failures** (SwiftUI cannot
catch a trapping view body) after the closing pass found `CardBodyBoundary` was a no-op under a comment
claiming it was a framework guarantee; implemented as **T-0399** (SP-101) and verified on \*\*test-fixture
evidence, user-accepted\*\* — no UI path can make a card fail. **T-0368 closed as delivered by T-0399.**
ctest **413/413** · macOS interop **56/56** (from 53) · TEST SUCCEEDED. Sprints archived: SP-092, SP-093,
SP-094, SP-101. Epics archived to `Closed/Epic-EP-019.md` and `Closed/Epic-EP-030.md`.
**Active Epics 3 → 1 (EP-031 only).** Carried past close, deliberately: **T-0400** history log-segment
rotation (🟢 nice-to-have, user ruling); EP-019 has **no large-scale perf coverage** (T-0216 closed OBE) and
`HistoryCapture` is **not in the test target**, so AC2 items 5–8 rest on live verification alone.
Prior note follows.)\*

---

*Last Updated: 2026-08-03 (*\*EP-029 ✅ CLOSED (Human-approved) — cross-boundary structured Cut/Copy/Paste
complete.\*\* Final sprint **SP-089** delivered all four `[Apple]` tasks: T-0354 (Pass A clipboard, Verified
2026-07-29), T-0357 (title-capture + chapter promotion, Verified 2026-07-29), T-0355 (Pass B structured buffers,
Verified 2026-08-03), T-0356 (AC6 reversible structured undo, Verified 2026-08-03). AC1–AC7 all met; the AC6 undo
runs **app-side** (`HistoryService` stays in-memory; new core endpoint `scrivi_fragment_uncut_paste` is the exact
inverse of paste). ScriviCore `ctest` **369/369**, macOS interop **45/45**, app **BUILD SUCCEEDED**. \*\*EP-029
archived to `Closed/Epic-EP-029.md`; SP-089 archived to `Sprints/Closed/Sprint-SP-089.md`.\*\* Active Epics: EP-019
(held pending SP-057). Prior note follows.)\*

\*2026-07-27 (**SP-085 ✅ closed (Human-approved) — EP-029 design doc APPROVED.** The EP-029
`[Cross]` design sprint delivered + got approval for `docs/Scrivi_Structured_CutCopyPaste_Design_v0_1.md`: the
`scrivi.fragment.v1` ordered-pieces schema, extract / paste-splice / cut-merge behaviour (composing EP-027
create/split + EP-028 `SceneMerger`/`ChapterMerger`), buffer-schema evolution (extend `scrivi.buffers.v1` in
place), and a one-reversible-event history shape (`structuredCut`/`structuredPaste`, undo = inverse op). Trades
ruled **T1=A · T2=A · T3=A · T4=A**; Open Questions #1–#3 resolved — no cross-window/cross-project structured
paste in v1; **caret-in-heading paste = refuse + flash the screen** (user override of the drafted silent-retarget);
divider-anchored selection normalised to the adjacent scene body. Task T-0350 Verified; docs-only. **EP-029 → SP-086**
(ScriviCore extract-fragment) next, awaiting go-ahead. Epics Active 1 (EP-029) + held/draft (EP-019 held, EP-026
Linux draft). Next available Sprint **SP-086**, Task **T-0351**. Prior note follows.)\*

*2026-07-27 (*\*SP-056 ✅ closed (Human-approved) — AC6 Verified; EP-019 held pending SP-057; new
Epic EP-029 opened.\*\* T-0213 + T-0214 both ✅ Verified live: multiple copy buffers delivered as explicit
⌘1–9/⌃1–9/⌥1–9 (copy/paste/cut) chords + app-global per-project palette + Edit/Scene/Chapter menu items +
bufferID-tagged cut event (backend schema extended); ctest 327 + interop 43 green. AC6 met. EP-019 → \*\*held
pending\*\* its final sprint SP-057 (AC2/AC7/AC8 verify + history panel + close). **EP-029** `[Cross]`
(cross-boundary structured Cut/Copy/Paste) opened to capture the gap surfaced during SP-056 verification —
manuscript-as-monolithic-document copy/cut/paste with structured buffers, cut-that-merges, paste-that-splits;
5 sprints planned (SP-085 design → SP-086 extract → SP-087 paste-splice → SP-088 cut-merge → SP-089 Apple wiring);
design doc to be written & approved first (T-0350). Epics Active 1 (EP-019, held) + Draft 2 (EP-026 Linux, EP-029);
next available Epic EP-030, Sprint SP-085, Task T-0351. Prior note follows.)\*

---

*Last Updated: 2026-07-24 (*\*EP-025 ✅ CLOSED (Human-approved) — the final sprint SP-084 delivered AC6b and
closed the Epic.\*\* SP-084 (co-located dot **clustering** — aggregate dot: larger core + count + segmented arc
ring + selection arc; hover fan-out; zoom-resolve) + a persistence sweep + full EP-025 verify; T-0346–T-0349 all
Verified live over VNC. Four clustering findings surfaced + fixed + re-verified same session (larger-diameter
aggregate-of-aggregates; fan overlay + grey backing; tighter dismiss; the **phantom double-draw** defect — a
fanned aggregate's members drew twice because the skip-set excluded the fanned aggregate; fixed so all members
are skipped from the baseline loops and the ring is the sole draw). Final container build **green (216/216)** +
new `timeline_cluster_smoke` + 14 regression smokes + app-launch PASS; `scrivi.h` untouched; no pbxproj
(Linux-only). **EP-025 archived to `Closed/Epic-EP-025.md`; Active Epics 1→0.** The full six-sprint completion
summary lives in the archive. Prior note follows.)\*

\*2026-07-24 (**EP-025 SP-082 ✅ closed (Human-approved) + SP-084 planned + activated** — the final sprint of
EP-025. SP-082 delivered **AC5** (historical events + imported timelines + export + File-menu items,
T-0340–T-0345 all Verified; I-0090/I-0091 fixed + re-verified; container green 211/211 + `timeline_events_smoke`
+ 11 smokes PASS; archived `Closed/Sprint-SP-082.md`). SP-084 then delivered **AC6b** clustering (above) and
closed the Epic. Prior EP-025 history — SP-079 [AC1/AC2], SP-080 [AC3], SP-081 [AC4], SP-083 [AC6a] — lives in
`Closed/Epic-EP-025.md`.)\*

[1]:	../Sprints/Closed/Sprint-SP-102.md
[2]:	../Sprints/Sprint-active.md
[3]:	../Issues/Verified/Issue-verified-0111-0120.md
[4]:	../Sprints/Closed/Sprint-SP-102.md
[5]:	../Sprints/Closed/Sprint-SP-102.md
[6]:	../Issues/Verified/Issue-verified-0121-0130.md
[7]:	../Issues/Verified/Issue-verified-0121-0130.md