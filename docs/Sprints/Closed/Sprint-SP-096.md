## SP-096: [ScriviCore] Relationship Graph — Canonical Edges, Append-Log, Compaction

**Status:** ✅ **Closed (Human-approved 2026-08-12).** All 5 tasks ✅ Verified; both suites green on macOS
and Linux; retrospective reviewed by the user.
**Epic:** EP-031 `[ScriviCore]` Worldbuilding Object Model & Relationship Graph (second of 6 sprints)
**Goal:** Build the relationship graph itself — the Epic's core deliverable. A typed vocabulary in
`relation-types.json`, **one canonical edge** per relationship in an append-only `relationships.jsonl`,
create-time normalization that rejects duplicates from either endpoint, and compaction. No integrity
behaviours (SP-097), no worlds (SP-098), no UI (SP-099).
**Design:** `docs/Scrivi_Worldbuilding_Object_Model_v0_2.md` §5.1–§5.4, §6 (✅ Approved 2026-08-05,
amended 2026-08-12 §3.4/§5.1).
**Start Date:** 2026-08-12 | **End Date:** 2026-08-12 | **Capacity:** ~10–12 hours

---

### Sprint Goal

After SP-096 a writer's project can hold typed relationships: *Ada appears in Scene 4*, *House Vance is at war
with House Ordo*, *this source cites that character*. Creating the same relationship from either end produces
**exactly one** record. The graph loads by linear scan, appends one line per edge regardless of size, and
compacts when tombstones accumulate.

This sprint is where **SP-095's object index earns its keep** — edge endpoints are bare `{id}` (§5.2), so every
endpoint resolution goes through `ObjectIndex::find`.

---

### Assigned Tasks

| ID     | Title | Priority | Status |
| ------ | ----- | -------- | ------ |
| T-0373 | `relation-types.json` — schema, seeded vocabulary, `canonicalDirection` + `symmetric`, upsert | High | ✅ **Verified (2026-08-12)** |
| T-0374 | `relationships.jsonl` append-log — create/delete/list-for-endpoint, tombstones, torn-line recovery | High | ✅ **Verified (2026-08-12)** |
| T-0375 | Canonical normalization + duplicate rejection (asymmetric **and** symmetric) | High | ✅ **Verified (2026-08-12)** |
| T-0376 | Compaction at 30% / 1,000 tombstones + graph settings | High | ✅ **Verified (2026-08-12)** |
| T-0402 | ⚠️ **Endpoint-kind resolution** — replace the unreliable ID-prefix rule (see R1) | High | ✅ **Verified (2026-08-12)** |

### Assigned Issues

None open at planning. **T-0402 is a design defect found at planning**, not a shipped bug — the prefix rule has
never run, so there is nothing to regress. Logged as a task rather than an Issue.

---

### Scope rulings taken at planning (2026-08-12)

#### R1 — ⚠️ §5.2's "distinguish endpoints by ID prefix" **does not work**; T-0402 replaces it

Doc 1 §5.2 says: *"Scene endpoints resolve via the scene identity path (EP-027 sidecar scan), not the object
index; the loader distinguishes them by ID prefix (`scene_…`)."* **Verified against the shipped ID generators
at planning, this rule is unreliable in three separate ways:**

| Generator | Actually produces | Problem |
| --- | --- | --- |
| `SystemUUIDProvider::newObjectID()` (`SystemUUIDProvider.cpp:74`) | **`character_01J…` for EVERY object kind** | A `location` object's ID literally begins `character_`. The prefix names the *generator*, not the kind. |
| `SystemUUIDProvider::newSceneID()` (`:70`) | `scene_01J…` | Correct — but see below. |
| `DeterministicUUIDProvider` (mock, `:16`/`:20`) | `scene-1`, `obj-1` — **hyphen**, and `obj` not `character` | Every test would take a different branch from production. |

A prefix test would therefore be **wrong in production** (all objects look like characters) and **differently
wrong in tests** (separator and stem both differ). Worse, it fails *silently*: an edge would resolve against
the wrong subsystem and report "not found" rather than erroring.

**Ruling: endpoint kind is resolved by LOOKUP, not by string inspection.** `T-0402` adds a single
`resolveEndpoint(projectRoot, id)` used by every edge path:

1. ask `ObjectIndex::find` — a hit yields the object and its true kind (SP-095 already made the index
   authoritative for ID→kind, precisely so promotion cannot stale an endpoint, §3.1);
2. on a miss, ask the EP-027 scene-identity scan;
3. neither → **unresolved**, reported distinctly from "no such relation type" or "endpoint not permitted".

**This is strictly better than the design's intent, not a workaround.** Prefixes are a naming convention that
nothing enforces; the index is the actual authority. SP-097's pending-vs-dangling distinction (T-0380, the
Epic's highest-risk task) then hangs off *one* resolver rather than a prefix test scattered across call sites.

> **Doc 1 §5.2 must be amended** to strike the ID-prefix sentence and point at the resolver. Doing that is part
> of T-0402 — the design and the code must not disagree once this ships.
>
> **Do NOT "fix" the generators instead.** Making `newObjectID()` kind-aware would change ID *shape*, and IDs
> are the one thing in this system that must never change: `objectID` preservation across promotion is
> load-bearing (§3.1), and SP-095 shipped projects whose index rows carry today's IDs. The resolver is
> additive and touches no stored data.

#### R2 — Seed all four relation types, scene endpoints included (ruled)

`relation-types.json` ships with §5.1's four types verbatim: `appears-in` (character→scene), `located-at`
(scene→location), `sibling-of` (character↔character, symmetric/lexical), and `cites` (any↔any, the 2026-08-12
amendment). Scene endpoints are part of the approved model, so SP-096 can create and resolve a
character→scene edge and prove R1's resolver end-to-end. SP-099's cards consume exactly this vocabulary.

#### R3 — `scrivi_list_edges_for` lands in SP-096, not SP-097 (ruled)

SP-096 ships the complete read path (create / delete / list-for-endpoint), so edges are observable through the
ABI within the sprint and its tests are direct rather than inferential. SP-097 then adds only integrity
behaviour — cascade-prune, orphans, promotion, pending-vs-dangling — on a graph that already works. This
matches T-0374's existing wording in the Epic ("create/delete/list").

---

### Task detail

#### T-0373 — `relation-types.json`

**New:** `src/objects/RelationTypes.{hpp,cpp}`, `src/schemas/RelationTypeJson.{hpp,cpp}`.

Schema `scrivi.relation-types.v1` per §5.1. Each type: `code`, `forwardLabel`, `inverseLabel`,
`sourceKind`/`targetKind` (**nullable** — `null` = any kind), `canonicalDirection`
(`source-to-target` | `lexical`), `symmetric` (bool).

- **Seeded on project creation** with the four types from §5.1 (R2). `ProjectCreator` writes the file.
- **An existing project without the file gets the seed on first read** — never an error. Same
  repair-before-validation posture as SP-095's index.
- `scrivi_list_relation_types` / `scrivi_upsert_relation_type`; upsert replaces by `code`.
- **Validation on upsert:** `symmetric: true` **requires** `canonicalDirection: "lexical"`, and a symmetric
  type with differing forward/inverse labels is rejected — §5.3's rule is only sound if both hold.

> **Nullable kind constraints need care in the JSON wrapper.** `JsonDoc::getString` returns `""` for both an
> absent key and an explicit `null`, so "any kind" and "unset" are indistinguishable through the current API.
> Represent an unconstrained end as **absent** on write and treat empty-as-any on read, and state that in the
> schema comment. `cites` (§3.4) is the first type to exercise this on **both** ends.

**Done when:** the four seeded types round-trip; upsert replaces by code; a symmetric type declaring
`source-to-target` is rejected; `cites` round-trips with both kind constraints unconstrained.

#### T-0374 — `relationships.jsonl` append-log

**New:** `src/objects/RelationshipStore.{hpp,cpp}`.

Reuses the EP-019 pattern (`HistoryStore.cpp`) — `rec`/`seq` records via `FileSystem::appendTextFile`,
torn-final-line detection at load — rather than inventing a second persistence mechanism.

```jsonl
{"rec":"edge","seq":1,"edgeID":"edge_01J…","from":"…","to":"…","relationType":"appears-in","note":"","sortIndex":0.0}
{"rec":"tomb","seq":2,"edgeID":"edge_01J…"}
```

- **Load** = linear scan; tombstones applied in `seq` order; full in-memory map retained (§5.4 — memory is not
  the constraint; the map is what makes duplicate detection and SP-097's orphan queries O(1)).
- **Create** = one appended line, regardless of graph size.
- **Torn final line** → truncate and continue: everything before it is intact.
- `scrivi_create_edge(projectRoot, fromID, toID, relationTypeCode, note)` — **bare IDs, no kind params**
  (§5.2/§6); `scrivi_delete_edge(projectRoot, edgeID)`; `scrivi_list_edges_for(projectRoot, endpointID)`
  returning **both directions** with each edge's resolved label for that endpoint (forward from one end,
  inverse from the other — §5.2's read-time projection).

> ⚠️ **`seq` must be assigned at the single point of append, and monotonically.** `HistoryStore.cpp:194`
> records a shipped bug where a value was read before assignment, so 13 records were written with `seq 1`;
> replay survived only because order came from file position. **Do not repeat it:** one `++lastSeq_` at the
> writer, never a value captured earlier, and a test asserting strictly increasing `seq` across mixed
> create/delete traffic.

**Done when:** an edge round-trips through create → reopen → list; delete appends a tombstone and the edge
stops being listed; a hand-truncated final line loads the surviving records; `seq` is strictly increasing.

#### T-0375 — canonical normalization + duplicate rejection

The core correctness task (§5.3, §9 AC4).

- **Asymmetric types** (`canonicalDirection: "source-to-target"`): normalize to the type's declared direction
  before write **and** before lookup. Creating "Scene 4 has-characters Ada" when "Ada appears-in Scene 4"
  exists resolves to the same canonical edge → **rejected as duplicate**, not written twice.
- **Symmetric types** (`lexical`): sort the two endpoint IDs lexically before write and lookup, so
  "Vance at-war-with Ordo" and "Ordo at-war-with Vance" are one edge.
- Rejection is an **error with a distinct code**, not a silent no-op — the caller must be able to tell
  "already related" from "created".

> **§9 AC4 requires BOTH cases tested**, and is explicit that *"the symmetric case is the one that regresses
> silently."* Test asymmetric **cross-kind** (character→scene) and symmetric **same-kind**
> (faction↔faction "at war with"), each created from both ends.
>
> ⚠️ **`faction` is a world-scoped kind and is NOT creatable until SP-098** (SP-095 R1). The symmetric test
> therefore uses a **character↔character** symmetric type (`sibling-of`, already seeded) rather than the
> faction example from §3.3. The *shape* under test — same-kind, symmetric, lexical — is identical, which is
> what AC4 is about; a faction-specific test is added in SP-098 when factions can exist. **State this in the
> test file** so it is not mistaken for an omission.

**Done when:** both duplicate cases are rejected from either creation order; a self-edge (`from == to`) is
rejected; an unknown `relationTypeCode` is rejected; kind constraints are enforced when non-null and ignored
when null (`cites` relates anything).

#### T-0376 — compaction + graph settings

Compact **on open** when tombstones exceed **30% of records** *or* **1,000 tombstones**, whichever comes first
(§5.4, ruled 2026-08-05).

- Compaction rewrites the log with live edges only, atomically (write temp → replace), and **resets `seq`**
  from 1 with the surviving records renumbered in order.
- **Both triggers tested independently** (§9 AC9): a small graph crossing 30% while well under 1,000, and a
  large graph crossing 1,000 while still under 30%.
- **Settings home:** `project.json` is canonical with a mirror beside the log — the exact pattern
  `HistorySettings` already uses (`HistoryStore.hpp:30-36`: *"project.json is canonical; state.json mirrors
  for self-containment"*). Do **not** put graph settings inside history's `state.json`; they are unrelated
  subsystems that happen to share a persistence shape.

**Done when:** both thresholds fire independently; a compacted log replays to an identical in-memory graph;
compaction is crash-safe (an interrupted compaction leaves the previous log readable).

#### T-0402 — endpoint resolution (see R1)

**New:** `resolveEndpoint()` in `src/objects/` + the Doc 1 §5.2 amendment.

Returns a small result: `{ found, isScene, kind, path }`. Object lookup via `ObjectIndex::find`; scene lookup
via the EP-027 identity path; miss → unresolved.

- Every edge operation resolves **both** endpoints through it.
- **Create refuses an unresolvable endpoint** — an edge into nothing is never written in SP-096. (SP-097's
  pending case is *load-time* and world-driven; it does not relax this.)
- **`list_edges_for` does not refuse** — it reports what it has, since SP-097 will need to surface edges whose
  endpoints are temporarily unreachable.

**Done when:** an object endpoint resolves to its true kind even though its ID begins `character_`; a scene
endpoint resolves; an unknown ID is unresolved; a **`location` object whose ID starts `character_` is
correctly reported as `location`** — the explicit regression guard against the prefix rule ever returning.

---

### Out of scope — explicitly

| Not in SP-096 | Where it lands |
| --- | --- |
| Cascade-prune on delete, load-time repair | SP-097 (T-0377) |
| `list_objects` / `list_orphaned_objects` | SP-097 (T-0378) |
| `promote_object` (item↔artifact) | SP-097 (T-0379) |
| ⚠️ Pending-vs-dangling, frozen graph toward offline worlds | SP-097 (T-0380) |
| Worlds, bindings, epoch chain; the `rule` relocation | SP-098 |
| The `source` object kind + `cites` **card** | `source` kind: SP-097 · card: SP-099 (T-0365 split) |
| Any UI, any card | SP-099 |
| Source→scene edges (footnotes / pull quotes) | **EP-032** — deferred, additive when it lands |

> **`cites` the TYPE ships here; `source` the KIND does not.** T-0373 seeds the `cites` relation type because
> it is part of §5.1's approved vocabulary and costs nothing extra. The `source` **object kind** is T-0365's
> ScriviCore half, scheduled for SP-097. A `cites` edge is therefore creatable in SP-096 between any two
> existing kinds — which is correct, since `cites` is unconstrained on both ends by design.

**`scrivi.h` WILL change this sprint** — 5 new endpoints (`scrivi_create_edge`, `scrivi_delete_edge`,
`scrivi_list_edges_for`, `scrivi_list_relation_types`, `scrivi_upsert_relation_type`), all additive per §6.
This is the first `scrivi.h` change since EP-029. No existing endpoint changes signature.

**No `project.pbxproj` change** — ScriviCore-only; the app links the prebuilt `libScriviCore.a`.

---

### Success criteria

1. The four seeded relation types round-trip; upsert replaces by code; invalid symmetric declarations rejected.
2. An edge round-trips create → reopen → list; delete tombstones it; a torn final line does not lose prior records.
3. `seq` is strictly increasing across mixed create/delete traffic (the `HistoryStore.cpp:194` guard).
4. **One canonical edge** from either creation order, for **both** an asymmetric cross-kind type and a
   symmetric same-kind type; duplicates rejected with a distinct error.
5. One edge renders `forwardLabel` from one endpoint and `inverseLabel` from the other (§5.2 projection).
6. Compaction fires at **30%** and at **1,000** tombstones **independently**, and replays identically.
7. **Endpoint kind comes from the index, never from an ID prefix** — including a `location` whose ID begins
   `character_`.
8. `ctest` green on **macOS** (≥ 432 baseline) **and Linux/GCC** (≥ 439 baseline), plus the new tests.
9. macOS interop green (≥ 59 test cases) — count test cases, **not** xcodebuild's `Executed N` line.
10. Existing projects open unchanged; a project with no `relationships.jsonl` is not an error.

---

### Risks

| Risk | Mitigation |
| --- | --- |
| ⚠️ **The ID-prefix rule (§5.2) is wrong and would fail silently** — all objects are `character_`-prefixed, and the mock uses different separators entirely. | **T-0402**: resolve by index lookup, never by string inspection; explicit regression test on a `location` with a `character_` ID; Doc 1 §5.2 amended in the same task. |
| **`seq` assignment bug repeats** — it already shipped once in `HistoryStore`. | One `++lastSeq_` at the single append point; strictly-increasing assertion across mixed traffic. |
| **Symmetric duplicates regress silently** — §9 AC4 says so explicitly. | Both cases tested from both creation orders. `faction` is unavailable until SP-098, so `sibling-of` (character↔character) carries the same-kind symmetric shape; noted in the test file. |
| **Compaction loses edges or corrupts the log mid-write.** | Write temp → atomic replace; interrupted-compaction test asserts the previous log still replays. |
| **Nullable kind constraints collapse to `""`** in `JsonDoc`, making "any" and "unset" indistinguishable. | Absent-on-write, empty-as-any-on-read, documented in the schema; `cites` exercises both ends. |
| Scope creep into SP-097's integrity work — cascade-prune is tempting once edges exist. | Out-of-scope table is explicit. Create refuses unresolvable endpoints; everything load-time is SP-097's. |

---

### Notes

- **Baseline at planning:** ctest **432/432 macOS**, **439/439 Linux (GCC 14)**, interop **59 passed**.
- **SP-095 dependency is real and load-bearing:** bare `{id}` endpoints have no meaning without
  `ObjectIndex::find`. If index resolution is wrong, every edge is wrong — which is why T-0402 routes all
  endpoint questions through it rather than around it.
- **Doc amendment required this sprint:** Doc 1 §5.2's ID-prefix sentence (T-0402). Per CLAUDE.md a
  design/code disagreement must be reconciled, not left implicit.
- **Deferred, still tracked:** the `rule` relocation + Package Structure §11 correction (SP-098); T-0365's
  split (SP-097 + SP-099); EP-032 (source→scene).

---

## Implementation summary (2026-08-12)

**Files added:** `src/objects/EndpointResolver.{hpp,cpp}`, `src/objects/RelationTypes.{hpp,cpp}`,
`src/objects/RelationshipStore.{hpp,cpp}`, `tests/integration/RelationshipTests.cpp`.
**Files changed:** `include/scrivi/scrivi.h` (**+5 endpoints**), `include/scrivi/ObjectTypes.hpp`
(`objectKindFromName` promoted out of `ObjectIndex.cpp`), `src/objects/ObjectIndex.cpp`,
`src/public_api/scrivi_c_api.cpp`, `src/project_package/ProjectCreator.cpp`, both `CMakeLists.txt`.
**No `project.pbxproj` change** — ScriviCore-only, as planned.

### T-0402 — endpoint resolution (done first; everything else depends on it)

`EndpointResolver::resolve()` → object index → EP-027 scene path → unresolved. No string inspection anywhere.
**Doc 1 §5.2 amended in the same task** with the evidence table, so the design and code no longer disagree.
The regression guard is explicit: a `location` object is created, and the test asserts it resolves as
`ObjectKind::location` **even though its ID begins `character_`** — which is precisely what the withdrawn
prefix rule would have got wrong, silently.

### T-0373 — relation types

`scrivi.relation-types.v1`, seeded with §5.1's four types at project creation and **re-seeded on load** if the
file is missing, malformed, or carries the wrong schema — a project must always be able to relate things.

- **"Any kind" is encoded as an ABSENT key**, never an explicit null: `JsonDoc::getString` returns `""` for
  both, so absence is the only unambiguous round-trip. `cites` exercises it on both ends.
- **`validate()` protects §5.3's soundness**, and does so in *both* directions: a `symmetric` type must be
  `lexical` with identical labels, **and** an asymmetric type may not be `lexical` (lexical sorting discards
  the direction that distinguishes its two labels — the inverse trap, which the plan had not called out).

### T-0374 / T-0375 — the log, canonical edges, duplicate rejection

`relationships.jsonl` on the EP-019 `rec`/`seq` pattern: create appends one line; delete appends a tombstone;
load is a linear scan that truncates at a torn final line.

- **`seq` is assigned once, at the append point**, from the replayed maximum. A test walks the raw log and
  asserts strictly-increasing `seq` across mixed create/delete traffic — the `HistoryStore.cpp:194` guard.
  Replay order still comes from file position, so `seq` stays forensic rather than load-bearing.
- **Duplicate rejection compares the unordered pair + relation type**, so a direction-flipped create is caught
  regardless of symmetry. Rejection carries `error.detail == "duplicateEdge"`, distinct from every other
  refusal.
- **A create from the inverse end is accepted and normalized**, not refused: if the endpoints satisfy the type
  reversed, they are swapped into the declared direction before the duplicate check.
- `listFor` returns both directions with the correct label per direction (§5.2's read-time projection) — one
  stored edge, asserted to render as "appears in" from the character and "has characters" from the scene.

### T-0376 — compaction

Fires on open at **30% of records** *or* **1,000 tombstones**. Both triggers are tested **independently**:
a 6-record graph at 33% (well under the absolute bound), and a hand-built 5,001-record log at 20% with 1,001
tombstones (under the ratio). A third test asserts a graph under **both** thresholds is **not** compacted.

### Verification

| Check | Result |
| --- | --- |
| ScriviCore `ctest` (macOS) | ✅ **455/455**, 0 failures (baseline 432; **+23**) |
| ScriviCore `ctest` (**Linux / GCC 14**) | ✅ **462/462**, 0 failures, **zero warnings** |
| SP-096 tests specifically | ✅ 23 test cases / ~190 assertions |
| macOS interop | ✅ **59 passed / 0 failed** (count test cases, not `Executed N`) |
| New C symbols in `libScriviCore.a` | ✅ all 5 exported (`nm`) |

### Two corrections made during implementation

1. **A test of mine was wrong, not the code.** "delete appends a tombstone" asserted the tombstone was on disk
   *after* calling `load()` — but `load()` compacts, and 1 edge + 1 tombstone is 50%, over the ratio. The
   tombstone was correctly erased by the very behaviour T-0376 ships. Fixed by checking the log **before** the
   load, with a comment explaining why the ordering matters.
2. **A claim I made in the plan was untested.** "Compaction is crash-safe (an interrupted compaction leaves
   the previous log readable)" rested on reading `AtomicWrite.cpp` rather than on a test. Confirmed the
   temp-write → `rename` semantics and added a test that plants a half-written `.tmp` and asserts the real log
   still replays.

---

### Retrospective (drafted 2026-08-12, for user review)

**Completed:** T-0373, T-0374, T-0375, T-0376, T-0402 — all ✅ Verified (user-approved 2026-08-12).
**Returned to backlog:** none.

**What went well.**
- **Checking the design against the shipped code before writing the plan paid off again.** §5.2's ID-prefix
  rule read as settled design; it was broken in three ways and would have failed *silently*. Finding it at
  planning made it a scoped task (T-0402) instead of a mid-sprint surprise — and made the resolver a single
  choke point that SP-097's highest-risk task can now hang off.
- **Doing T-0402 first was the right order.** Every other task consumes endpoint resolution, so building it
  first meant no task had to be revisited.
- **Reusing the EP-019 append-log pattern** meant the torn-line, replay-order, and atomic-write behaviours came
  with a shipped precedent — including its documented `seq` bug, which became a test rather than a repeat.

**What didn't.**
- **I wrote a test that contradicted a feature in the same sprint.** "delete appends a tombstone" asserted the
  tombstone was on disk *after* `load()`, but `load()` compacts and 1-edge/1-tombstone is 50%. The code was
  right; the test encoded an assumption the sprint itself invalidated. Worth remembering that when a sprint
  adds a background behaviour, existing-style assertions around it need re-reading.
- **I made a crash-safety claim in the plan that no test backed.** It happened to be true, but it was
  asserted from reading `AtomicWrite.cpp` rather than demonstrated. Now tested.
- **The plan missed one validation direction.** It specified "symmetric ⇒ lexical" but not "asymmetric ⇒ not
  lexical" — lexical sorting discards the direction that distinguishes an asymmetric type's two labels. Caught
  while implementing `validate()`, not while planning.

**Adjust for SP-097.**
- `EndpointResolver` is the single place that decides what an endpoint *is*. T-0380's pending-vs-dangling
  distinction should extend its unresolved case rather than introduce a parallel path.
- The duplicate-rejection `detail` marker (`"duplicateEdge"`) is the pattern for machine-readable error
  discrimination; SP-097's pending/dangling refusals should use `detail` the same way.
- **Carry to SP-098:** the faction↔faction symmetric duplicate test, which is the last clause blocking AC3.

---

*Last Updated: 2026-08-12 (**All 5 tasks ✅ Verified — user-approved.** SP-096 → 🟠 Review, awaiting close
approval. **EP-031 AC5 is met**; **AC3 is met but for its faction↔faction clause**, which needs SP-098.
Suites: ctest **455/455 macOS** + **462/462 Linux (GCC 14)**, interop **59 passed / 0 failed**. Prior note
follows.)*

*2026-08-12 (**SP-096 implemented — all 5 tasks 🟠 Implemented, Not Verified.** ctest
**455/455 macOS** + **462/462 Linux (GCC 14, no warnings)**; interop **59 passed / 0 failed**; all 5 new C
symbols exported. **T-0402 landed first** and its Doc 1 §5.2 amendment is written — the ID-prefix rule is
formally withdrawn with the generator evidence recorded. Awaiting user verification. Prior note follows.)*

*2026-08-12 (SP-096 planned. Scope fixed by three rulings: **R1** — §5.2's endpoint ID-prefix
rule was verified **broken** against the shipped generators (`newObjectID()` returns `character_…` for every
kind; the test mock uses `obj-`/`scene-`), so **T-0402** was added to resolve endpoint kind by index lookup and
to amend the design; **R2** — seed all four §5.1 relation types including the scene-endpoint ones, so the
resolver is proven end-to-end; **R3** — `scrivi_list_edges_for` ships here, giving SP-096 a complete,
directly-testable read path. Awaiting user confirmation before activation.)*
