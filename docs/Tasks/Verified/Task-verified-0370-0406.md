# T-0370 – T-0406 — `[ScriviCore]` EP-031 Object Model, Graph & Worlds (SP-095 – SP-098)

**Status:** ✅ **All twenty Verified 2026-08-12**
**Epic:** EP-031 — `[ScriviCore]` Worldbuilding Object Model & Relationship Graph (🟡 **active** —
SP-102 then SP-100 remain)
**Design:** `docs/Scrivi_Worldbuilding_Object_Model_v0_2.md` (Doc 1) +
`docs/Scrivi_World_Data_Separation_v0_1.md` (Doc 3), both ✅ Approved 2026-08-05

**Records of truth:** [`Sprint-SP-095.md`](../../Sprints/Closed/Sprint-SP-095.md) ·
[`Sprint-SP-096.md`](../../Sprints/Closed/Sprint-SP-096.md) ·
[`Sprint-SP-097.md`](../../Sprints/Closed/Sprint-SP-097.md) ·
[`Sprint-SP-098.md`](../../Sprints/Closed/Sprint-SP-098.md)

---

## SP-095 — Object kinds, fields, object index

**T-0370 — `ObjectKind` additions + `objectKindSubdir` + schema table; retire `timeline`.** (High)
Adds 8 kinds (`building`, `vehicle`, `map` live; `artifact`, `chronicle`, `faction` **gated**;
`world` declared but not creatable). World-scoped kinds were declared but gated (§R1) because world
packages did not exist until SP-097 — create returned `invalidArgument` naming the world requirement
rather than writing to `objects/`. `rule` was left untouched here (relocation was T-0404). Retires
`ObjectKind::timeline` (T4=A) across 5 files, and fixes `objectKindFromStr`'s silent `character`
default (`scrivi_c_api.cpp:350`) — a mis-dispatch that becomes reachable at 12 kinds.

**T-0371 — `WorldObjectFields`: `subtitle`, `image`, `worldID`.** (High) All additive, all defaulting
empty; legacy 5-kind files parse unchanged. **`image` carries `assetID`/`thumbnailAssetID` only —
never inline bytes** (Doc 1 §4.1); images live in `assets/` via `scrivi_import_asset`. `worldID`
round-trips but carried no meaning until worlds landed.

**T-0372 — `objects/index.json` + `findByID` over the index.** (High) `scrivi.object-index.v1`;
replaces the O(n) scan-and-parse in `ObjectStore.cpp:20-55`. **Load-bearing, not an optimization** —
SP-096's edges store bare `{id}` endpoints and cannot be rendered without it. Written atomically
alongside every mutation, **after** the object write succeeds (a phantom entry is silently wrong; a
missing entry self-heals).

**T-0401 — Index rebuild + corruption coverage.** (High) Doc 1 §4.2 calls the rebuild path "a
correctness requirement, not a convenience." Five tests: missing index, corrupt index, stale index
(hand-edited slug), idempotent rebuild, unparseable object file skipped-not-fatal. **The project must
open in every case.**

## SP-096 — Relationship graph

**T-0402 — ⚠️ Endpoint-kind resolution via `ObjectIndex`** (High) — *a design defect found at
planning.* Doc 1 §5.2 said endpoints are distinguished "by ID prefix (`scene_…`)". **That rule is
broken:** `SystemUUIDProvider::newObjectID()` returns **`character_…` for every object kind**, so a
`location`'s ID begins `character_`; and `DeterministicUUIDProvider` uses `scene-`/`obj-` — a
different separator *and* stem, so tests would take a different branch from production. Both failure
modes are **silent**. Replaced by `resolveEndpoint()`: `ObjectIndex::find` first (authoritative for
ID→kind), then the EP-027 scene path, else unresolved. **Doc 1 §5.2 was amended in the same task.**
The generators were deliberately *not* "fixed" instead: ID shape is load-bearing (`objectID`
preservation across promotion, §3.1) and SP-095 had already shipped index rows carrying today's IDs.

**T-0373 — `relation-types.json` + `canonicalDirection` + `symmetric`.** (High)
`scrivi.relation-types.v1`, seeded on project creation with §5.1's four types (`appears-in`,
`located-at`, `sibling-of`, `cites`) and re-seeded on first read if absent. Upsert validates that
`symmetric: true` implies `lexical` and matching labels. ⚠️ **Nullable kind constraints:**
`JsonDoc::getString` cannot distinguish absent from explicit `null`, so "any kind" is written as
**absent** and read as empty-means-any. **This task delivered the `cites`/`documented-by` type — the
first ScriviCore third of T-0365** and the first type unconstrained on *both* ends.

**T-0374 — `relationships.jsonl` append-log + tombstones + torn-line recovery.** (High) `rec`/`seq`
records via `appendTextFile`, reusing EP-019's `HistoryStore` pattern. Create = one appended line;
delete = a tombstone; load = a linear scan retaining the full in-memory map. Ships
`scrivi_create_edge` / `scrivi_delete_edge` / `scrivi_list_edges_for` (bare IDs, no kind params).
⚠️ **`seq` is assigned once at the append point** — `HistoryStore.cpp:194` documents a shipped bug
where 13 records were written with `seq 1`.

**T-0375 — Canonical normalization + duplicate rejection.** (High) §5.3. Asymmetric types normalize
to the declared direction; symmetric types sort endpoint IDs lexically. Creating a relationship from
**either** end yields **one** edge; the reverse create is rejected with a distinct error.
⚠️ `faction` was world-scoped and uncreatable at this point, so the same-kind symmetric case used
`sibling-of` (character↔character); the faction-specific test landed in SP-097/T-0385.

**T-0376 — Compaction at 30% / 1,000 tombstones + graph settings.** (High) Compacts on open at 30% of
records **or** 1,000 tombstones, whichever comes first, with **both triggers tested independently**
(§9 AC9). Temp-write → atomic replace; `seq` renumbered from 1. Settings follow the `HistorySettings`
precedent — **`project.json` canonical + a local mirror**, *not* inside history's `state.json`.

## SP-097 — World packages

> ⚠️ **Content was swapped with SP-098 at planning.** SP-097 was to be the integrity sprint, but
> planning verified **two of its five tasks were unbuildable**: T-0379 (`promote_object`) had no
> destination while `artifact` was refused by SP-095's `checkKindStorable` gate, and T-0380
> (pending-vs-dangling) had **no world plumbing to interrogate** — the Epic's highest-risk branch
> could not be reached, let alone tested. Worlds landed first. **Sprint IDs stayed in sequence.**

**T-0403 — ⚠️ `FileSystem::createFileExclusive` (done first).** (High) Doc 3 §6.5 specified lock
acquisition as "atomic create-if-absent (`AtomicWrite`'s exclusive-create path)". **No such path
existed:** `util::atomicWriteTextFile` is temp-write → `fs::rename`, and **rename silently
overwrites** — two writers would both believe they held the lock. Adds a create-if-absent primitive to
`FileSystem` + `LocalFileSystem` + mocks, with a **concurrent-acquire test proving exactly one
winner**. T-0383 depended on it.

**T-0381 — `.scrivworld` package + `world.json` + world index + `scrivi_create_world`.** (High)
`scrivi.world.v1`, its own `index.json` (reusing `scrivi.object-index.v1`), and the world-scoped kind
directories. `scrivi_create_world` creates the package **and** its binding atomically.
Self-contained: moving the directory loses nothing.

**T-0382 — `binding.json` + `worldID`-verified resolution + relink.** (High)
`worlds/<worldID>/binding.json` with a **platform-neutral** reference (Doc 3 §4.4.1 — no Apple
bookmarks in the model). Resolution tries `lastKnownPath` **relative to the project** first, then
absolute; the first candidate whose `world.json` carries the **matching `worldID`** wins.
⚠️ **A different `worldID` is not the world — resolution stops**, which is what prevents a same-named
package being silently substituted. No search, no registry (ruled).

**T-0383 — Lock→write→unlock + heartbeat + stale-lock recovery.** (High) A `.lock` file, **not** an
OS advisory lock (semantics differ across seven targets and behave poorly on network volumes). Held
**per write, never per session**; heartbeat; **60 s stale-lock recovery** so a crashed writer blocks
others for at most a minute; contention **reports and moves on, never hangs**. `AtomicWrite` does not
subsume this — it makes one write atomic, not a cross-process read-modify-write sequence.

**T-0384 — Epoch chain + `scrivi_resolve_timeline_project_times`.** (High) Three layers (Doc 1 §7.0):
timeline → world → project. Timeline offsets are **world-relative, always**, so rebinding changes
exactly one number. Editing a binding never mutates `world.json`.

**T-0385 — Cached world index → named pending entries; world kinds become creatable.** (High)
`binding.cachedIndex` exists for one reason: a pending entry must read "⟨Midgard: Sword of Dawn⟩",
not a bare UUID — **a writer asked whether to clear world references cannot decide blind**. Never
authoritative; overwritten whenever the world is reachable. Then relaxes `checkKindStorable` so
`artifact`/`chronicle`/`faction` are creatable **into world scope**. ⚠️ **Closed two carried-forward
items:** AC1's gated half, and AC3's faction↔faction symmetric duplicate test deferred from SP-096.

**T-0404 — `rule` relocation to world scope.** (Medium; deferred from SP-095) Moves `objects/rules/`
→ `worlds/<worldID>/rules/` and flips `objectKindIsWorldScoped`. **No migration code** (Doc 3 §7 —
nothing had shipped); fixtures regenerated. Also corrects **Package Structure v0.1 §11** and the
`TODO(SP-098)` marker in `ObjectTypes.hpp`, which named the wrong sprint.

## SP-098 — Integrity, orphans, promotion, `source`

**T-0405 — ⚠️ I-0113: `worldID` on `scrivi_create/open/delete_object`.** (High) A **breaking ABI
widen** — 3 signatures widened and `worldID` plumbed through `scrivi.h`, `scrivi_c_api.cpp` and
`ScriviEngine.swift`, making world objects reachable through the ABI at all.
⚠️ **This is the canonical instance of the ABI-duplication defect class** recorded in CLAUDE.md's
standing rule: a facade test could not see the gap.

**T-0380 — ⚠️ Pending-vs-dangling distinction + frozen graph.** (High) **The Epic's highest-risk
task** — the one failure that is *silent and unrecoverable*. `ResolvedEndpoint::pending()` /
`dangling()`, a frozen graph toward unavailable worlds, and `scrivi_list_pending_edges`
(`EndpointResolver.*`, `RelationshipStore.*`). **Delivers EP-031 AC7 — *absence is never deletion*:**
the graph holds edges pending toward an unavailable world and never prunes them.

**T-0377 — Cascade-prune on object **and** scene delete + load-time repair.** (High) `cascadeDelete`
+ `repairDangling` wired into object, scene and chapter delete, and into project open
(`ObjectStore.cpp`, `SceneDeleter.cpp`, `ChapterDeleter.cpp`, `ProjectOpener.cpp`).

**T-0378 — `scrivi_list_objects` / `scrivi_list_orphaned_objects`.** (High)
`ObjectIndex::loadAllVisible` + 2 endpoints.

**T-0379 — `scrivi_promote_object` (item↔artifact).** (High) `objectID`-preserving, with **zero edges
rewritten** — which is precisely why ID shape was left load-bearing in T-0402.

**T-0406 — `source` object kind.** (Medium) Project-scoped at `objects/sources/` — the enum work
SP-095 deliberately left out — round-tripping through the C ABI with its own schema tag and index
participation (`ObjectTypes.hpp`, `ObjectJson.cpp`, `ObjectIndex.cpp`, `ScriviCore.cpp`). A `cites`
edge relates a source to any kind **across both partitions** — project character and world artifact
alike. **This closed EP-031 AC1**, and was the second ScriviCore third of T-0365.

---

## T-0365's remaining third

T-0373 (`cites` type) and T-0406 (`source` kind) are the two ScriviCore thirds of **T-0365**. The
remaining aggregate **`sources` card** is `[Apple]` work, 🟡 **active in SP-102** →
[`Task-active.md`](../Task-active.md). Source-in-manuscript (footnotes, pull quotes) is deferred to
**EP-032**.

---

*Archived 2026-08-18 during the tracking-documentation audit. All twenty rows had remained in
`Task-backlog.md` after SP-095–SP-098 closed; the Sprint archives were and remain the authoritative
records.*
