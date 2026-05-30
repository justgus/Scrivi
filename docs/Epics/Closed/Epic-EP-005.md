# Epic EP-005 — Closed

## EP-005: Full Project Package — Objects, Assets, and Comments

**Status:** ✅ Closed
**Goal:** Implement the full `.scrivi` project package beyond manuscripts: objects (characters, locations, items, rules, timelines), assets (with metadata sidecars), comments, and inbox. Delivers the worldbuilding layer that makes Scrivi more than a scene editor.
**Date Created:** 2026-05-20
**Start Date:** 2026-05-27
**Target Close Date:** TBD
**Actual Close Date:** 2026-05-28

### Acceptance Criteria

- [x] Character object schema defined (`objects/characters/<slug>.json`) and implemented (`ObjectJson` schema module)
- [x] `createObject` / `openObject` / `saveObject` / `deleteObject` facade methods for character objects
- [x] Location, item, rule, and timeline object schemas defined and implemented
- [x] `createObject` / `openObject` / `saveObject` / `deleteObject` generalized to all object types
- [x] Asset metadata sidecar schema defined (`<filename>.meta.json` in `assets/`)
- [x] `importAsset` / `listAssets` / `removeAsset` facade methods
- [x] Comments schema defined (`comments/<scope>/<objectID>.comments.json`) and implemented
- [x] `addComment` / `listComments` / `resolveComment` facade methods
- [x] Inbox implemented (`inbox/dropped-files/`) with `listInbox` / `importFromInbox` facade methods
- [x] All object/asset/comment types covered by integration tests
- [x] `ctest --test-dir build --output-on-failure` passes all tests (158/158)
- [x] `ScriviCoreAdapter` exposes all new facade methods with JSON envelope output
- [x] `swift test` passes all interop tests (17/17)

### Sprints

| Sprint | Title | Status | Dates |
| ------ | ----- | ------ | ----- |
| SP-010 | Objects Layer Foundation — Character Schema and CRUD | ✅ Closed | 2026-05-27 – 2026-05-27 |
| SP-011 | Objects Layer — Remaining Types and Generalized CRUD | ✅ Closed | 2026-05-28 – 2026-05-28 |
| SP-012 | Assets and Comments Layer | ✅ Closed | 2026-05-28 – 2026-05-28 |
| SP-013 | Inbox and Adapter Completion | ✅ Closed | 2026-05-28 – 2026-05-28 |

### Tasks

| ID     | Title | Status |
| ------ | ----- | ------ |
| T-0034 | Character Object Schema (`ObjectJson`) | ✅ Implemented - Verified |
| T-0035 | `createObject` / `openObject` / `saveObject` / `deleteObject` Facade — Characters | ✅ Implemented - Verified |
| T-0036 | Integration Tests for Character Object CRUD | ✅ Implemented - Verified |
| T-0037 | Remaining Object Types — Location, Item, Rule, Timeline Schemas | ✅ Implemented - Verified |
| T-0038 | Generalize Object CRUD Facade to All Object Types | ✅ Implemented - Verified |
| T-0039 | Integration Tests for All Object Types CRUD | ✅ Implemented - Verified |
| T-0040 | Asset Metadata Sidecar Schema (`AssetMetaJson`) | ✅ Implemented - Verified |
| T-0041 | `importAsset` / `listAssets` / `removeAsset` Facade Methods | ✅ Implemented - Verified |
| T-0042 | Integration Tests for Asset Operations | ✅ Implemented - Verified |
| T-0043 | Comments Schema (`CommentJson`) and Comment Types | ✅ Implemented - Verified |
| T-0044 | `addComment` / `listComments` / `resolveComment` Facade Methods | ✅ Implemented - Verified |
| T-0045 | Integration Tests for Comment Operations | ✅ Implemented - Verified |
| T-0046 | Inbox — `listInbox` / `importFromInbox` Facade Methods | ✅ Implemented - Verified |
| T-0047 | `ScriviCoreAdapter` — Expose All EP-005 Facade Methods | ✅ Implemented - Verified |

### Issues

| ID | Title | Status |
| -- | ----- | ------ |
| —  | None  | —      |

### Scope Notes

Drawn from `Scrivi_Project_Package_Structure_v0_1.md` (objects/, assets/, comments/, inbox/ layout). None of this package structure was implemented in EP-001 through EP-004. Tasks were defined incrementally as each layer was completed.

**Out of scope for EP-005 (deferred):**
- SwiftUI worldbuilding UI (EP-006)
- Cloud sync of object data
- Relationship graphs between objects
- Export of object data

### Completion Summary

EP-005 delivered the full worldbuilding layer for the `.scrivi` project package across 4 sprints and 14 tasks, completed in two sessions on 2026-05-27 and 2026-05-28:

**Objects layer (SP-010, SP-011 — T-0034–T-0039):** Character, location, item, rule, and timeline schemas under `ObjectJson`. Generalized `createObject` / `openObject` / `saveObject` / `deleteObject` CRUD dispatching by `ObjectKind` enum. 30 integration tests across all five types.

**Assets layer (SP-012 — T-0040–T-0042):** `AssetMetaJson` sidecar schema keyed `scrivi.asset.meta.v1`. `AssetStore` imports binary files (binary-safe via `readTextFile`), writes `.meta.json` sidecars, lists by category scan, removes both files by assetID scan. 4 integration tests.

**Comments layer (SP-012 — T-0043–T-0045):** `CommentJson` thread schema keyed `scrivi.comments.v1`. `CommentStore` uses atomic read-modify-write of `comments/<scopeKind>/<targetID>.comments.json`. Supports scene and object scopes independently. 7 integration tests.

**Inbox layer (SP-013 — T-0046):** `InboxStore` scanning `inbox/dropped-files/`. Three actions: `importAsAsset` (delegates to `AssetStore` then removes source), `ignore` (no-op), `deleteFile`. `ProjectCreator` amended to create `inbox/dropped-files/` on every new project. 6 integration tests.

**Adapter completion (SP-013 — T-0047):** All 14 new C++ facade methods exposed through `ScriviCoreAdapter` with JSON envelope output. `ScriviEngine.swift` wraps all methods with full `withCString` chains. 12 new Swift result types. 17 interop tests total (9 prior + 8 new covering objects, assets, comments, inbox).

Final state at close: **158/158 CTest passing, 17/17 `swift test` passing, 0 issues, 14/14 tasks verified.**

---

*Closed: 2026-05-28*
