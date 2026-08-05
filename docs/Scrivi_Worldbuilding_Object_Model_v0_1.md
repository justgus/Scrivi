# Scrivi — Worldbuilding Object Model Design (v0.1)

**Epic:** TBD (the ScriviCore data foundation for the Scene Inspector; "EP-A/EP-B" per the plan below)
**Sprint:** TBD (design)
**Status:** 🔵 **DRAFT — for user review.** This is **Doc 1 of 2** (the ScriviCore model port). Doc 2 is the
Scene Inspector card UI framework. Nothing here is binding until approved; per CLAUDE.md, implementation follows
the approved doc and any deviation is surfaced + reconciled first.
**Date:** 2026-08-03
**Author:** Claude (planning), for user review.
**Source of truth it extends:** `Scrivi_Project_Package_Structure_v0_1.md` §11 (`objects/`),
`Scrivi_Minimum_Schema_Set_v0_1.md`, `Scrivi_Architecture_v0_3.md` (JSON-over-C-ABI boundary). Derived from the
**Cumberland** abstract model (`~/Xcode-Projects/Cumberland/Cumberland/Model` — `Card`, `CardEdge`,
`RelationType`, `Kinds`), reoriented for Scrivi's writing-surface focus and JSON-in-`.scrivi` persistence.

---

## 1. Purpose & scope

Scrivi's Scene Inspector will surface, per scene, a writer-configurable stack of **worldbuilding-object cards**
(characters, locations, artifacts, …) and **writing-tool cards** (tags, todo, outline, undo history, …). Those
worldbuilding cards need a data model that can answer *"which characters / locations / artifacts / maps /
chronicles belong to **this scene**"* — i.e. a **typed relationship graph** between the scene and other objects.

**This doc designs only the ScriviCore data foundation** (the model port + C ABI). The card UI, per-scene card
configuration, and layout trades live in **Doc 2** (Scene Inspector card framework).

**In scope:** the object kind set; a directed, typed **relationship graph** (the biggest gap); on-disk JSON
schemas; the C ABI; reconciliation with the existing `ObjectStore` and the EP-027 scene/chapter model; and the
**cross-project `worlds`** question.

**Out of scope (Scrivi subsystems already own these — NOT modeled as inspector objects):** `scenes` &
`chapters` (EP-027 filesystem-authoritative), `timelines` (Timeline Panel EP-016/EP-025), `calendars` (calendar
system), `structure` (Story Structure), `projects` (project model). The graph may *reference* a scene by its
`sceneID`, but a scene is never an `objects/` file.

---

## 2. What already exists (baseline) vs. Cumberland (the gap)

Scrivi is a cross-platform rewrite of **Cumberland**. Cumberland's model is card-centric: one SwiftData `Card`
(kind-discriminated, 16 kinds) + `CardEdge` (directed, typed, temporal) + `RelationType` (bidirectional,
kind-constrained). **Only the abstract model ports** — Cumberland persists in SwiftData/CloudKit; Scrivi persists
JSON in the `.scrivi` package via ScriviCore (Architecture v0.3), never SwiftData.

| Aspect | Cumberland | Scrivi today | Gap this doc closes |
| --- | --- | --- | --- |
| Entity model | one unified `Card` (kind on the card) | `WorldObject` variant (5 kinds) + scenes/chapters as filesystem entities | keep Scrivi's split; do **not** unify scenes into objects |
| Kinds | 16 | 5 (character, location, item, rule, timeline) | add buildings, vehicles, artifacts, maps, chronicles, worlds; keep locations/characters/rules |
| **Relationships** | `CardEdge` + `RelationType` | **none** | **new: a relationship graph (§5) — the core deliverable** |
| Per-object fields | name, subtitle, detail, image, tags, temporal | displayName, notes, tags, freeform attributes, assets | add image + subtitle (§4) |
| Storage | SwiftData/CloudKit | JSON in `.scrivi` | schemas + C ABI (§4, §6) |

**Key finding:** Scrivi has solid single-object CRUD (`objects/<kind>/<slug>.json`, `scrivi_create/open/save/
delete_object`, tags + attributes + assets) but **no relationship graph at all**. The graph is what every
"entities in this scene" card depends on, so it is the heart of this doc.

---

## 3. Object kinds (user-ruled 2026-08-03)

Cumberland's 16 kinds map onto Scrivi as follows. **Worldbuilding object kinds** (become `objects/` files and
can be inspector cards):

| Kind | Notes |
| --- | --- |
| `character` | (exists) |
| `location` | (exists) |
| `building` | new |
| `vehicle` | new |
| `artifact` | new (Cumberland's `artifacts`; supersedes/renames the generic `item` — see Trade **T3**) |
| `rule` | (exists) — world rules / magic systems / physics |
| `map` | new — **image-bearing**; a writing AID, may depict a place that **does not exist** (a battle map the writer drew to write the scene); need **not** bind to a `location` |
| `chronicle` | new — **narrative prose**, distinct from a Timeline "historical event" (a dot). A chronicle is a *story*: the Lay of Beleriand — deep history-behind-the-history informing a scene, may never appear in the manuscript |
| `world` | new — **cross-project container** (see §7); a writer may write a *series of projects spanning multiple worlds* |

**Writing-tool "object":**

| Kind | Notes |
| --- | --- |
| `source` | citations / references (Cumberland's `sources` + Citation subsystem). Modeled as a **writing-tool card**, not a world object. Whether it is an `objects/` file or lives with the writing-tool cards is a **Doc 2** question; the model may still store it here for reuse across scenes. |

**Excluded** (Scrivi subsystems own them): `scene`, `chapter`, `timeline`, `calendar`, `structure`, `project`.

> The existing `ObjectStore` currently also lists `timeline` as an ObjectKind. That is a **legacy overlap** —
> the Timeline Panel is the real timeline owner. Trade **T4** covers retiring `timeline` from the object kinds.

---

## 4. On-disk object schema (extends Package Structure §11)

Each object stays one JSON file under `objects/<kind-plural>/<slug>.json`, identity key `objectID` (UUID-v7
style, per the on-disk naming conventions), file name a human convenience. Schema tag `scrivi.object.<kind>.v1`.
The shared field block (already implemented in `WorldObjectFields`) is retained and **extended**:

```jsonc
{
  "schema": "scrivi.object.character.v1",
  "objectID": "character_01J8X…",
  "slug": "ada",
  "displayName": "Ada",
  "subtitle": "",                    // NEW (Cumberland parity) — one-line descriptor
  "status": "active",                // "active" | "archived"
  "notes": "…freeform detail text…",
  "tags": ["protagonist", "engineer"],
  "attributes": { "eyeColor": "grey" },  // freeform key/value (exists)
  "image": {                          // NEW — optional; assets already exist via import_asset
    "assetID": "asset_01J…",          // reference into assets/ (NOT inline bytes)
    "thumbnailAssetID": "asset_01J…"  // optional pre-rendered thumb
  },
  "worldID": "world_01J…",            // NEW — optional; which world this object belongs to (§7)
  "createdAt": "…", "createdBy": { … },
  "modifiedAt": "…", "modifiedBy": { … }
}
```

- **Images reference `assets/`, never inline bytes** — Scrivi already has `scrivi_import_asset`; an object's
  image is an `assetID` pointer, keeping object files small and Git-friendly (contrast Cumberland's
  `@Attribute(.externalStorage)` blob, which is a SwiftData/CloudKit mechanism Scrivi doesn't use).
- **`map` and `chronicle`** use the same block: a `map` typically has an `image` + optional linked `location`
  edges; a `chronicle` is mostly `notes` (long-form prose) + edges to the scenes/characters it concerns.
- Additive: existing character/location/item/rule files load unchanged (missing new keys default empty).

---

## 5. The relationship graph — `CardEdge` / `RelationType` port (the core deliverable)

This is the net-new capability. A **directed, typed edge** connects two endpoints; a **RelationType** gives the
edge a bidirectional vocabulary and optional kind constraints. Endpoints are **objects OR scenes** (a scene is
referenced by `sceneID`; it is not itself an object).

### 5.1 Storage (Trade **T1** — where edges live)

**Recommended (T1 = A): a single project-level edge file `objects/relationships.json`** holding an array of
edges, plus `objects/relation-types.json` holding the RelationType vocabulary.

```jsonc
// objects/relationships.json
{
  "schema": "scrivi.relationships.v1",
  "edges": [
    {
      "edgeID": "edge_01J…",
      "from": { "kind": "scene",     "id": "scene_01J8Z…" },   // endpoint = {kind, id}
      "to":   { "kind": "character", "id": "character_01J8X…" },
      "relationType": "appears-in",     // code into relation-types.json
      "note": "",
      "sortIndex": 0.0
    }
  ]
}
```

```jsonc
// objects/relation-types.json
{
  "schema": "scrivi.relation-types.v1",
  "types": [
    { "code": "appears-in", "forwardLabel": "appears in", "inverseLabel": "features",
      "sourceKind": "character", "targetKind": "scene" },     // kind constraints optional (null = any)
    { "code": "located-at",  "forwardLabel": "takes place at", "inverseLabel": "hosts",
      "sourceKind": "scene", "targetKind": "location" }
  ]
}
```

- **Endpoint = `{kind, id}`** — uniform whether the endpoint is a `character` object or a `scene` (`sceneID`).
  So "characters in this scene" = edges where one endpoint is `{scene, thisSceneID}` and the other is a
  `character`. This is exactly what the inspector cards query.
- **Bidirectional labels** come from Cumberland's `RelationType` (forward/inverse), so an edge reads sensibly
  from either endpoint.
- **`sortIndex`** (Double, mid-insertion friendly) ports Cumberland's ordered swimlanes — lets the inspector
  order "characters in this scene" deliberately.
- **Temporal fields deferred.** Cumberland's `CardEdge` carries `temporalPosition`/`duration` for timeline
  placement. Scrivi's Timeline already owns scene story-time (EP-016) via scene sidecars, so edge temporality is
  **out of scope here** to avoid two sources of truth. (Revisit only if a non-scene object ever needs timeline
  placement.)

*Alternatives considered:* **T1 = B** per-object edge lists (edges stored inside each object file) — natural for
"this object's relationships" but a scene endpoint has no object file to live in, and it duplicates each edge on
both ends (sync hazard). **T1 = C** one file per edge (`objects/relationships/<edgeID>.json`) — Git-granular but
heavy for a graph that is read wholesale per scene. A single project-level file (A) is read once per open, edited
atomically, and matches how the inspector consumes it (all edges for a scene at once).

### 5.2 Referential integrity (Trade **T2**)

When an object or scene is deleted, its edges dangle. **Recommended (T2 = A): cascade-prune on delete** — the
delete endpoints (`scrivi_delete_object`, and the scene-delete path) remove edges that reference the deleted id,
inside the same atomic operation (mirrors EP-027's "self-consistent on open" principle). A load-time repair pass
drops any edge whose endpoint no longer resolves (belt-and-suspenders, like `pruneInconsistentNodes` in history).

*Alternative:* **T2 = B** keep dangling edges + a "broken link" UI. Rejected for v1: a project should always open
self-consistent; broken-link UX is complexity the writer didn't ask for.

---

## 6. C ABI (JSON-over-C-ABI, additive)

New endpoints, mirroring the existing object CRUD style (heap JSON envelopes, `scrivi_free`). All additive to
`scrivi.h`; no existing endpoint changes.

```c
/* Relationship graph (scrivi.relationships.v1) */
const char* scrivi_create_edge(const char* projectRootPath,
                               const char* fromKind, const char* fromID,
                               const char* toKind,   const char* toID,
                               const char* relationTypeCode, const char* note);
const char* scrivi_delete_edge(const char* projectRootPath, const char* edgeID);
/* All edges touching an endpoint (either direction) — the inspector's per-scene query. */
const char* scrivi_list_edges_for(const char* projectRootPath,
                                   const char* endpointKind, const char* endpointID);

/* RelationType vocabulary (scrivi.relation-types.v1) */
const char* scrivi_list_relation_types(const char* projectRootPath);
const char* scrivi_upsert_relation_type(const char* projectRootPath, const char* relationTypeJson);
```

New object kinds (`building`, `vehicle`, `artifact`, `map`, `chronicle`, `world`) need **no new endpoints** — the
existing `scrivi_create/open/save/delete_object` already dispatch on an `objectKind` string; they gain the new
kinds in `ObjectKind` + `objectKindSubdir` + the schema table. The `image`/`subtitle`/`worldID` fields extend
`WorldObjectFields` + `ObjectJson` additively.

---

## 7. Cross-project `worlds` — the hard question (Trade **T5**)

You ruled that a writer may work on **a series of projects spanning multiple worlds**, so a `world` is a
**cross-project container**, not just a per-scene tag. But Scrivi's `.scrivi` package is **project-scoped** —
objects live inside one project. A world shared across projects is in tension with that.

**Options:**

- **T5 = A — `worldID` is a label now; true cross-project sharing is a later epic.** In v1 a `world` is an
  ordinary object *inside* a project (`objects/worlds/…json`), and objects carry an optional `worldID`. Projects
  in a series each hold their own copy; genuine shared-world storage (a world library above the project) is
  designed later. **Lowest risk; unblocks the inspector now; honors the intent without solving cross-project
  storage prematurely.** ✅ *Recommended.*
- **T5 = B — introduce a project-external world store now** (e.g. `~/…/ScriviWorlds/<world>.scrivworld`,
  referenced by projects). Solves sharing properly but is a large architectural addition (new package type, sync,
  identity across packages) that dwarfs the inspector work and should not gate it.
- **T5 = C — worlds live in the app-support root, shared across a machine's projects.** Middle ground, but
  app-support is per-device (not Git-visible, not synced with the project) — a poor home for canonical creative
  source.

**Recommendation: T5 = A for v1**, with a note that cross-project world sharing (B) is its own future epic. This
keeps the model honest (`worldID` exists, objects can be grouped by world) without forcing the cross-package
architecture before the Scene Inspector can ship.

---

## 8. Reconciliation with existing subsystems

- **Scenes/chapters (EP-027):** never become objects. The graph references a scene by `sceneID`; scene-delete
  cascades edge-prune (§5.2). Chapters are addressable the same way if a card ever needs chapter-level links.
- **Existing `ObjectStore` (5 kinds):** extended, not replaced — add kinds + fields; `character`/`location`/
  `rule` files are already compatible; `item` → `artifact` handled by Trade **T3**; `timeline` retired by **T4**.
- **Timeline (EP-016/025):** owns scene story-time; edges carry **no** temporal fields (§5.1) to avoid a second
  source of truth. `chronicle` (narrative) is explicitly distinct from timeline historical events (dots).
- **Assets:** object images reference `assets/` via `assetID` (§4), reusing `scrivi_import_asset`.
- **History (EP-019):** object/edge edits are **not** manuscript-text edits; whether they get their own
  undo is a Doc 2 / later question — this doc does not wire object edits into the text `HistoryService`.

---

## 9. Trade studies (to be ruled — **user decision required before implementation**)

| # | Question | Options | Recommendation |
| --- | --- | --- | --- |
| **T1** | Where edges live | A project-level `relationships.json` · B per-object edge lists · C file-per-edge | **A** |
| **T2** | Dangling-edge integrity | A cascade-prune on delete + load repair · B keep + broken-link UI | **A** |
| **T3** | `item` vs `artifact` | A rename `item`→`artifact` (migrate) · B keep both · C `artifact` new, `item` deprecated-alias | **A or C** (need ruling) |
| **T4** | Legacy `timeline` object kind | A retire it (Timeline Panel owns) · B keep for non-panel timelines | **A** |
| **T5** | Cross-project `worlds` | A `worldID` label now, sharing later · B external world store now · C app-support world store | **A** |
| **T6** | Endpoint model | A uniform `{kind,id}` (objects + scenes) · B objects-only edges + a separate scene-link table | **A** |

---

## 10. Proposed epic/sprint sequencing (per the "phased epics" ruling)

1. **This doc (Doc 1) approved + trades ruled** — design sprint.
2. **Doc 2** — Scene Inspector card UI framework (per-scene card config, rendering, layout Trade T-A, lookahead
   Trade T-B). Design sprint.
3. **EP-A — base card structure**: the inspector card framework + **writing-tool cards** (tags, todo, outline,
   **undo history** [folds in SP-057's history panel], sources, lookahead). Minimal/no new object-model work —
   ships on what exists + Doc 2.
4. **Complete & close EP-019** (its remaining AC2/AC7/AC8 verify; its history panel now lives as a card).
5. **EP-B — worldbuilding-object cards**: implement this doc's model (kinds + graph + C ABI), then the
   character/location/building/vehicle/artifact/rule/map/chronicle/world cards on top.

> Sequencing rationale: EP-A needs almost none of this doc's model (writing-tool cards are mostly app-side +
> existing history), so it can ship and let EP-019 close *before* the larger object-model port (EP-B) lands. This
> doc (Doc 1) is the EP-B foundation; it is written now so EP-A's framework is designed against the eventual model
> rather than boxing it in.

---

## 11. Open questions for the reviewer

1. **T3** — rename `item` → `artifact`, or keep `item` and add `artifact` as distinct? (Cumberland has
   `artifacts`; Scrivi has generic `item`.)
2. **T5** — confirm `worldID`-label-now (A) is acceptable for v1, with true cross-project world sharing as a
   named future epic.
3. Should `source` (citations) be an `objects/` file reusable across scenes, or purely an app-side writing-tool
   card with no core object? (Bridges into Doc 2.)
4. Any kinds beyond the nine worldbuilding kinds you want in v1 (e.g. factions/organizations, events-as-objects)?
