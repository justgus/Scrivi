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
