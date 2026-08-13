## SP-098: [ScriviCore] Graph Integrity — Cascade-Prune, Orphans, Promotion, ⚠️ Pending-vs-Dangling

**Status:** ✅ **Closed (Human-approved 2026-08-12).** All 6 tasks ✅ Verified; both suites green on macOS
and Linux; retrospective reviewed by the user.
**Epic:** EP-031 `[ScriviCore]` Worldbuilding Object Model & Relationship Graph (fourth of 6 sprints)
**Goal:** Make the graph self-consistent under deletion — **without ever mistaking an absent world for a
deleted object.** Cascade-prune on object and scene delete, orphan queries, `objectID`-preserving promotion,
and the pending-vs-dangling distinction the design calls the one failure that is *silent and unrecoverable*.
**Design:** `docs/Scrivi_Worldbuilding_Object_Model_v0_2.md` §3.1, §5.5, §6 + `docs/Scrivi_World_Data_Separation_v0_1.md`
§4.6, §9 AC-A1–A7 (both ✅ Approved 2026-08-05, amended 2026-08-12).
**Start Date:** 2026-08-12 | **End Date:** 2026-08-12 | **Capacity:** ~12–14 hours

> ⚠️ **SP-097 and SP-098 had their content swapped** (ruled 2026-08-12). This is the integrity sprint that was
> originally SP-097; worlds landed first because two of these tasks were unbuildable without them. Everything
> here is now fully unblocked.

---

### Assigned Tasks

| ID     | Title | Priority | Status |
| ------ | ----- | -------- | ------ |
| T-0405 | ⚠️ **I-0113** — `worldID` on `scrivi_create/open/delete_object`; world objects reachable through the ABI | High | ✅ **Verified (2026-08-12)** |
| T-0380 | ⚠️ **Pending-vs-dangling** loader distinction + frozen graph toward unavailable worlds | High | ✅ **Verified (2026-08-12)** |
| T-0377 | Cascade-prune on object **and** scene/chapter delete + load-time repair | High | ✅ **Verified (2026-08-12)** |
| T-0378 | `scrivi_list_objects` / `scrivi_list_orphaned_objects` | High | ✅ **Verified (2026-08-12)** |
| T-0379 | `scrivi_promote_object` (item↔artifact), `objectID`-preserving, **zero edges rewritten** | High | ✅ **Verified (2026-08-12)** |
| T-0406 | `source` object kind (T-0365's ScriviCore half) — **closes EP-031 AC1** | Medium | ✅ **Verified (2026-08-12)** |

### Assigned Issues

| ID | Title | Severity | Status |
| -- | ----- | -------- | ------ |
| I-0113 | World-scoped objects unreachable through the C ABI — `scrivi_create_object` has no `worldID` | High | ✅ **Resolved - Verified (2026-08-12)** (T-0405) |

---

### Findings from the SP-098 planning audit (2026-08-12)

The user asked for an implementation-vs-design review. The shipped C ABI was diffed against both design docs,
signatures compared, and a probe linked against `libScriviCore.a` to test the boundary directly. **Three
divergences found; all three are now reconciled in the docs, and one is a real defect.**

#### F1 — ⚠️ **I-0113: world objects are unreachable through the C ABI** (a real defect → T-0405)

SP-097 added `worldID` to the object *requests* and routed storage through `ObjectStore::kindDirFor`, but the
four `scrivi_*_object` C entry points were **never widened**. `req.worldID` is therefore always empty at the
boundary and `kindDirFor` correctly refuses. **Confirmed by probe**, not inferred:

```
scrivi_create_object(root, "artifact", …)
→ {"ok":false,"message":"kind 'artifact' is world-scoped; a worldID is required"}
```

…with a world created and bound moments earlier in the same process.

> **Why the suite did not catch this.** `WorldTests.cpp` calls the **C++ facade** directly, bypassing the C
> ABI entirely. Every world-scoped assertion passes while the boundary is broken. **Lesson for T-0405: the
> regression test must go through `scrivi_*`, not `core.createObject`.** This is the second time a
> facade-only test has masked a boundary gap; worth a standing habit.

**Doc 1 §6 amended** — it asserted "New object kinds need **no** new CRUD endpoints," which was true when
written and became false when kinds acquired scope. The corrected signatures are now in the doc.

#### F2 — timeline epoch offsets ship in a sidecar, not the timeline file (doc amended; ruled)

Doc 1 §7.0 places a timeline's `epochOffsetMs` inside `historical-timelines/<slug>.json`. It ships in
`worlds/<worldID>/timeline-offsets.json` (`scrivi.world-timeline-offsets.v1`) because **those timeline files do
not exist yet** — they arrive with world *content*, which is Timeline Panel territory (EP-016/EP-025), not
EP-031. **Ruled: keep the sidecar, amend the doc.** The value is still world-relative (the load-bearing
property), and it folds into the timeline file when one exists. No code change.

#### F3 — lock `holder.host` / `holder.pid` are placeholders (doc amended)

Doc 3 §6.5 specifies `holder{host, pid, projectID}`; the implementation writes `"local"` and `0`. **Neither is
obtainable from ScriviCore** — both are platform facts and `CoreServices` exposes no provider, by design
(Architecture v0.3 keeps the core free of platform APIs). They are **diagnostic only**; nothing in the locking
protocol reads them, so exclusivity and staleness are unaffected. Doc amended to say so, with the real
consequence recorded: a contended writer cannot be told *which* machine holds the lock until a host/PID
provider exists. **Deliberately not scheduled here** — it belongs with the warning surface that would display
it (Doc 2 §7.2, SP-099).

**Also reconciled:** Doc 3 §8's `worldPackagePath` ships as `packagePath` (same meaning), and an
implementation-status note now marks `scrivi_list_pending_edges` as the one §8 endpoint still outstanding —
it is T-0380's.

---

### Scope rulings taken at planning (2026-08-12)

#### R1 — T-0405 goes first, and widens the three shipped signatures (ruled)

`worldID` is added to `scrivi_create_object`, `scrivi_open_object`, and `scrivi_delete_object` (empty string =
project scope). **`scrivi_save_object` is untouched** — the object JSON it carries already holds `worldID`.

> ⚠️ **This is the first breaking change to a shipped `scrivi.h` signature in EP-031.** Taken now because the
> only in-tree callers are ScriviCore's own tests: the Apple and Linux layers have not adopted object CRUD for
> world kinds yet. The blast radius is minimal **today** and grows with every sprint that builds on the current
> shape. The rejected alternative — parallel `*_world_object` endpoints — would permanently double the object
> CRUD surface and force every caller to branch on scope, which is exactly what bare-`{id}` endpoints exist to
> avoid.

**It goes first** because T-0379 (promotion) moves an object *between* scopes and needs the widened boundary to
be testable end-to-end.

#### R2 — T-0380 before T-0377, and they must be built together

**Cascade-prune must not ship before the guard that protects it.** Doc 3 §4.6 is unambiguous: a prune pass that
reads "world unavailable" as "endpoint deleted" destroys every relationship into that world **silently**, and
the writer may not notice for weeks. Every other hazard in EP-031 is recoverable; this one is not.

So T-0380 lands first and T-0377 is written against it — never the reverse, and never with a "we'll add the
check after" ordering.

#### R3 — `source` (T-0406) is folded in, closing AC1

T-0365's ScriviCore half is one enum entry plus a project-scoped subdir; the `cites` relation type it pairs
with **already shipped in SP-096**. It is small, it is the *only* thing keeping **EP-031 AC1** unticked, and it
belongs with the object-model work rather than with SP-099's card. The aggregate `sources` **card** stays in
SP-099.

---

### Task detail

#### T-0405 — I-0113: `worldID` at the boundary (do first)

**Files:** `include/scrivi/scrivi.h`, `src/public_api/scrivi_c_api.cpp`, plus every in-tree caller.

Three signatures widened; `scrivi_save_object` unchanged. **The regression test must exercise `scrivi_*`
directly** — a facade-level test would pass against the broken boundary, which is how this shipped.

**Done when:** a world-scoped object round-trips create → open → delete **through the C ABI**; an empty
`worldID` still creates project-scoped kinds unchanged; and the probe from the audit now succeeds.

#### T-0380 — ⚠️ pending vs. dangling (the Epic's highest-risk task)

**Files:** `src/objects/EndpointResolver.{hpp,cpp}`, new pending-edge query, `scrivi_list_pending_edges`.

The loader must distinguish **two** unresolvable cases (Doc 1 §5.5, Doc 3 §4.6):

| Case | Cause | Action |
| --- | --- | --- |
| **Dangling** | endpoint gone, its world (or the project) **is present** | prune |
| **Pending** | endpoint's **world is unavailable** | hold — never prune, never modify |

> **Build on what SP-097 already shipped, do not add a parallel notion.** `EndpointResolver` deliberately
> leaves `found == false` on its cached-index path — the object is *named but unverified* while its world is
> away. `ResolvedEndpoint` should gain an explicit `pending` flag (plus the `WorldStatus`) so callers stop
> inferring state from `found` alone. `WorldStatus` is already the vocabulary for "why can't I see this?"

**Hard rules (Doc 3 §4.6), each its own test:**
1. **No edge into an unavailable world may be added or removed** — the graph is frozen toward that world, both
   directions, and refusal is explicit (`detail`), never a silent drop.
2. **Pending edges survive save** verbatim.
3. **Reattaching the world restores them** with no repair pass and no writer action.
4. **Pending entries display names** from `binding.cachedIndex`, not bare IDs (AC-A7) — the reason the cache
   exists at all: a writer asked whether to clear references cannot decide blind.

**Done when:** AC-A1–A7 all pass, and **both branches are tested explicitly** — *world present + endpoint
missing* prunes; *world absent* does not.

#### T-0377 — cascade-prune + load-time repair

`scrivi_delete_object` and the **scene/chapter delete paths** append tombstones for every edge referencing the
deleted ID, in the same operation. A load-time repair pass drops edges whose endpoints no longer resolve —
**consulting T-0380's distinction first.**

> ⚠️ **This reaches into EP-027's shipped delete paths** (`SceneDeleter`, `ChapterDeleter`). Ruled at SP-097
> planning: both object *and* scene delete, per §5.5. Small and well-covered, but it is shipped code — the
> existing delete tests must stay green.

**Done when:** deleting an object tombstones its edges atomically; deleting a scene does the same; a load-time
repair drops genuinely dangling edges; and **an edge into an unavailable world is never pruned** by any path.

#### T-0378 — object and orphan queries

`scrivi_list_objects(projectRoot, kindOrNull)` and `scrivi_list_orphaned_objects(projectRoot)`.
**Orphan** = present in the index, absent from every endpoint in the edge map (§5.5). Both are pure reads over
resident structures — O(n), no I/O.

> **Orphans are deliberately retained, not cleaned up.** An object with no relationships is a legitimate
> creative state — a character sketched before they have a scene. Pruning the *edge* while keeping the
> now-unrelated *object* is the correct behaviour; these endpoints exist to make that state **findable**.

**Done when:** an object survives deletion of its last edge and is returned by the orphan query; `kindOrNull`
filters; world objects appear when their world is available.

#### T-0379 — `scrivi_promote_object` (item ↔ artifact)

Moves the file (`objects/items/…` ↔ the world's `artifacts/`), sets/clears `worldID`, and **preserves
`objectID` unchanged**. Demotion is the exact inverse through the same endpoint.

> **This is the promotion-safety proof for the whole bare-endpoint ruling (§3.1, §9 AC8).** Edge endpoints
> dropped `kind` precisely so promotion would not stale them. The test must assert **zero edges rewritten** —
> not "edges still resolve", but that the edge log is byte-identical across the promotion.

**Done when:** an `item` with existing edges promotes to a world `artifact`, `objectID` is unchanged, **the
relationship log is byte-identical**, and every edge still resolves. Demotion likewise.

#### T-0406 — the `source` object kind

`source` added to `ObjectKind` as **project-scoped** (`objects/sources/`), with its schema tag and index
participation. The `cites`/`documented-by` relation type already shipped (SP-096 T-0373).

**Done when:** `source` round-trips through the ABI; a `cites` edge relates a source to any object kind; and
**EP-031 AC1 is fully satisfiable**.

---

### Out of scope — explicitly

| Not in SP-098 | Where it lands |
| --- | --- |
| The aggregate `sources` **card** (T-0365's Apple half) | SP-099 |
| Worlds menu, warning view, pending **presentation** | SP-099 |
| Populating lock `holder.host`/`holder.pid` (needs a platform provider) | With the warning surface — SP-099 or later |
| Historical-timeline files; folding the offset sidecar into them | Timeline Panel work, outside EP-031 |
| `scrivi_import_world` (true cross-project sharing) | Superseded by `scrivi_add_world`; not in EP-031 |
| Source→scene edges (footnotes / pull quotes) | **EP-032** |

**`scrivi.h` changes:** 3 **widened** signatures (breaking — R1) + ~4 additive endpoints
(`list_objects`, `list_orphaned_objects`, `promote_object`, `list_pending_edges`).
**No `project.pbxproj` change** — ScriviCore-only.

---

### Success criteria

1. World-scoped objects round-trip **through the C ABI** (I-0113 closed); the audit probe succeeds.
2. ⚠️ **Pending ≠ dangling**, both branches tested: world present + endpoint missing prunes; world absent does
   **not**, and the edge survives save and reattach.
3. The graph is **frozen** toward an unavailable world — add/remove refused explicitly, never silently dropped.
4. Pending entries carry **names**, not bare IDs.
5. Cascade-prune fires on object **and** scene/chapter delete; EP-027's existing delete tests stay green.
6. Orphaned objects **survive** and are findable.
7. Promotion preserves `objectID` with the **edge log byte-identical**.
8. `source` round-trips; **EP-031 AC1 fully satisfiable**.
9. `ctest` green on **macOS** (≥ 477) **and Linux/GCC** (≥ 484), plus new tests.
10. macOS interop green (≥ 59 test cases) — count test cases, **not** `Executed N`.
11. Existing projects open unchanged.

---

### Risks

| Risk | Mitigation |
| --- | --- |
| ⚠️ **Cascade-prune destroys relationships into an offline world** — silent and unrecoverable, the worst failure in the Epic. | T-0380 **before** T-0377 (R2); explicit tests on both branches; prune consults `WorldStatus` before touching anything. |
| **A facade-only test masks a boundary gap again** — exactly how I-0113 shipped. | T-0405's regression test goes through `scrivi_*`, not the C++ facade. Worth making a habit for every ABI change. |
| **The breaking ABI change strands a caller.** | Only in-tree callers are ScriviCore tests today; sweep for `scrivi_create_object`/`_open_object`/`_delete_object` across Swift, Qt, and tests before landing. |
| **Touching EP-027's shipped delete paths regresses manuscript editing.** | Additive tombstone step only; the existing scene/chapter delete suites must stay green and are the gate. |
| Sprint is large — 6 tasks, ~7 ABI changes. | T-0405 and T-0406 are small. If capacity runs short, **T-0378 (queries) is the safe carry** — it is pure read, blocks nothing. **T-0380 must not be cut.** |

---

### Implementation results (2026-08-12)

All six tasks implemented in the ruled order — **T-0405 → T-0380 → T-0377 → T-0378 → T-0379 → T-0406**.
R2 was honoured literally: the pending guard was written, built, and tested **before** any prune code existed,
and T-0377's dangling-branch test failed until cascade-prune landed on top of it.

| Task | Landed | Where |
| --- | --- | --- |
| T-0405 | 3 signatures widened + `worldID` plumbed | `scrivi.h`, `scrivi_c_api.cpp`, `ScriviEngine.swift` |
| T-0380 | `ResolvedEndpoint::pending()`/`dangling()`, frozen graph, `scrivi_list_pending_edges` | `EndpointResolver.*`, `RelationshipStore.*` |
| T-0377 | `cascadeDelete` + `repairDangling` wired into object/scene/chapter delete and project open | `ObjectStore.cpp`, `SceneDeleter.cpp`, `ChapterDeleter.cpp`, `ProjectOpener.cpp` |
| T-0378 | `ObjectIndex::loadAllVisible` + 2 endpoints | `ObjectIndex.*`, `scrivi_c_api.cpp` |
| T-0379 | `ObjectStore::promote` + `scrivi_promote_object` | `ObjectStore.*`, `scrivi_c_api.cpp` |
| T-0406 | `source` kind through enum, schema, index, ABI | `ObjectTypes.hpp`, `ObjectJson.cpp`, `ObjectIndex.cpp`, `ScriviCore.cpp` |

**Tests:** `tests/integration/ObjectCApiTests.cpp` — a new file, **33 test cases, all through `scrivi_*`**,
which is the standing habit F1 asked for. ctest **510/510 macOS** (477 baseline + 33) and **517/517 Linux
(GCC 14, zero warnings)** (484 baseline + the same 33; Linux runs 7 more because the `EncryptedFileSecureStore`
suite is Linux-only), interop **56/56**.

**Two findings the plan did not anticipate:**

1. ⚠️ **R1's blast-radius claim was wrong.** The plan said "the only in-tree callers are ScriviCore's own
   tests." `ScriviEngine.swift` in fact wraps all three widened endpoints (lines ~232–307). They were updated
   with a **defaulted `worldID: String = ""`**, so every existing Swift call site is source-compatible and the
   Apple layer gains world-scoped CRUD for free. *The lesson mirrors F1's: sweep for callers, don't assume.*

2. **A duplicated kind list silently rejected `source`.** `scrivi_c_api.cpp` carried its own copy of the kind
   table in `objectKindFromStr`, so T-0406 passed every enum, schema, and index site and still failed at the
   boundary with `unknown object kind: 'source'`. It now delegates to `objectKindFromName` — **one list, not
   two**. This is the same shape of defect as I-0113: a boundary that duplicates what the core already knows.

**Linux/GCC verified (2026-08-12).** Built and tested under **GCC 14.2** (Ubuntu 24.04 container, Ninja,
`-DSCRIVI_BUILD_TESTS=ON`): **517/517 passing, zero warnings** at the project's warning level. Success
criterion 9 is met on both platforms.

> The GCC run needed `libssl-dev` — `ScriviCore/CMakeLists.txt:104` requires OpenSSL ≥ 3 for the Linux-only
> `EncryptedFileSecureStore` (SP-059). Worth noting for any future containerised run: a bare
> `g++`/`cmake`/`ninja` image fails at **configure**, not at build.
>
> Nothing needed fixing for GCC. The two portability risks were handled preemptively while writing the code —
> designated initializers kept in declaration order (GCC enforces this, Clang does not) and an unused lambda
> parameter removed after a strict-warning pass.

---

### Notes

- **Baseline at planning:** ctest **477/477 macOS**, **484/484 Linux (GCC 14)**, interop **59 passed**.
  ⚠️ **The interop figure was stale** — the baseline was re-measured on a clean tree at `2af8446` and is
  **56**, not 59. SP-098 leaves it at 56; no interop test was lost.
- **After SP-098, EP-031 has one ScriviCore sprint's worth of work left** — AC4 and AC7 are this sprint's;
  SP-099 is Apple cards; SP-100 is the repair matrix + Epic verification.
- **Design amendments already applied** (F1–F3 above): Doc 1 §6 and §7.0, Doc 3 §6.5 and §8.

---

*Last Updated: 2026-08-12 (SP-098 planned, with the requested implementation-vs-design audit. **Three
divergences found and reconciled:** ⚠️ **I-0113** — world-scoped objects are unreachable through the C ABI
(SP-097 widened the requests but not the entry points), confirmed by probe and scheduled as **T-0405**;
timeline epoch offsets ship in a project-local sidecar rather than the timeline file (**doc amended**, ruled —
the files do not exist yet); and lock `holder.host`/`pid` are placeholders because ScriviCore has no platform
provider (**doc amended**). Rulings: **R1** widen the three shipped signatures now, while the only callers are
tests; **R2** T-0380 before T-0377 — cascade-prune must never ship ahead of the guard that protects it;
**R3** fold in `source` (T-0406), the last thing blocking AC1.)*

*Updated 2026-08-12 — **all six tasks Implemented, Not Verified.** ctest **510/510 macOS** (+33 new, all
through the C ABI), **517/517 Linux (GCC 14.2, zero warnings)**, interop **56/56** (the planning doc's "59"
baseline was stale; re-measured at `2af8446`). **Success criterion 9 met on both platforms.** Two unplanned
findings: R1 understated the blast
radius (`ScriviEngine.swift` wraps all three widened endpoints; updated with a defaulted `worldID`), and a
duplicated kind list in `scrivi_c_api.cpp` rejected `source` at the boundary after every other site accepted it
— now delegating to `objectKindFromName`, one list instead of two.*

---

## Close-out (2026-08-12, Human-approved)

### Verification

| Check | Result |
| --- | --- |
| ScriviCore `ctest` (macOS) | ✅ **510/510**, 0 failures (baseline 477; **+33**) |
| ScriviCore `ctest` (**Linux / GCC 14.2**) | ✅ **517/517**, 0 failures, **zero warnings** (baseline 484; **+33**) |
| SP-098 tests specifically | ✅ 33 test cases, **all through `scrivi_*`** |
| macOS interop | ✅ **56 passed / 0 failed** |
| EP-027 scene/chapter delete suites | ✅ green — the sprint's stated gate for touching shipped delete paths |

> Linux runs 7 more cases than macOS because the `EncryptedFileSecureStore` suite is Linux-only (SP-059).
> The GCC run needs `libssl-dev` — `ScriviCore/CMakeLists.txt:104` requires OpenSSL ≥ 3 for that store, so a
> bare `g++`/`cmake`/`ninja` image fails at **configure**, not at build. Worth knowing before the next
> containerised run; the project's own `platforms/linux/docker/Dockerfile` builds with `SCRIVI_BUILD_TESTS=OFF`
> (it is the Qt GUI image) and is not the right vehicle for ctest.

### Retrospective

**What went well**

- **R2's ordering earned its keep, and it was provable.** T-0380's guard was written, built, and tested with no
  prune code in the tree; T-0377's dangling-branch assertion then failed until cascade-prune landed on top of
  it. The sequencing was not merely respected — the test log shows the guard existing first.
- **The strong assertions were chosen deliberately over the convenient ones.** AC8 compares the relationship
  log **byte-for-byte** across a promotion rather than checking that edges still resolve, and AC-A2 does the
  same across a save with the world away. Both weaker readings would pass while the property under test was
  false — a rewrite that happens to produce working edges still falsifies the bare-endpoint premise, and a
  tombstone later compacted away still loses the writer's data.
- **F1's lesson was applied as a rule, not a note.** Every one of the 33 new tests goes through `scrivi_*`.
  The file exists because a facade-only suite cannot see a boundary gap, and it says so at the top.
- **Portability was handled while writing rather than after.** Designated initializers kept in declaration
  order and a strict-warning pass over the touched files meant GCC 14 built clean on the first attempt, with
  zero warnings and nothing to fix.

**What didn't**

- ⚠️ **R1's blast-radius claim was simply wrong.** The plan asserted "the only in-tree callers are ScriviCore's
  own tests." `ScriviEngine.swift` wraps all three widened endpoints. The sweep the risk table asked for was
  what caught it — had the ruling been trusted instead of the sweep, the macOS build would have broken.
- ⚠️ **A duplicated kind list rejected `source` at the boundary.** `scrivi_c_api.cpp` carried its own copy of
  the kind table, so T-0406 passed the enum, the schema, the index, and the search extractor, and still failed
  with `unknown object kind: 'source'`. **This is the same defect shape as I-0113** — a boundary re-stating
  what the core already knows — found twice in one Epic, two sprints apart.
- **The planning baseline contained a stale number.** Interop was recorded as 59; the real figure at `2af8446`
  is 56. Nothing was lost, but a wrong baseline is indistinguishable from a regression until someone measures
  the tree it supposedly describes, which cost a stash-and-remeasure cycle mid-sprint.
- **The first Linux attempt failed at configure, not build.** Two container rounds were spent discovering the
  OpenSSL dependency. Now recorded above.

**What to adjust**

1. ⚠️ **Grep for duplicated dispatch before adding anything to `ObjectKind`.** Twice now a boundary has kept a
   private copy of what the core knows. `objectKindFromStr` now delegates, but the next enum-shaped addition
   should start by searching for other lists, not by editing the enum.
2. **Treat "the only callers are X" as a hypothesis, not a fact.** Cheap to check, and R1 was wrong about it.
3. **Re-measure a baseline before quoting it in a close-out.** A figure copied forward across sprints is not
   evidence; the 59 had been carried since a tree that no longer existed.
4. **Carry the boundary-test habit into SP-099.** It is Apple-side, which is exactly where an ABI-shaped gap
   would hide next — and where I-0113 would have surfaced had it not been caught by audit first.

**Deferred / not in this sprint** — unchanged from the plan: the aggregate `sources` card, the Worlds menu and
pending *presentation*, and populating lock `holder.host`/`pid` (still needs a platform provider, and still
belongs with the warning surface that would display it). Nothing was returned to the backlog; all six tasks
completed.

### Epic impact

**EP-031 AC1, AC4, and AC7 are met** — 8 of 10, with 4 of 6 sprints closed. AC7 in particular is the criterion
the design calls the only *silent and unrecoverable* failure in the Epic; it is now covered by explicit tests on
**both** branches. Remaining: **AC9/AC10** (the Apple worldbuilding cards, SP-099) and the Epic verification
pass (SP-100).
