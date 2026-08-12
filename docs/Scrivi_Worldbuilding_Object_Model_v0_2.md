# Scrivi — Worldbuilding Object Model Design (v0.2)

**Epic:** TBD (EP-B per §10; the ScriviCore data foundation for the Scene Inspector)
**Sprint:** TBD (design)
**Status:** ✅ **APPROVED 2026-08-05 (Human).** This is **Doc 1 of 3**. All six trades (T1–T6) and all §11
questions are ruled. Implementation follows this doc; per CLAUDE.md any deviation must be surfaced and reconciled
before it is built.
**⚠️ AMENDED 2026-08-12 (user ruling) — `source` relates to OBJECTS, not scenes.** The 2026-08-05 text said
`source` was "related to scenes by ordinary edges"; **that clause is withdrawn** in §3 and §11 Q2. A citation
documents an object, so `source` is a full object relatable to any other via **`cites`/`documented-by`** (the
first relation type with `null` on both kind constraints, §5.1). Putting a source *into* a scene as a footnote
or pull quote requires rendering an object inside manuscript text — a capability Scrivi does not have — and is
deferred to **EP-032**. New **§3.4**; `source` promoted into the §3 kinds table.

| Doc | File | Status |
| --- | --- | --- |
| **1** | `Scrivi_Worldbuilding_Object_Model_v0_2.md` *(this doc)* | ✅ **Approved 2026-08-05** |
| **2** | `Scrivi_Scene_Inspector_Card_Framework_v0_1.md` | ✅ **Approved 2026-08-05** |
| **3** | `Scrivi_World_Data_Separation_v0_1.md` | 🔵 Draft — W1–W6 ruled, body drafted |
**Date:** 2026-08-04
**Author:** Claude (planning), for user review.
**Supersedes:** `Scrivi_Worldbuilding_Object_Model_v0_1.md` (2026-08-03). All six trades are now **ruled**; three
rulings materially revised the v0.1 design (T3, the edge model, and the world/timeline boundary).
**Source of truth it extends:** `Scrivi_Project_Package_Structure_v0_1.md` §11 (`objects/`),
`Scrivi_Minimum_Schema_Set_v0_1.md`, `Scrivi_Architecture_v0_3.md` (JSON-over-C-ABI boundary),
`Scrivi_Timeline_Panel_Design_v0_3.md` §4.9–4.10 (historical events, imported timelines). Derived from the
**Cumberland** abstract model (`Card`, `CardEdge`, `RelationType`, `Kinds`), reoriented for Scrivi.

---

## 0. What changed from v0.1 (user rulings, 2026-08-03/04)

| # | Ruling | Effect on the design |
| --- | --- | --- |
| **T1** | **A** — project-level edge storage | Kept, but the **file format changed** to an append-only log (§5.4) to remove write amplification. |
| **T2** | **A** — cascade-prune on delete | Kept. **Orphaned objects are deliberately retained** (§5.5) + new find/orphan endpoints. |
| **T3** | **B (refined)** — *not* v0.1's "A or C" | `item` and `artifact` both exist as **distinct kinds** split by ownership, with **promotion** (§3.1). |
| **T4** | **A** — retire legacy `timeline` kind | Kept (§3.2). |
| **T5** | **A** + world **partition** + import stubs | Expanded well beyond a label; the partition boundary lands here, the migration goes to **Doc 3** (§7). |
| **W2** | **Three layers** — each historical timeline, each world, and the project each own an epoch; offsets chain | Ruled early (it constrains the world *and* timeline schemas): timeline → world → project, with timeline offsets **world-relative** (§7.0). Pre-empts a Doc 3 trade. |
| **T6** | **A** — uniform endpoints | Kept, but endpoints are now **bare `{id}`** — `kind` dropped (§5.2). |

Two structural findings drove the revision, both confirmed against shipped code:

1. **`ObjectStore::findByID` is an O(n) directory scan with a JSON parse per file**
   (`ScriviCore/src/objects/ObjectStore.cpp:20-55`). It already runs on every open/save/delete. Under a
   relationship graph it would run per edge endpoint per scene selection. → **object index** (§4.2).
2. **Cumberland's relationship trouble was not two files — it was two independently-editable records with no
   canonical identity.** → **single canonical edge + read-time inverse projection** (§5.2), which avoids
   Cumberland's write-the-reverse-every-time cost entirely.

---

## 1. Purpose & scope

Scrivi's Scene Inspector will surface, per scene, a writer-configurable stack of **worldbuilding-object cards**
(characters, locations, artifacts, …) and **writing-tool cards** (tags, todo, outline, undo history, …). Those
cards need a model that answers *"which characters / locations / artifacts / maps / chronicles belong to **this
scene**"* — i.e. a **typed relationship graph**.

**This doc designs only the ScriviCore data foundation** (model port + C ABI). Card UI and per-scene card
configuration live in **Doc 2**. Migration of world-scoped data out of project scope lives in **Doc 3**.

**In scope:** object kind set; the object index; a directed, typed relationship graph; on-disk schemas; the C ABI;
reconciliation with `ObjectStore` and EP-027; and the **world partition boundary**.

**Out of scope (other subsystems own these):** `scenes` & `chapters` (EP-027, filesystem-authoritative),
the **project timeline** (EP-016/EP-025), `calendars`, `structure`, `projects`. The graph may *reference* a scene
by `sceneID`, but a scene is never an `objects/` file.

---

## 2. Baseline vs. Cumberland

| Aspect | Cumberland | Scrivi today | Gap this doc closes |
| --- | --- | --- | --- |
| Entity model | one unified `Card` (16 kinds) | `WorldObject` variant (5 kinds) + scenes/chapters as filesystem entities | keep Scrivi's split; do **not** unify scenes into objects |
| Kinds | 16 | 5 (character, location, item, rule, timeline) | add building, vehicle, artifact, map, chronicle, world; retire `timeline` |
| **Relationships** | `CardEdge` + `RelationType` | **none** | **new: the relationship graph (§5) — the core deliverable** |
| ID → file lookup | SwiftData object graph | **O(n) scan + parse per lookup** | **new: object index (§4.2)** |
| Per-object fields | name, subtitle, detail, image, tags | displayName, notes, tags, attributes, assets | add image + subtitle + worldID (§4.1) |
| Storage | SwiftData/CloudKit | JSON in `.scrivi` | schemas + C ABI (§4, §6) |

---

## 3. Object kinds

**Worldbuilding object kinds** (become `objects/` files; can be inspector cards):

| Kind | Scope | Notes |
| --- | --- | --- |
| `character` | project | (exists) |
| `location` | project | (exists) |
| `building` | project | new |
| `vehicle` | project | new |
| `item` | **project** | (exists) — *a noun in this story*; no backstory required (T3) |
| `artifact` | **world** | new — *has a backstory*; may relate to a `chronicle` (T3) |
| `rule` | **world** | (exists, but **relocates**) — magic systems, sci-fi physics. **Ruled 2026-08-04:** rules govern an *environment*, not a manuscript, so they live at `worlds/<worldID>/rules/` — **not** `objects/rules/`. There is no project-scoped `rule` and no promotion path; a writer defines a rule *in the context of a world*. This **supersedes** the legacy `objects/rules/` layout in Package Structure v0.1 §11 (migration: Doc 3). |
| `map` | project | new — **image-bearing**; a writing AID that may depict a place that **does not exist**; need not bind to a `location` |
| `chronicle` | **world** | new — **narrative prose**; deep history-behind-the-history, may never appear in the manuscript |
| `faction` | **world** | new (ruled 2026-08-05) — organizations, houses, guilds, orders. See §3.3 |
| `source` | project | new (**promoted to this table 2026-08-12**) — citations/references. A **full object**, therefore relatable to any other object via `cites`/`documented-by`. Surfaced as a writing-tool card, but it is not a writing-tool-only artifact. See §3.4 |
| `world` | container | new — see §7 |

**Writing-tool "object":** `source` (citations/references) — **ruled 2026-08-05: a real `objects/` file**
(`objects/sources/<slug>.json`), **project**-scoped, because citations are reusable. It is *surfaced* as a
writing-tool card (Doc 2 §3.1) but is a first-class object. See **§3.4** for what it relates to.

> ⚠️ **CORRECTED 2026-08-12 (user ruling).** The 2026-08-05 text ended *"…related to **scenes** by ordinary
> edges,"* and §11 Q2 said the same. **That was wrong and is withdrawn.** A `source` is a citation — the 3×5
> index card of a research paper — and it attaches to **objects**, not scenes. Source→scene is a genuinely
> different and much larger feature (footnotes / pull quotes, which require rendering an object *inside*
> manuscript text) and is **deferred to a future version** — see §3.4.1 and **EP-032**.

**Excluded** (other subsystems own them): `scene`, `chapter`, `calendar`, `structure`, `project`, and the
**project timeline**.

### 3.1 `item` vs `artifact`, and promotion (T3 = B refined)

v0.1 proposed renaming `item` → `artifact`. **Rejected.** The two kinds mean genuinely different things, and the
distinction is **ownership**, not vocabulary:

- **`item`** — project-owned. A brass key that opens a door in chapter 3. Has no `worldID`.
- **`artifact`** — world-owned. Has a backstory, is prominent, may relate to a `chronicle`. Carries a `worldID`.

**Promotion** (`item` → `artifact`) is a first-class operation: an item that accumulates enough backstory becomes
a world artifact. Promotion **moves the file** (`objects/items/…` → the world's `artifacts/`), assigns a
`worldID`, and **preserves the `objectID` unchanged**.

> **Why `objectID` preservation is load-bearing:** every existing edge pointing at the item must keep resolving
> across promotion. This is precisely why edge endpoints drop `kind` (§5.2) — an endpoint that stored
> `{kind:"item", id:…}` would be **stale the moment the object is promoted**, on every edge that touches it.
> Bare `{id}` + index-resolved kind makes promotion a metadata change rather than a graph rewrite.

Demotion (`artifact` → `item`) is the exact inverse and is supported by the same endpoint.

### 3.2 Retiring the legacy `timeline` object kind (T4 = A)

`ObjectKind::timeline` (`ScriviCore/include/scrivi/ObjectTypes.hpp:23`, subdir `"timelines"`) is a legacy overlap
— the Timeline Panel is the real owner. It is **retired** from `ObjectKind`.

This is *not* the same as the world/timeline split in §7.1, which concerns historical events and imported
timelines (already stored under `objects/historical-events/` and `objects/imported-timelines/` per Timeline v0.3
FR-051/FR-070). Those move to world scope in **Doc 3**; the legacy `timeline` kind simply goes away.

### 3.3 `faction` — and why it matters to §5.3 (ruled 2026-08-05)

Organizations, houses, guilds, orders, noble families. **World-scoped**, alongside `artifact` / `rule` /
`chronicle`: a faction belongs to the world it operates in, and survives any one project that features it.

`faction` needs **no new machinery** — new kinds require no new endpoints (§6), and it uses the same
`WorldObjectFields` block as every other kind. What it *does* do is **exercise the symmetric-relation rule
(§5.3) for the first time**, which is why it was worth ruling now rather than after edges exist on disk:

| Relation | Shape | Canonical direction |
| --- | --- | --- |
| character **belongs to** faction | asymmetric, cross-kind | `source-to-target` |
| faction **controls** location | asymmetric, cross-kind | `source-to-target` |
| faction **at war with** faction | **symmetric, same-kind** | **`lexical`** (§5.3) |

That last row is exactly the case §5.3 exists for: both endpoints are factions, both labels read "at war with,"
and neither end is naturally the `from` side. Without `symmetric: true` + lexical endpoint sort, "House Vance at
war with House Ordo" and "House Ordo at war with House Vance" would normalize to **two different canonical
edges** — reintroducing the Cumberland duplication this model was designed to prevent.

> **This is the concrete justification for the §5.3 rule.** It was written as a hedge against a hypothetical
> symmetric type; `faction` makes it certain. §9 AC4 must therefore include a same-kind symmetric case, not only
> the asymmetric character→scene case.

### 3.4 `source` — citations attach to OBJECTS, not scenes (ruled 2026-08-12)

A `source` is a **citation**: the 3×5 index card of a research paper. It records where something came from.
The thing it documents is an **object** — a character, a location, a rule, a faction — not a passage of
manuscript text.

**`source` is a full object** (`objects/sources/<slug>.json`, project-scoped, kind `source`), which by
definition makes it relatable to **any other object** through the ordinary graph. Its relation type is
**`cites` / `documented-by`**, valid against **any kind** (`sourceKind: null`, `targetKind: null`):

```jsonc
{ "code": "cites",
  "forwardLabel": "cites", "inverseLabel": "documented by",
  "sourceKind": null, "targetKind": null,          // any object ↔ any object
  "canonicalDirection": "source-to-target", "symmetric": false }
```

**No new relationship machinery is required, and the cardinality is already right.** §5.2's model stores one
canonical edge per relationship, so:

- one source related to **many objects** = many edges, one per object;
- one object carrying **many sources** = many edges, one per source.

Many-to-many falls out of the edge model with nothing added. (Earlier drafting characterised this as a
"one-to-many shape" needing special treatment — it does not; an edge is an edge.)

#### 3.4.1 ⏸ Source → scene is DEFERRED — and why it is not a small task

Sources may well end up *in* the manuscript, as **footnotes** or **pull quotes**. That is desirable and is
**not** being designed away — it is being **deferred**, because it is far larger than a relation type:

> Putting a source into a scene means **rendering an object inside manuscript text**. Scrivi has no such
> capability today. Scene bodies are Markdown (`SceneReader`/`SceneWriter`); the structured-fragment model
> (`scrivi.fragment.v1`, EP-029) carries scenes and chapters, not object references; and nothing in the
> editor, the manuscript renderer, or export knows how to resolve an embedded object reference, keep it
> current when the object changes, or survive cut/copy/paste of the surrounding text.

This is **epic-sized and version-crossing** → **EP-032 `[Cross]` — Inline Object References in the
Manuscript** (Epic backlog, 🔵 Proposed). It is deliberately **not** in EP-031. Until it ships:

- **no `source`→`scene` relation type is defined**, and none should be added opportunistically;
- sources reach a scene **indirectly and read-only**, through the objects that scene relates to
  (scene → objects → sources), which is exactly what the Doc 2 `sources` card renders.

> **Why deferring costs nothing structurally.** Scene endpoints are already supported by the graph in general
> (`appears-in` is character→scene, §5.1), and endpoints are bare `{id}` (§5.2). If EP-032 later adds a
> source→scene type, it is **additive** — a row in `relation-types.json`, no schema change, no migration of
> existing edges.

---

## 4. On-disk object schema & the object index

### 4.1 Object schema (extends Package Structure §11)

Each object remains one JSON file, identity key `objectID`, filename a human convenience. Schema tag
`scrivi.object.<kind>.v1`. The shared `WorldObjectFields` block is retained and extended:

```jsonc
{
  "schema": "scrivi.object.character.v1",
  "objectID": "character_01J8X…",
  "slug": "ada",
  "displayName": "Ada",
  "subtitle": "",                    // NEW — one-line descriptor (Cumberland parity)
  "status": "active",                // "active" | "archived"
  "notes": "…freeform detail text…",
  "tags": ["protagonist", "engineer"],
  "attributes": { "eyeColor": "grey" },
  "image": {                          // NEW — optional
    "assetID": "asset_01J…",          // reference into assets/ (NOT inline bytes)
    "thumbnailAssetID": "asset_01J…"  // optional
  },
  "worldID": "world_01J…",            // NEW — optional; present on world-scoped kinds (§7)
  "createdAt": "…", "createdBy": { … },
  "modifiedAt": "…", "modifiedBy": { … }
}
```

- **Images reference `assets/`, never inline bytes** — reuses `scrivi_import_asset`; keeps object files small and
  Git-friendly (contrast Cumberland's `@Attribute(.externalStorage)`, a SwiftData mechanism Scrivi doesn't use).
- Additive: existing character/location/item/rule files load unchanged (missing keys default empty).

### 4.2 The object index — `objects/index.json` (NEW in v0.2)

**Problem.** `ObjectStore::findByID` (`ObjectStore.cpp:20-55`) lists the kind directory and parses every JSON file
until one matches. Acceptable for one-at-a-time CRUD; **not** acceptable when rendering a scene's inspector
resolves every edge endpoint — a 12-object scene would trigger 12 directory scans and hundreds of parses per
scene selection. Dropping `kind` from endpoints (§5.2) makes an ID→kind lookup mandatory besides.

```jsonc
// objects/index.json
{
  "schema": "scrivi.object-index.v1",
  "entries": [
    { "objectID": "character_01J8X…", "kind": "character", "slug": "ada",
      "displayName": "Ada", "worldID": null }
  ]
}
```

Two properties keep this from becoming a second source of truth:

1. **Derived cache, never authoritative.** The `<slug>.json` files remain the truth. If the index is missing,
   stale, or corrupt, ScriviCore **rebuilds it by scanning `objects/`** — the same repair-on-open-before-validation
   pattern EP-027 used to fix C6 for scenes. A hand-edited file or a resolved Git conflict can never break the
   project; it costs one rebuild.
2. **It absorbs existing cost.** `findByID` becomes an index lookup with the scan as fallback. This **removes**
   work already being done rather than adding a new subsystem.

The index is written atomically (existing `AtomicWrite`) alongside any create/save/delete/promote.

> **Consequence of the T6 endpoint ruling:** the index is now **load-bearing**, not merely an optimization —
> edge endpoints cannot be rendered without it. That is the accepted cost of promotion-safety (§3.1). The rebuild
> path is therefore a correctness requirement, not a convenience, and needs explicit test coverage (§9 AC).

---

## 5. The relationship graph (the core deliverable)

A **directed, typed edge** connects two endpoints; a **RelationType** supplies the bidirectional vocabulary and
optional kind constraints. Endpoints are **objects OR scenes**.

### 5.1 RelationType vocabulary — `objects/relation-types.json`

```jsonc
{
  "schema": "scrivi.relation-types.v1",
  "types": [
    { "code": "appears-in",
      "forwardLabel": "appears in", "inverseLabel": "has characters",
      "sourceKind": "character", "targetKind": "scene",
      "canonicalDirection": "source-to-target",   // NEW — see §5.2
      "symmetric": false },                        // NEW — see §5.3
    { "code": "located-at",
      "forwardLabel": "takes place at", "inverseLabel": "hosts",
      "sourceKind": "scene", "targetKind": "location",
      "canonicalDirection": "source-to-target", "symmetric": false },
    { "code": "sibling-of",
      "forwardLabel": "sibling of", "inverseLabel": "sibling of",
      "sourceKind": "character", "targetKind": "character",
      "canonicalDirection": "lexical", "symmetric": true },
    { "code": "cites",                               // §3.4 — citations
      "forwardLabel": "cites", "inverseLabel": "documented by",
      "sourceKind": null, "targetKind": null,        // ANY object ↔ ANY object
      "canonicalDirection": "source-to-target", "symmetric": false }
  ]
}
```

Kind constraints are optional (`null` = any kind). **`cites` is the first type to use `null` on both ends**
(ruled 2026-08-12) — a citation may document a character, a location, a rule, a faction, or anything else, so
constraining either endpoint would be wrong. Its canonical direction is `source-to-target` with the `source`
object on the `from` side, so "this source cites that object" and "that object is documented by this source"
normalize to **one** edge (§5.3).

### 5.2 One canonical edge, inverse as a read-time projection (T6 = A, endpoints bare)

**The Cumberland failure mode, stated precisely.** Cumberland's trouble was *not* that two files existed. It was
that the two directions were **independently editable records with no shared canonical identity**, so
`appears-in` and `has-characters` could drift — one edited, one not — with nothing to say which was right.
Writing the reverse edge on every create is the expensive workaround for a model that never decided which record
was canonical. **Scrivi does neither.**

Store **one edge, directed, canonical**:

```jsonc
{
  "edgeID": "edge_01J…",
  "from": { "id": "character_01J8X…" },   // bare {id} — kind resolved via objects/index.json
  "to":   { "id": "scene_01J8Z…" },
  "relationType": "appears-in",
  "note": "",
  "sortIndex": 0.0
}
```

`relation-types.json` carries **both** labels, so the *same single edge* renders as:

- from Ada's card → "**appears in** Scene 4"
- from Scene 4's card → "**has characters:** Ada"

The two labels you named are not two relationships; they are two renderings of one. **No reverse write, no
doubled work for ScriviCore.**

**Endpoints are bare `{id}`** (ruled 2026-08-04). `kind` is resolved through the object index (§4.2). This
permanently eliminates the promotion-staleness class described in §3.1 — an `item` promoted to `artifact` needs
**zero** edge rewrites. Scene endpoints resolve via the scene identity path (EP-027 sidecar scan), not the object
index.

> ⚠️ **CORRECTED 2026-08-12 (SP-096 T-0402).** This paragraph previously ended: *"the loader distinguishes them
> by **ID prefix** (`scene_…`)."* **That rule does not work and is withdrawn.** Verified against the shipped
> generators:
>
> | Generator | Mints | Consequence |
> | --- | --- | --- |
> | `SystemUUIDProvider::newObjectID()` | **`character_…` for EVERY object kind** | a `location`'s ID literally begins `character_` — the prefix names the *generator*, not the kind |
> | `DeterministicUUIDProvider` (tests) | `obj-1` / `scene-1` | different separator **and** stem, so tests would take a different branch from production |
>
> A prefix test would therefore be wrong in production, differently wrong under test, and **silent** in both
> cases — an edge would resolve against the wrong subsystem and report "not found" rather than erroring.
>
> **Endpoint kind is resolved by LOOKUP, never by string inspection** (`objects/EndpointResolver`):
> object index → EP-027 scene path → unresolved. This is stronger than the original intent: prefixes are an
> unenforced naming convention, while the index is the actual authority for ID→kind — which is exactly why
> promotion cannot stale an endpoint (§3.1). SP-097's pending-vs-dangling distinction hangs off this one
> resolver rather than a prefix test repeated at every call site.
>
> **The generators were deliberately NOT changed.** Making `newObjectID()` kind-aware would alter ID *shape*,
> and IDs must never change: `objectID` preservation across promotion is load-bearing, and SP-095 already
> shipped projects whose `objects/index.json` rows carry today's IDs. The resolver is additive and touches no
> stored data.

**`sortIndex`** (Double, mid-insertion friendly) ports Cumberland's ordered swimlanes, letting the inspector order
"characters in this scene" deliberately.

**Temporal fields deferred.** Cumberland's `CardEdge` carries `temporalPosition`/`duration`. Scrivi's Timeline
owns scene story-time (EP-016) via scene sidecars; edge temporality is out of scope to avoid two sources of truth.

### 5.3 Duplicate prevention — a create-time normalization rule

With one canonical edge, duplication becomes a **create-time uniqueness problem**, not a storage problem:

> An edge is uniquely identified by the unordered pair {endpointA, endpointB} plus the relation type's
> **canonical direction**.

When a writer creates a relationship from *either* end, ScriviCore normalizes to canonical direction **before**
writing. Adding "Scene 4 has characters: Ada" when "Ada appears-in Scene 4" already exists resolves to the same
canonical edge and is **rejected as a duplicate** rather than written as a second record.

**⚠️ The symmetric-type caveat — surfaced explicitly.** Canonical direction works cleanly only for *asymmetric*
types (character→scene). **Symmetric** types — `sibling-of`, `allied-with`, where both labels are identical and
neither kind is naturally the `from` side — have **no natural canonical direction**, and are exactly where
duplicates would reappear. Rule:

- `symmetric: true` types set `canonicalDirection: "lexical"`; ScriviCore canonicalizes by **sorting the two
  endpoint IDs lexically** before write and lookup.

This is a field and a sort now; it is **much harder to retrofit once edges exist on disk**, which is why it lands
in v0.2 rather than waiting for a symmetric type to be requested.

### 5.4 Storage format — append-only log (T1 = A, format revised)

v0.1 specified a single `relationships.json` array. Memory was analysed and is **not** the constraint:

| Edges | Resident memory (bare `{id}` endpoints, incl. multimap overhead) |
| --- | --- |
| 1,000 | ~0.2 MB |
| 10,000 | ~2 MB |
| 100,000 | ~20 MB |

"Thousands of relationships" is well under a megabyte; even a pathological 100k-edge project is ~20 MB — less
than one imported map image. **The full in-memory map is therefore retained** (ruled), because it is what makes
duplicate detection (§5.3) and orphan queries (§5.5) O(1).

The costs that *do* scale badly are I/O-shaped:

- **Write amplification** — a whole-array rewrite on *every* edge add. At 100k edges, adding one relationship
  rewrites megabytes. This is the worst of the two and v0.1 did not address it.
- **Parse time on open** — a whole-document parse of a large array, on the project-open critical path.

**Resolution: `objects/relationships.jsonl`, an append-only record log with periodic compaction** — the pattern
already shipped and test-covered in EP-019 (`ScriviCore/src/history/HistoryStore.cpp`, `rec`/`seq` records via
`FileSystem::appendTextFile`, torn-final-line detection at load).

```jsonl
{"rec":"edge","seq":1,"edgeID":"edge_01J…","from":"character_01J8X…","to":"scene_01J8Z…","relationType":"appears-in","note":"","sortIndex":0.0}
{"rec":"tomb","seq":2,"edgeID":"edge_01J…"}
```

- Adding an edge = **one line appended**, regardless of graph size. Write amplification gone.
- Load = linear scan, no whole-document JSON parse; tombstones applied in `seq` order.
- **Compaction on open** when tombstones exceed **30% of records _or_ 1,000 tombstones, whichever comes first**
  (ruled 2026-08-05; mirrors history's eviction/compaction). Both bounds are needed: the **ratio** alone would
  make a small graph compact constantly, while the **absolute** alone would let a 100k-edge graph accumulate tens
  of thousands of dead records first. Stored with the other history-style settings so it is tunable without a
  schema change.

This changes T1-A's *file format* only — edges still live in one project-level location, still load wholesale,
still serve the inspector identically, and reuse a persistence pattern with existing test coverage rather than
inventing a second one.

### 5.5 Referential integrity and orphans (T2 = A)

**Cascade-prune on delete.** `scrivi_delete_object` and the scene-delete path append tombstones for every edge
referencing the deleted id, in the same atomic operation (mirrors EP-027's "self-consistent on open"). A
load-time repair pass drops any edge whose endpoint no longer resolves — belt-and-suspenders, like
`pruneInconsistentNodes` in history.

> ⚠️ **Critical exception — absence is never deletion (ruled 2026-08-04; Doc 3 §4.6).** The load-time repair pass
> **must not** prune an edge whose endpoint fails to resolve because its **world is offline, missing, or
> unreadable**. Such edges are held **pending**: neither resolved nor pruned, and **no edge into that world may
> be added or removed** until the world returns. Pending edges must also survive save.
>
> The loader therefore distinguishes **two** unresolvable cases:
>
> | Case | Cause | Action |
> | --- | --- | --- |
> | **Dangling** | endpoint gone, its world (or the project) **is present** | prune per this section |
> | **Pending** | endpoint's **world is unavailable** | hold; never prune; never modify |
>
> **Why this exception is load-bearing:** the failure it prevents is silent and unrecoverable. A writer opens her
> project on a machine where the world package isn't mounted; a naive repair pass reads "unresolvable" as
> "deleted" and destroys every relationship into that world with no error shown. Every other integrity hazard in
> this doc is recoverable; this one is not. Implementations must treat "cannot reach the world" as a distinct
> outcome from "endpoint not found," and this needs explicit test coverage (§9 AC16).

**Orphaned objects are deliberately retained** (ruled). An object with no relationships is a legitimate creative
state — a character sketched before they have a scene. Pruning the *edge* while keeping the now-unrelated
*object* is correct; the object stays available to be related later.

Because orphans are intentional, they must be **findable**:

- **orphan** = present in `objects/index.json`, absent from every endpoint in the in-memory edge map.

Both queries are pure reads over structures already resident (§4.2, §5.4), so they are O(n) over the index with
no I/O. These endpoints also serve a future "unrelated objects" inspector view or project-health card.

---

## 6. C ABI (JSON-over-C-ABI, additive)

All additive to `scrivi.h`; heap JSON envelopes freed with `scrivi_free`. No existing endpoint changes.

```c
/* Relationship graph (scrivi.relationships.v1) */
const char* scrivi_create_edge(const char* projectRootPath,
                               const char* fromID, const char* toID,
                               const char* relationTypeCode, const char* note);
const char* scrivi_delete_edge(const char* projectRootPath, const char* edgeID);
/* All edges touching an endpoint (either direction) — the inspector's per-scene query. */
const char* scrivi_list_edges_for(const char* projectRootPath, const char* endpointID);

/* RelationType vocabulary (scrivi.relation-types.v1) */
const char* scrivi_list_relation_types(const char* projectRootPath);
const char* scrivi_upsert_relation_type(const char* projectRootPath, const char* relationTypeJson);

/* Object discovery (§5.5) — backed by objects/index.json */
const char* scrivi_list_objects(const char* projectRootPath, const char* kindOrNull);
const char* scrivi_list_orphaned_objects(const char* projectRootPath);

/* Promotion / demotion (§3.1) — preserves objectID; moves the file, sets/clears worldID */
const char* scrivi_promote_object(const char* projectRootPath,
                                  const char* objectID, const char* targetKind,
                                  const char* worldIDOrNull);

/* World data import — STUBBED in v1 (§7.2); returns notImplemented.
   NOTE (Doc 3 §4.4.1): the stored world reference must be PLATFORM-NEUTRAL. Any
   platform access token (e.g. Apple security-scoped bookmarks) belongs to the
   platform layer, never to ScriviCore's reference or this ABI. */
const char* scrivi_import_world(const char* projectRootPath, const char* sourceWorldPath);
const char* scrivi_list_worlds(const char* projectRootPath);

/* Epoch chain (§7.0). Layer 2→3: the world↔project-timeline translation.
   Created automatically when a world is created or added; editable after. */
const char* scrivi_set_world_epoch_offset(const char* projectRootPath,
                                          const char* worldID, int64_t epochOffsetMs);
const char* scrivi_get_world_binding(const char* projectRootPath, const char* worldID);

/* Epoch chain layer 1→2: a historical timeline's offset against its WORLD's epoch. */
const char* scrivi_set_timeline_epoch_offset(const char* projectRootPath,
                                             const char* worldID, const char* timelineID,
                                             int64_t epochOffsetMs);
/* Resolves the full chain (event → timeline → world → project) for one timeline's events. */
const char* scrivi_resolve_timeline_project_times(const char* projectRootPath,
                                                  const char* worldID, const char* timelineID);
```

Note `scrivi_create_edge` and `scrivi_list_edges_for` take **bare IDs** — no kind parameters — per §5.2.

New object kinds need **no** new CRUD endpoints: `scrivi_create/open/save/delete_object` already dispatch on an
`objectKind` string and gain the new kinds in `ObjectKind` + `objectKindSubdir` + the schema table. The
`image`/`subtitle`/`worldID` fields extend `WorldObjectFields` + `ObjectJson` additively.

---

## 7. Worlds — the partition boundary (T5 = A, expanded)

**Ruled:** a project may span **multiple worlds** — a sci-fi series documenting several planets; a fantasy with
multiple realities characters travel between; Heaven, Hell, and Midgard each a world. **World data is kept
separate from project data inside the `.scrivi` package.**

This is materially larger than v0.1's §7, which treated `worldID` as an optional label. What lands **here** is
only the part that constrains the object model now:

```text
.scrivi/
  objects/                  ← PROJECT-scoped objects
    characters/  locations/  buildings/  vehicles/  items/  maps/
    index.json
    relationships.jsonl
    relation-types.json
  worlds/                   ← WORLD-scoped data (NEW)
    <worldID>/
      world.json            ← the world object itself (+ its intrinsic epoch, §7.0)
      binding.json          ← THIS project's epoch translation (§7.0)
      artifacts/
      rules/                ← relocated from objects/rules/ (ruled 2026-08-04, §3)
      chronicles/
      historical-events/    ← Doc 3 W1
      historical-timelines/ ← Doc 3 W1; each with its own epoch (§7.0)
```

> **Doc 3 W6=A supersedes this layout's location.** Worlds are ultimately **separate `.scrivworld` packages**
> referenced by the project, not folders inside `.scrivi`; only `binding.json` stays project-local. The structure
> above is correct as to *contents and ownership* — which is what constrains this doc's model — while Doc 3
> settles where the package lives. Objects are unaffected either way, because endpoints are bare `{id}` (§5.2).

- Objects carry an optional **`worldID`** (§4.1); world-scoped kinds (§3) live under their world's folder.
- The **object index spans both partitions**, so edges resolve across the project/world boundary uniformly — a
  project `character` may relate to a world `artifact` with no special casing.
- **Import hooks are stubbed** (`scrivi_import_world`, `scrivi_list_worlds` — §6), returning `notImplemented` in
  v1. Designing the *boundary* now means the stub does not have to be redesigned when sharing ships.

### 7.0 The epoch chain — three layers (ruled 2026-08-04)

Epoch is **not** a single value at a single layer. Three distinct reference points exist, each owned by whoever
it is intrinsic to, and each translated to the next by a stored offset:

```text
historical timeline epoch  ──(timeline.epochOffsetMs)──▶  world epoch
world epoch                ──(binding.epochOffsetMs)───▶  project timeline
```

1. **Each historical timeline owns its own epoch.** Timelines within one world do **not** necessarily share a
   reference point — one history may be recorded from "The First Sundering," another from "Founding of the
   Republic." Relating events across two histories requires translating both, so each timeline must carry its own
   epoch rather than inheriting the world's.
2. **Each world owns its own epoch** — intrinsic to the world, part of what it *is*, travelling with the world
   when shared or imported.
3. **Each project owns the translation** — when a world is created or added, an **epoch relationship** is created
   defining that world's epoch against the main project timeline.

```jsonc
// worlds/<worldID>/world.json — the world's INTRINSIC epoch
{
  "schema": "scrivi.object.world.v1",
  "objectID": "world_01J…",
  "displayName": "Midgard",
  "epoch": { "label": "The First Sundering" }      // world-owned; travels with the world
}

// worlds/<worldID>/historical-timelines/<slug>.json — the TIMELINE's own epoch
{
  "schema": "scrivi.externalTimeline.v1",
  "epoch": { "label": "Founding of the Republic" }, // timeline-owned
  "epochOffsetMs": 31536000000                      // → translates to the WORLD epoch
}

// worlds/<worldID>/binding.json — THIS PROJECT's translation of the world
{
  "schema": "scrivi.world-binding.v1",
  "worldID": "world_01J…",
  "epochOffsetMs": -94608000000                     // → translates to the PROJECT timeline
}
```

Project-relative position of a historical event is therefore a **chain**:

```text
project_time(event) = event.offsetMs
                    + timeline.epochOffsetMs        // → world time
                    + world.binding.epochOffsetMs   // → project story-time
```

**Why each layer must own its own epoch.** Two projects in a series may bind the *same* world at *different*
offsets — a prequel opening 1,000 years before the sequel, against one unchanged world history. If the offset
lived in the world, those projects would fight over one field; if the label lived in the project, the world would
lose its identity when shared. Likewise, if timelines inherited the world's epoch, two histories recorded from
different reference points could not be related without silently re-basing one of them.

**Why timeline offsets are world-relative, not project-relative** (ruled): rebinding a world to a different
project changes **one** number — `binding.epochOffsetMs` — and every timeline in that world follows correctly.
Project-relative timeline offsets would require re-offsetting every timeline independently and keeping N numbers
mutually consistent, which would make the world epoch decorative and undermine shareability. Relating two
timelines *within* one world is then pure arithmetic against their shared world epoch, with no project involved
— which is exactly the cross-history comparison this ruling exists to support.

**This generalizes a pattern already shipped.** Timeline v0.3 §6.6/§6.7 does this for imported timelines: the
exported file carries `epochLabel` (intrinsic to the source), and the importing project's stored copy adds
`epochOffsetMs` (the translation) — see `ExternalTimelineJson.cpp:13,26`. The ruling lifts that two-layer split
up to worlds and adds the third layer beneath it.

**Consequences for Doc 3:** this **rules W2** before Doc 3 is drafted — the answer is "every layer owns its own,"
which was neither option Doc 3 sketched. It also means `scrivi_import_world` must produce a binding (not merely
copy files), and that migrating an imported timeline into world scope must **re-base its existing
`epochOffsetMs`** from project-relative to world-relative — a data migration, not a file move (Doc 3 §4 W1/W5).

### 7.1 The project/world timeline split (ruled 2026-08-04)

> **Project timeline** — the manuscript's own story-time. Owned by the Timeline Panel (EP-016/EP-025), sourced
> from scene sidecars. **Project-scoped.**
> **Historical timelines** — derived from an external history. That history belongs to the **world**.
> **World-scoped.**

This is a principled cut, not a compromise: the project timeline is **authored** by writing scenes; historical
timelines are **derived** from a world's recorded history. Different provenance, different owner. It also puts
`chronicle` and historical events on the same side of the line — both are world history, one as prose and one as
dated points — which is more coherent than v0.1.

**This reconciles, rather than contradicts, Timeline v0.3.** FR-050 already states historical events "are
worldbuilding objects — not scenes," and they already persist under `objects/historical-events/` (FR-051), with
imported timelines under `objects/imported-timelines/` (FR-070). The split completes a distinction the Timeline
design had already drawn; it does not overturn it.

### 7.2 Deferred to Doc 3 — World Data Separation

The following are **out of scope for Doc 1** and belong to Doc 3, sequenced **before EP-B implementation**:

1. Migrating `objects/historical-events/` and `objects/imported-timelines/` into world scope (touches EP-016/
   EP-025 ownership; requires a migration path for existing projects).
2. Which world a **scene** is set in (reaches into the scene sidecar and Doc 2's inspector).
3. Package Structure v0.1 §11 revision for the `worlds/` partition.
4. True cross-project world sharing (v0.1's T5-B) — the external world store, and what `scrivi_import_world`
   actually does.

---

## 8. Reconciliation with existing subsystems

- **Scenes/chapters (EP-027):** never become objects. The graph references a scene by `sceneID`; scene-delete
  cascades edge-prune (§5.5). Chapters are addressable the same way if a card needs chapter-level links.
- **Existing `ObjectStore`:** extended, not replaced. `character`/`location`/`rule` files are already compatible;
  `item` stays (T3, §3.1); `timeline` retired (T4, §3.2); `findByID` re-implemented over the index (§4.2).
- **Timeline (EP-016/EP-025):** owns the **project** timeline and scene story-time; edges carry **no** temporal
  fields (§5.2). Historical/imported timelines move to world scope in Doc 3 (§7.1).
- **Assets:** object images reference `assets/` via `assetID` (§4.1), reusing `scrivi_import_asset`.
- **History (EP-019):** object/edge edits are **not** manuscript-text edits; whether they get their own undo is a
  Doc 2 / later question. This doc does not wire object edits into the text `HistoryService`. The edge log
  **reuses** history's append-log persistence pattern (§5.4) without sharing its state.

---

## 9. Acceptance criteria (draft, for the EP-B sprint)

1. New kinds (`building`, `vehicle`, `artifact`, `map`, `chronicle`, `world`) round-trip through
   create/open/save/delete; legacy 5-kind files load unchanged.
2. `objects/index.json` is built on open, updated atomically on every mutation, and **rebuilt from a scan** when
   missing/stale/corrupt (explicit test: delete the index, corrupt the index, hand-edit a slug).
3. `findByID` resolves via the index; the directory scan remains only as the rebuild path.
4. A canonical edge created from **either** endpoint produces exactly **one** record; the reverse create is
   rejected as a duplicate. **Both cases must be tested:** an *asymmetric cross-kind* type (character→scene,
   `source-to-target`) **and** a *symmetric same-kind* type (**faction ↔ faction "at war with"**, `lexical`) —
   §3.3, §5.3. The symmetric case is the one that regresses silently.
5. An edge renders with `forwardLabel` from one end and `inverseLabel` from the other, from a single record.
6. Deleting an object/scene tombstones all its edges atomically; a load-time repair drops unresolvable endpoints.
7. Orphaned objects **survive** deletion of their last edge and are returned by `scrivi_list_orphaned_objects`.
8. `scrivi_promote_object` moves an `item` to a world `artifact`, **preserves `objectID`**, and **zero** edges
   require rewriting (the promotion-safety proof for the bare-endpoint ruling).
9. `relationships.jsonl` compacts on open at **30% tombstones or 1,000 tombstones, whichever first** (§5.4);
   a torn final line is detected and truncated (parity with history-log behavior). Test **both** triggers
   independently — a small graph crossing 30%, and a large graph crossing 1,000 while below 30%.
10. Cross-partition edges (project `character` ↔ world `artifact`) resolve identically to same-partition edges.
11. Creating or adding a world **automatically creates its epoch relationship** (`binding.json`); the world's own
    `epoch.label` is never written by the binding, and editing the offset never mutates `world.json` (§7.0).
12. The same world bound by two projects at **different** `binding.epochOffsetMs` values yields correct,
    independent project-relative times from one unchanged world history (the shareability proof).
13. **The epoch chain resolves correctly:** an event in a world timeline reports
    `event.offsetMs + timeline.epochOffsetMs + binding.epochOffsetMs` as its project time.
14. **Two historical timelines in one world with different epochs** can be related to each other **without**
    reference to any project (pure world-relative arithmetic) — the cross-history comparison this ruling exists
    to support.
15. **Rebinding a world changes exactly one number.** Editing `binding.epochOffsetMs` shifts every timeline in
    that world correctly, and **no** timeline's own `epochOffsetMs` is rewritten.
16. **Absence is never deletion (§5.5).** Opening a project whose referenced world is unavailable holds every
    edge into that world **pending**: none are pruned, the project saves without losing them, and reattaching the
    world restores all of them with no repair pass. Test both branches explicitly — *world present + endpoint
    missing* prunes; *world absent* does not.
17. **The graph is frozen toward an offline world.** Attempts to add or remove an edge into an unavailable world
    are refused (not silently dropped) until the world returns.

---

## 10. Epic/sprint sequencing

1. **Doc 1 (this doc) approved** — all six trades ruled. Design sprint.
2. **Doc 2** — Scene Inspector card UI framework. Design sprint.
3. **Doc 3** — World Data Separation (§7.2). Design sprint; **must precede EP-B implementation**.
4. **EP-A — base card structure:** inspector card framework + **writing-tool cards** (tags, todo, outline, undo
   history [folds in SP-057's history panel], sources, lookahead). Minimal new object-model work.
5. **Complete & close EP-019** (remaining AC2/AC7/AC8 verify; its history panel now lives as a card).
6. **EP-B — worldbuilding-object cards:** implement this doc's model (kinds + index + graph + C ABI), then the
   per-kind cards on top.

> Rationale: EP-A needs almost none of this model, so it can ship and let EP-019 close *before* the larger object
> port (EP-B). Doc 1 is written now so EP-A's framework is designed against the eventual model rather than boxing
> it in.

---

## 11. Open questions for the reviewer

**All resolved as of 2026-08-05.**

1. ~~**§5.4 compaction threshold**~~ ✅ **RULED: compact when tombstones exceed **30%** of records **or** **1,000**
   tombstones, whichever comes first.** Stored beside the other history-style settings so it is tunable without a
   schema change. The absolute bound matters because 30% alone would let a very large graph accumulate tens of
   thousands of dead records before compacting; the ratio matters because 1,000 alone would compact a small graph
   constantly. Either trigger fires the same compaction pass (§5.4).
2. ~~**`source` (citations)** — an `objects/` file or app-side only?~~ ✅ **RULED: `source` is a real
   `objects/` file** (`objects/sources/<slug>.json`), project-scoped — not a per-scene app-side artifact.
   Surfaced as a **writing-tool card** (Doc 2 §3.1) but a first-class object.
   ⚠️ **AMENDED 2026-08-12:** the original ruling continued *"…related to **scenes** by ordinary edges"* and
   justified itself with *"citations are reusable across scenes."* **Both clauses are withdrawn.** Citations
   attach to **objects** (`cites`/`documented-by`, any kind ↔ any kind); source→scene means rendering an
   object inside manuscript text and is **deferred to EP-032**. See **§3.4**.
3. ~~Any kinds beyond the ten in §3?~~ ✅ **RULED: `faction` is approved** — see §3.3.
4. ~~**Doc 3 trigger**~~ — moot; Doc 3 exists (`Scrivi_World_Data_Separation_v0_1.md`).
