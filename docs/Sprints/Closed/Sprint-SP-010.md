# SP-010: Objects Layer Foundation — Character Schema and CRUD

**Status:** ✅ Closed
**Epic:** EP-005: Full Project Package — Objects, Assets, and Comments
**Goal:** Define the character object schema, implement the `ObjectJson` schema module for serializing/deserializing character objects, add `createObject` / `openObject` / `saveObject` / `deleteObject` facade methods scoped to characters, and cover them with integration tests. This establishes the pattern for all future object types (locations, items, rules, timelines).
**Start Date:** 2026-05-27
**End Date:** 2026-05-27

---

### Assigned Tasks

| ID     | Title | Priority | Status |
| ------ | ----- | -------- | ------ |
| T-0034 | Character Object Schema (`ObjectJson`) | High | ✅ Implemented - Verified |
| T-0035 | `createObject` / `openObject` / `saveObject` / `deleteObject` Facade — Characters | High | ✅ Implemented - Verified |
| T-0036 | Integration Tests for Character Object CRUD | High | ✅ Implemented - Verified |

### Assigned Issues

| ID | Title | Severity | Status |
| -- | ----- | -------- | ------ |
| —  | None  | —        | —      |

---

### Retrospective

**What was delivered:**

- **T-0034 — Character Object Schema:** `ObjectTypes.hpp` defines `CharacterObject` with all fields (objectID, slug, displayName, status, createdAt/By, modifiedAt/By, notes, tags, attributes). `ObjectJson.hpp/.cpp` serializes/deserializes under schema tag `scrivi.object.character.v1`. Tags encoded as `[{v:"..."}]`; attributes as `[{k:"...", v:"..."}]` via existing `JsonDoc` API. 3 unit tests added (round-trip minimal, round-trip with tags/attributes, wrong-schema rejection).

- **T-0035 — CRUD Facade:** `ObjectStore` class in `src/objects/` handles all four operations. `findByID` scans `objects/characters/` directory and matches on `objectID.value` (no separate index file). `create` derives slug from displayName if omitted, enforces slug uniqueness via file existence check. `save` writes `.bak` before overwriting (best-effort). `UUIDProvider` interface extended with `newObjectID()`; `SystemUUIDProvider` uses `character_` prefix; `DeterministicUUIDProvider` uses `obj-N` sequence. `ErrorCode::ioError` for not-found; `ErrorCode::invalidArgument` for duplicate slug.

- **T-0036 — Integration Tests:** `ObjectCrudTests.cpp` with `ObjectFixture` (real temp dirs, real project bootstrap). 6 tests covering: slug derivation, explicit slug override, openObject field verification, saveObject mutation + re-open, deleteObject + subsequent open failure, duplicate slug rejection.

**Final test count:** 121/121 passing

**Key patterns established for future object types:**
- Schema tag convention: `scrivi.object.<type>.v1`
- File location: `objects/<type>s/<slug>.json`
- ID prefix: `<type>_` (e.g. `character_`)
- CRUD via `ObjectStore` (or per-type variant)

---

*Closed: 2026-05-27*
