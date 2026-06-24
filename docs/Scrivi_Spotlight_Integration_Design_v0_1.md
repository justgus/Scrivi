# Scrivi Spotlight Integration Design v0.1

**Status:** Draft — for review before implementation
**Project:** Scrivi
**Task:** T-0175
**Platforms:** macOS first (iOS/iPadOS share the Core Spotlight APIs; visionOS follows iOS)
**Related docs:**
- `Scrivi_Project_Package_Structure_v0_1.md` (on-disk `.scrivi` layout — what there is to index)
- `Scrivi_Minimum_Schema_Set_v0_1.md` (schema fields)
- `Scrivi_Apple_Wrapper_Design_v0_1.md` (ScriviCoreAdapter / ScriviEngine boundary)
- `Scrivi_Architecture_v0_3.md` (pass-by-value, JSON-over-string boundary, Swift = UI only)

---

## 1. Why

Scrivi is an authoring and worldbuilding app. Writers accumulate large, long-lived projects
(manuscripts, characters, locations, items, rules, timelines). Being able to find "the project
and the scene where I wrote about the silver mines of Khaz'tul" from **system Spotlight** —
without opening Scrivi first — is a first-class capability for a writing tool, not a nicety.

This work was triggered while investigating benign `CSInlineDonation … SetStoreUpdateService
was invalidated` console errors (the OS attempting auto-donation and the indexing daemon
rejecting it). The decision (per user) is **not** to suppress that path but to implement
Spotlight support properly so the donations succeed and serve a real feature.

## 2. Scope (approved)

Both layers:

- **Layer 1 — In-app Core Spotlight donations (`CSSearchableItem`).** While Scrivi runs (or on
  project open/save), donate searchable items for project, scenes, and worldbuilding objects.
  Tapping a result deep-links back into Scrivi at that item.
- **Layer 2 — On-disk Spotlight importer (`CSImportExtension`).** A Spotlight importer app
  extension so the OS indexes `.scrivi` package contents at the filesystem level, **even when
  Scrivi is not running**. This is the strongest form of search for a package document type.

## 3. The architecture decision to reconcile (MUST resolve before coding)

`Scrivi_Architecture_v0_3.md` / CLAUDE.md state: **all project I/O and schema read/write live
in C++ (ScriviCore); Swift is UI only; the boundary is JSON-over-`std::string`.**

A Spotlight **importer extension is a separate OS-launched process** that reads a `.scrivi`
package from disk. It is not UI, and it must read project schema content. Two options:

- **Option A (architecturally clean): the importer links ScriviCore.** The extension calls a
  small ScriviCore entry point (e.g. `extractSearchableText(projectPath) -> JSON`) and emits
  Spotlight attributes from that JSON. Keeps "one C++ core owns all project I/O" intact. Cost:
  ScriviCore (and the adapter) must be buildable/linkable into an app-extension target with the
  extension sandbox.
- **Option B (expedient, violates the boundary): the extension parses `.scrivi` JSON directly
  in Swift.** Faster to build, but duplicates project-reading logic outside ScriviCore and
  breaks the "no backend logic in Swift" rule. **Not recommended.**

**Recommendation: Option A.** A new ScriviCore read-only facade method that returns the
indexable text/attributes as JSON, consumed by both Layer 1 (in-app) and Layer 2 (extension),
so there is a single source of indexing truth.

### 3a. Open structural discrepancy to settle first
CLAUDE.md says the adapter lives at `Scrivi/Adapter/` and "there is no SPM package," but the
adapter currently builds under `platforms/apple/` (SPM-style: `ScriviCoreAdapter`,
`ScriviPackageTests`). The extension's link strategy depends on how ScriviCore is actually
consumed by the Apple targets. **Confirm the real adapter location/build graph before deciding
how the extension links ScriviCore.**

### 3b. Gate resolution (T-0176, 2026-06-23) — RESOLVED

Investigated the real build graph (`Scrivi.xcodeproj/project.pbxproj`, `ScriviCore/include/scrivi/`,
`platforms/apple/`). Findings and decisions:

**Build-graph reality (settles §3a):**
- `platforms/apple/` is **not a live SPM package** — it contains only a stale `.build` cache and
  a `.DS_Store`. There is no `Package.swift` and no source there. The earlier "SPM-style
  ScriviCoreAdapter" observation was a leftover artifact directory, not the active build path.
  CLAUDE.md's "there is no SPM package" is therefore **correct**; the `platforms/apple/` cache is
  stale and should be ignored (candidate for cleanup, out of scope for this Epic).
- The **C++ adapter was retired in SP-026.** The live boundary is a **pure C ABI**: ScriviCore
  exposes `scrivi.h` (functions `scrivi_*(...) -> const char*` JSON, freed via `scrivi_free`),
  published as a Clang module (`ScriviCore/include/scrivi/module.modulemap` → `module ScriviCore`).
- The app target consumes ScriviCore via: a **"Build ScriviCore (CMake)" script phase**, linking
  `libScriviCore.a` (`LIBRARY_SEARCH_PATHS = $(SRCROOT)/build/ScriviCore`), with
  `HEADER_SEARCH_PATHS` including `ScriviCore/include/scrivi`. `ScriviEngine.swift` calls the C
  functions and decodes the JSON envelopes.

**Gate decision — boundary: Option A, refined.** The importer extension will call a **new C ABI
function** `scrivi_extract_searchable_text(const char* projectRootPath) -> const char*` (JSON),
implemented in ScriviCore's `public_api` facade, freed with `scrivi_free`. Both Layer 1 (app, via
`ScriviEngine`) and Layer 2 (extension) call the **same** function — single source of indexing
truth. This is *not* "link C++ into the extension"; it is "link the same C static lib + C module
the app already links," which an app-extension target can do cleanly. Option B (Swift JSON
parsing) remains rejected — it would duplicate project I/O outside ScriviCore.

**Consequences for later tasks:**
- T-0178 adds `scrivi_extract_searchable_text` to `scrivi.h` + `public_api`, building on the
  existing schema readers (no new file I/O logic invented — it composes existing readers).
- T-0186 (extension link) replicates the app's ScriviCore link recipe: the CMake-built
  `libScriviCore.a` + the `ScriviCore` Clang module, under the app-extension sandbox. No separate
  adapter is needed.
- Markdown→plain-text (T-0183): **belongs in the ScriviCore facade**, so both layers share
  identical indexed text (the facade already has the scene `.md` content; stripping there avoids
  duplicating a Markdown stripper in Swift and in the extension).
- The `platforms/apple/` stale cache is noted for cleanup but is **not** touched by this Epic.

## 4. What to index (from the package layout)

Source of truth = `.scrivi` package (`Scrivi_Project_Package_Structure_v0_1.md`):

| Searchable item | On-disk source | Key Spotlight attributes |
| --------------- | -------------- | ------------------------ |
| Project | `project.json` | title, displayName, summary |
| Scene | `manuscript/chapter-*/scene-*.md` + `.meta.json` | title (first line / meta), chapter, body text, keywords |
| Character | `objects/characters/*.json` | name, aliases, description |
| Location | `objects/locations/*.json` | name, description |
| Item | `objects/items/*.json` | name, description |
| Rule | `objects/rules/*.json` | name, description |
| Timeline | `objects/timelines/*.json` | name, event labels |

Notes:
- Manuscript bodies are Markdown — strip markup to plain text for the `contentDescription`.
- `domainIdentifier` = project identity (the `identity_…` already logged at open); lets us
  delete-by-domain when a project is removed.
- `uniqueIdentifier` = stable per-item id (scene id / object id) so deep-linking is reliable.

### 4a. Indexable record schema (T-0177, 2026-06-23) — RESOLVED

The output of `scrivi_extract_searchable_text(projectRootPath)` is a single JSON envelope,
following the existing `{ "ok": bool, ... }` Result convention used by every other `scrivi_*`
function. Schema name: `scrivi.searchableContent.v1`.

```json
{
  "ok": true,
  "schema": "scrivi.searchableContent.v1",
  "domainIdentifier": "identity_019ef4ac-...",   // project identity; delete-by-domain key
  "projectRootPath": "/abs/path/MyNovel.scrivi",
  "items": [
    {
      "uniqueIdentifier": "project:<projectID>",
      "kind": "project",                          // project|scene|character|location|item|rule|timeline
      "title": "My Novel",
      "displayName": "My Novel",
      "contentDescription": "<project summary, plain text>",
      "keywords": ["..."],                        // optional; omitted if empty
      "deepLink": "scrivi://open?project=<identity>&item=project:<projectID>"
    },
    {
      "uniqueIdentifier": "scene:<sceneID>",
      "kind": "scene",
      "title": "<scene title or first-line fallback>",
      "containerTitle": "Chapter 3",              // chapter title, for scenes only
      "contentDescription": "<scene body, Markdown stripped to plain text>",
      "keywords": [],
      "deepLink": "scrivi://open?project=<identity>&item=scene:<sceneID>"
    }
    // ... one entry per character/location/item/rule/timeline
  ]
}
```

Field rules:
- **`kind`** drives the Spotlight `contentType` mapping in Layers 1/2.
- **`uniqueIdentifier`** is `"<kind>:<id>"` — globally unique within a project and stable across
  runs (deep-link target).
- **`contentDescription`** is always plain text (Markdown already stripped by the facade, T-0183).
- **`containerTitle`** present only for scenes (the chapter). Omitted otherwise.
- Empty optional fields (`keywords`, `contentDescription`) are omitted, not emitted as null.
- On failure the envelope is `{ "ok": false, "error": { "code": ..., "message": ... } }`,
  matching the existing C API error convention; the C string is still owned by the caller and
  freed via `scrivi_free`.
- **Degenerate cases:** a project with no scenes/objects returns `ok:true` with an `items` array
  containing only the `project` record. A non-existent/invalid project path returns `ok:false`.

This envelope is the contract T-0178 implements and T-0179 tests; Layers 1 and 2 both consume it.

### 4b. T-0178 implementation map (composition plan — verified against the codebase)

The facade **composes existing readers**; it invents no new file I/O. Concrete plan:

- **New facade method:** `Result<ExtractSearchableTextResult> ScriviCore::extractSearchableText(const ExtractSearchableTextRequest&)`
  in `public_api/ScriviCore.cpp`, declared in `include/scrivi/ScriviCore.hpp`. Request carries
  `projectRootPath` (+ `appSupportRoot` if needed for bootstrap, matching `openProject`).
- **Project record:** read `project.json` via `schemas/ProjectJson` (used by `ProjectOpener`);
  pull title/displayName/summary + project identity for `domainIdentifier`.
- **Scene records:** `manuscript::ManuscriptOrderResolver::resolve(projectRoot)` → for each
  `ResolvedScene` use `sceneID`, `title` (fallback to first line), `chapterTitle`
  (→ `containerTitle`); body via `manuscript::SceneReader::readContent(projectRoot, contentPath)`,
  then Markdown-stripped (T-0183 helper, see below).
- **Object records:** enumerate `objects/<kind>/*.json` via `objects::ObjectStore` for kinds
  character/location/item/rule/timeline; map name/aliases/description.
- **Markdown→plain-text (T-0183):** a small `util/MarkdownStrip` helper in ScriviCore (new
  `.cpp/.hpp` under `ScriviCore/src/util/`), unit-tested independently. Shared by all scene
  bodies so Layers 1 & 2 index identical text.
- **C API (`scrivi.h` + `public_api/scrivi_c_api.cpp`):** add
  `const char* scrivi_extract_searchable_text(const char* projectRootPath);` returning the
  `scrivi.searchableContent.v1` JSON, owned by caller, freed via `scrivi_free`. **Changing
  `scrivi.h` and adding `MarkdownStrip.{c,h}pp` triggers the CLAUDE.md pbxproj rule** — update
  `project.pbxproj` (and `ScriviCore/CMakeLists.txt` for the new util source) in the same step.
- **Tests (T-0179):** Catch2 unit test for `MarkdownStrip`; integration test running
  `extractSearchableText` against a canonical fixture project (assert one project record + N
  scene records + object records, plain-text bodies, stable `uniqueIdentifier`s, degenerate
  empty-project and invalid-path cases). Wire into `ScriviCore/tests/`.

Status: T-0176 ✅ resolved, T-0177 ✅ schema fixed, T-0178 ✅ implemented (not verified).

### 4c. T-0178 implementation notes & schema reconciliations (2026-06-23)

Implemented `scrivi_extract_searchable_text` (C ABI) → `ScriviCore::extractSearchableText`
(`public_api/ScriviCore.cpp`), plus `util/MarkdownStrip.{hpp,cpp}` and a Catch2 unit test
(`tests/unit/MarkdownStripTests.cpp`). `ctest` green (217 tests). Three points in §4/§4a were
reconciled against the **actual** on-disk schemas during implementation:

1. **`domainIdentifier` is the `projectID`, not the local `identity_…`.** §4 said the domain key
   is "project identity (the `identity_…` already logged at open)". But `identity_…` is the
   *per-machine* user identity, shared across every project on that machine — using it as the
   Spotlight delete-by-domain key would wipe *all* projects' index entries when one project is
   removed (breaks AC3). The correct per-project key is the project's own `projectID`
   (`project_<uuid>` from `project.json`). The facade emits `domainIdentifier = projectID`, and
   deep links use `project=<projectID>`.
2. **Field mapping follows the real schema, not the §4 wishlist.** `project.json` has only
   `title` (no `displayName`/`summary`) → `title` and `displayName` both carry the title;
   `contentDescription`/`keywords` are omitted for the project record. World objects
   (`scrivi.{character,…}.v1`) expose `displayName`, `notes`, `tags` (no `aliases`/`description`
   field) → `title`/`displayName` ← `displayName`, `contentDescription` ← `notes`,
   `keywords` ← `tags`.
3. **Envelope nests under `result`.** Every existing `scrivi_*` function returns
   `{"ok":true,"result":{…}}`; the §4a flat example was illustrative. The
   `scrivi.searchableContent.v1` payload (`schema`/`domainIdentifier`/`projectRootPath`/`items`)
   sits inside `result`. The `ok:false` error form is unchanged.

Behaviour: unparseable individual object files are skipped (one bad object must not fail the
whole extraction); a missing/empty manuscript is non-fatal (project + objects still returned);
a non-existent/invalid project path (no readable/valid `project.json`) returns `ok:false`.

## 5. Deep-linking back into Scrivi (Layer 1)

- Each `CSSearchableItem` carries a `relatedUniqueIdentifier` / URL encoding
  `scrivi://open?project=<path|identity>&item=<id>`.
- App handles the continuation (`NSUserActivity` /
  `userActivity(CSSearchableItemActionType)` / custom URL scheme) to open the project and
  select the item. Scrivi opens via `NSOpenPanel` today (no `DocumentGroup`), so the open path
  must accept an external "open this project at this item" request.

## 6. Entitlements / project config

- Add the Spotlight/extension entitlements required for `CSSearchableIndex` and the importer
  extension; today `Scrivi.entitlements` has only `app-sandbox=false` +
  `files.user-selected.read-write`.
- New **app-extension target** in `Scrivi.xcodeproj` for the importer. **Per CLAUDE.md: every
  new `.cpp/.hpp/.swift` and the new target MUST be added to `project.pbxproj` in the same step
  — before building.**
- Importer extension declares it handles the `com.caposoft.scrivi.project` UTI.

## 7. Proposed milestones

1. **Design sign-off** — resolve §3 (Option A) and §3a (adapter location). No code.
2. **ScriviCore indexing facade** — read-only `extractSearchableText(projectPath) -> JSON`
   (+ unit tests against a fixture project).
3. **Layer 1** — in-app `CSSearchableIndex` donations on open/save; delete-by-domain on close;
   deep-link continuation.
4. **Layer 2** — `CSImportExtension` target linking ScriviCore (Option A); emits attributes
   from the facade JSON.
5. **Verification** — Spotlight finds project/scene/object content with app closed; result
   opens Scrivi at the item; the original `CSInlineDonation` errors no longer appear because
   donations now succeed.

## 8. Out of scope (v0.1)

- Live incremental re-index on every keystroke (donate on open/save/close granularity first).
- iOS/visionOS-specific UI for results (APIs are shared; revisit when those targets advance).
- Encrypted-project indexing (must not index decrypted content of an encrypted project — flag
  for the future encryption design).

---

*Created 2026-06-23 (T-0175). Draft — implementation gated on §3 / §3a sign-off.*
