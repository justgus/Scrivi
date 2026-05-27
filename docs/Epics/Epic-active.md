# Active Epics

---

## EP-005: Full Project Package — Objects, Assets, and Comments

**Status:** 🟡 Active
**Goal:** Implement the full `.scrivi` project package beyond manuscripts: objects (characters, locations, items, rules, timelines), assets (with metadata sidecars), comments, and inbox. Delivers the worldbuilding layer that makes Scrivi more than a scene editor.
**Date Created:** 2026-05-20
**Start Date:** 2026-05-27
**Target Close Date:** TBD
**Actual Close Date:** —

### Acceptance Criteria

- [x] Character object schema defined (`objects/characters/<slug>.json`) and implemented (`ObjectJson` schema module)
- [x] `createObject` / `openObject` / `saveObject` / `deleteObject` facade methods for character objects
- [ ] Location, item, rule, and timeline object schemas defined and implemented
- [ ] `createObject` / `openObject` / `saveObject` / `deleteObject` generalized to all object types
- [ ] Asset metadata sidecar schema defined (`<filename>.meta.json` in `assets/`)
- [ ] `importAsset` / `listAssets` / `removeAsset` facade methods
- [ ] Comments schema defined (`comments/<scope>/<objectID>.comments.json`) and implemented
- [ ] `addComment` / `listComments` / `resolveComment` facade methods
- [ ] Inbox implemented (`inbox/dropped-files/`) with `listInbox` / `importFromInbox` facade methods
- [ ] All object/asset/comment types covered by integration tests
- [ ] `ctest --test-dir build --output-on-failure` passes all tests
- [ ] `ScriviCoreAdapter` exposes all new facade methods with JSON envelope output
- [ ] `swift test` passes all interop tests

### Sprints

| Sprint | Title | Status | Dates |
| ------ | ----- | ------ | ----- |
| SP-010 | Objects Layer Foundation — Character Schema and CRUD | ✅ Closed | 2026-05-27 – 2026-05-27 |

### Tasks

| ID     | Title | Status |
| ------ | ----- | ------ |
| T-0034 | Character Object Schema (`ObjectJson`) | ✅ Implemented - Verified |
| T-0035 | `createObject` / `openObject` / `saveObject` / `deleteObject` Facade — Characters | ✅ Implemented - Verified |
| T-0036 | Integration Tests for Character Object CRUD | ✅ Implemented - Verified |

### Issues

| ID | Title | Status |
| -- | ----- | ------ |
| —  | None  | —      |

### Scope Notes

Drawn from `Scrivi_Project_Package_Structure_v0_1.md` (objects/, assets/, comments/, inbox/ layout). None of this package structure is implemented in EP-001 through EP-004. Tasks are defined incrementally as each object type is completed.

**Out of scope for EP-005:**
- SwiftUI worldbuilding UI (EP-006)
- Cloud sync of object data
- Relationship graphs between objects (deferred to EP-005+ or a dedicated Epic)
- Export of object data

### Completion Summary

*To be filled in when EP-005 reaches 🟠 Complete.*

---

*Last Updated: 2026-05-27 (SP-010 closed; T-0034/T-0035/T-0036 verified; 121 tests passing)*
