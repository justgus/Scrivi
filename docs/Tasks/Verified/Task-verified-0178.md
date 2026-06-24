# T-0178: ScriviCore `extractSearchableText(projectPath) -> JSON` read-only facade

**Status:** ✅ Implemented - Verified
**Component:** ScriviCore `public_api` + C ABI + `util/MarkdownStrip`
**Epic:** EP-017
**Sprint:** SP-044
**Date Requested:** 2026-06-23
**Date Implemented:** 2026-06-23
**Date Verified:** 2026-06-23
**Design Reference:** Spotlight Integration Design v0.1 §4a, §4b, §4c
**Related:** T-0176 (boundary), T-0177 (schema), T-0179 (tests), T-0183 (Markdown strip — core landed here)

**Goal:**
A read-only ScriviCore facade returning a `.scrivi` project's indexable content as the
`scrivi.searchableContent.v1` JSON envelope, exposed through the C ABI — the single source of
indexing truth for both Spotlight layers. Composes existing readers; invents no new file I/O.

**Implementation:**
- **C ABI:** `scrivi_extract_searchable_text(projectRootPath)` (`scrivi.h`,
  `public_api/scrivi_c_api.cpp`), JSON owned by caller, freed via `scrivi_free`.
- **Facade:** `ScriviCore::extractSearchableText` (`public_api/ScriviCore.cpp`) composing
  `ProjectJson` + `ManuscriptOrderResolver` + `SceneReader` + per-kind `objects/` enumeration.
- **Types:** `ExtractSearchableTextRequest` / `ExtractSearchableTextResult` / `SearchableItem`
  in the public headers.
- **Markdown→plain-text:** new `util/MarkdownStrip.{hpp,cpp}` (the T-0183 core, landed early per
  design §4b) so both layers index identical text.

**Schema reconciliations (design doc §4c):**
1. `domainIdentifier` = `projectID` (per-project delete-by-domain key), **not** the per-machine
   `identity_…` — using the latter would wipe all projects' index entries when one is removed.
2. Field mapping follows the real schema: project `title`→title/displayName (no summary); world
   objects `displayName`→title/displayName, `notes`→contentDescription, `tags`→keywords.
3. Envelope nested under `result` to match the existing `{"ok":true,"result":{…}}` convention.

**Behaviour:** unparseable individual object files are skipped (non-fatal); missing/empty
manuscript non-fatal; invalid project path → `ok:false`.

**Build/Test status:** `ctest` green. New files live under the `ScriviCore` folder reference in
Xcode (built by the CMake script phase) — no per-file `project.pbxproj` entry required.

**Acceptance Criteria:**
- [x] `extractSearchableText(projectPath) -> JSON` implemented in ScriviCore and exposed through the C API.
- [x] One record per project / scene / world object, per the `scrivi.searchableContent.v1` schema.
- [x] No UI types and no third-party types in public headers; no backend logic in Swift.

*Verified by the user 2026-06-23 (SP-044 close).*
