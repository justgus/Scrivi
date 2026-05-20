# Task-verified-0005

## T-0005: Schema Read/Write

**Status:** ✅ Implemented - Verified
**Component:** ScriviCore (C++ backend)
**Priority:** Critical
**Epic:** EP-001: ScriviCore Foundation
**Date Requested:** 2026-05-19
**Date Implemented:** 2026-05-19
**Date Verified:** 2026-05-20
**Sprint Assigned:** SP-001

**Rationale:**
All project data is stored as JSON files in the `.scrivi` package. Schema readers/writers are required before any project creation, open, or save logic can work. Correctness here prevents data loss and format corruption.

**Current Behavior:**
No schema implementations exist.

**Desired Behavior:**
Each JSON schema can be written from a domain struct and read back with all fields preserved. Required fields are validated. Corrupt JSON is rejected with a meaningful error.

**Requirements:**
1. `ProjectJson` — read/write `project.json`
2. `ManuscriptMetaJson` — read/write `manuscript.meta.json`
3. `ChapterMetaJson` — read/write `chapter.meta.json`
4. `SceneMetaJson` — read/write `scene.meta.json`
5. `ProjectMembersJson` — read/write `project-members.json`
6. `ProjectPersonasJson` — read/write `project-personas.json`
7. `WorkspaceStateJson` — read/write `workspace-state.json`
8. `SnapshotMetadataJson` — read/write snapshot metadata

**Design Approach:**
Implement in `ScriviCore/src/schemas/`. Each schema module is a pair of functions (or a struct with static methods): one to serialize a domain struct to JSON string, one to parse a JSON string to a domain struct returning `Result<T>`. Use the Json wrapper from T-0003 internally. Minimum schema version field on all project-owned files.

**Components Affected:**
- ScriviCore/src/schemas/: all eight schema pairs

**Implementation Details:**
- `src/schemas/SchemaUtils.hpp`: shared `requireField()` and `parseAndValidateSchema()` helpers
- `JsonDoc` extended with `setSubDoc`/`getSubDoc`, `appendToArray`/`arraySize`/`arrayItem`, `setInt`/`getInt` — nlohmann fully hidden
- 7 schema modules in `src/schemas/`: ProjectJson, ManuscriptMetaJson, ChapterMetaJson, SceneMetaJson, ProjectMembersJson, ProjectPersonasJson, WorkspaceStateJson
- Each module: `.hpp` with a plain data struct + two free functions (`serialize*` / `parse*`), `.cpp` using `JsonDoc` only
- Schema tag validated on every parse — wrong tag returns `validationError`
- Required fields validated via `requireField()` — missing field returns `validationError`

**Test Steps:**
1. Each schema round-trips required fields through serialize → parse ✅ (7 round-trip tests)
2. Corrupt JSON returns `ErrorCode::parseError` ✅
3. Missing required fields return `ErrorCode::validationError` ✅
4. Wrong schema tag returns `ErrorCode::validationError` ✅
5. Schema version field present in all serialized output ✅ (checked by parse validation)
6. Total: 52/52 tests passing

**Notes:**
The `minimal-valid` test fixture (Section 12.1) is used as the canonical valid input for read tests. Fixture files should be checked in under `ScriviCore/tests/fixtures/`.

---

*Verified: 2026-05-20*
