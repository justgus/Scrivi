# Task Backlog

Tasks listed here are documented and ready for Sprint assignment. All items are 🔵 Backlog.

---

## T-0014: Introduce `src/domain/` and Relocate Slug and TextStats

**Status:** ✅ Implemented - Verified
**Component:** ScriviCore (C++ backend) — repository structure
**Priority:** High
**Epic:** EP-007: Skeleton v0.2 Migration
**Date Requested:** 2026-05-22
**Date Implemented:** 2026-05-26
**Date Verified:** 2026-05-26
**Sprint Assigned:** SP-004

**Rationale:**
The approved skeleton v0.2 (Section 6) defines a `src/domain/` directory holding `IDs.cpp`, `Slug.cpp`, and `TextStats.cpp`. Currently `Slug` and `TextStats` live in `src/util/`, which conflates infrastructure utilities (path manipulation, atomic I/O, JSON wrapping) with domain-logic types (text statistics, slug generation). The split makes the distinction explicit and aligns the repo with the approved layout before further migration steps.

**Current Behavior:**
`Slug.hpp/.cpp` and `TextStats.hpp/.cpp` reside in `ScriviCore/src/util/`. All consumers include them via `"util/Slug.hpp"` and `"util/TextStats.hpp"`.

**Desired Behavior:**
`Slug.hpp/.cpp` and `TextStats.hpp/.cpp` reside in `ScriviCore/src/domain/`. A thin `src/domain/IDs.cpp` translation unit exists (empty or near-empty — `IDs.hpp` is header-only but the skeleton specifies the `.cpp`). All consumers include via `"domain/Slug.hpp"` and `"domain/TextStats.hpp"`. CMake source list updated. `project.pbxproj` updated in the same step.

**Requirements:**
1. `src/domain/` directory created with `Slug.hpp`, `Slug.cpp`, `TextStats.hpp`, `TextStats.cpp`, `IDs.cpp`
2. `src/util/Slug.hpp/.cpp` and `src/util/TextStats.hpp/.cpp` removed
3. Every `#include "util/Slug.hpp"` and `#include "util/TextStats.hpp"` in `src/` updated to `"domain/Slug.hpp"` and `"domain/TextStats.hpp"`
4. `ScriviCore/CMakeLists.txt` source list reflects new paths and removes old paths
5. `ScriviCore.xcodeproj/project.pbxproj` updated in the same step (non-negotiable)
6. `cmake --build build` succeeds with zero errors after this change

**Design Approach:**
File-move and include-path update only. No logic changes. Verify with a clean CMake configure + build after the move.

**Components Affected:**
- New: `ScriviCore/src/domain/Slug.hpp/.cpp`, `TextStats.hpp/.cpp`, `IDs.cpp`
- Deleted: `ScriviCore/src/util/Slug.hpp/.cpp`, `ScriviCore/src/util/TextStats.hpp/.cpp`
- Modified: Any `src/` file that includes `"util/Slug.hpp"` or `"util/TextStats.hpp"` (likely `src/schemas/`, `src/manuscript/`, `src/project_package/`)
- Modified: `ScriviCore/CMakeLists.txt`
- Modified: `ScriviCore.xcodeproj/project.pbxproj`

**Implementation Details:**
*To be filled in during implementation.*

**Test Steps:**
1. `cmake -S . -B build -DSCRIVI_BUILD_TESTS=ON` — configures without error
2. `cmake --build build --parallel` — builds with zero errors
3. `ctest --test-dir build --output-on-failure` — all previously-passing tests still pass

**Notes:**
This task must be completed before SA-2 (AppSupportLayout relocation) because both touch `CMakeLists.txt` and `project.pbxproj` — doing them separately reduces merge complexity in Xcode.

---

## T-0015: Move `AppSupportLayout` from `src/util/` to `src/platform/`

**Status:** ✅ Implemented - Verified
**Component:** ScriviCore (C++ backend) — repository structure
**Priority:** High
**Epic:** EP-007: Skeleton v0.2 Migration
**Date Requested:** 2026-05-22
**Date Implemented:** 2026-05-26
**Date Verified:** 2026-05-26
**Sprint Assigned:** SP-004

**Rationale:**
The approved skeleton v0.2 (Section 6) places `AppSupportLayout.hpp/.cpp` in `src/platform/` alongside `LocalFileSystem`, `SystemUUIDProvider`, and `NullLogger`. This is semantically correct — `AppSupportLayout` is a platform-level bootstrap concern, not a utility. Currently it lives in `src/util/`, which is reserved for infrastructure helpers (`PathUtils`, `Json`, `AtomicWrite`, `Hash`, `Process`).

**Current Behavior:**
`AppSupportLayout.hpp/.cpp` reside in `ScriviCore/src/util/`. `ScriviCore.cpp` includes it via `"util/AppSupportLayout.hpp"`.

**Desired Behavior:**
`AppSupportLayout.hpp/.cpp` reside in `ScriviCore/src/platform/`. `ScriviCore.cpp` includes it via `"platform/AppSupportLayout.hpp"`. CMake and `project.pbxproj` updated.

**Requirements:**
1. `src/platform/AppSupportLayout.hpp/.cpp` created (moved from `src/util/`)
2. `src/util/AppSupportLayout.hpp/.cpp` deleted
3. `src/public_api/ScriviCore.cpp` `#include` updated from `"util/AppSupportLayout.hpp"` to `"platform/AppSupportLayout.hpp"`
4. `ScriviCore/CMakeLists.txt` source list reflects new path
5. `ScriviCore.xcodeproj/project.pbxproj` updated in the same step
6. `cmake --build build` succeeds after this change

**Design Approach:**
File-move and single include-path update only. No logic changes.

**Components Affected:**
- New: `ScriviCore/src/platform/AppSupportLayout.hpp/.cpp`
- Deleted: `ScriviCore/src/util/AppSupportLayout.hpp/.cpp`
- Modified: `ScriviCore/src/public_api/ScriviCore.cpp`
- Modified: `ScriviCore/CMakeLists.txt`
- Modified: `ScriviCore.xcodeproj/project.pbxproj`

**Implementation Details:**
*To be filled in during implementation.*

**Test Steps:**
1. `cmake --build build --parallel` — zero errors
2. `ctest --test-dir build --output-on-failure` — all tests pass

**Notes:**
Depends on T-0014 completing first so `project.pbxproj` changes are not interleaved in the same Xcode session.

---

## T-0016: Merge `MockServicesTests.cpp` into `ResultTests.cpp`

**Status:** ✅ Implemented - Verified
**Component:** ScriviCore (C++ backend) — test suite
**Priority:** High
**Epic:** EP-007: Skeleton v0.2 Migration
**Date Requested:** 2026-05-22
**Date Implemented:** 2026-05-26
**Date Verified:** 2026-05-26
**Sprint Assigned:** SP-004

**Rationale:**
The skeleton v0.2 (Section 12.1) specifies exactly six canonical unit test files. The current test suite has two extra files: `MockServicesTests.cpp` and `SchemaTests.cpp`. Extra test files are not harmful but deviate from the approved layout and must be consolidated to reach exact conformance. This task handles `MockServicesTests.cpp`.

`MockServicesTests.cpp` contains 7 tests exercising `FixedClock`, `DeterministicUUIDProvider`, `MockSecureStore`, and `MockGitProvider`. These are value-type behavior tests suited for `ResultTests.cpp`, which is the most general-purpose unit test file.

**Current Behavior:**
7 mock service tests live in `tests/unit/MockServicesTests.cpp`. `tests/CMakeLists.txt` includes this file.

**Desired Behavior:**
The 7 tests are appended to `tests/unit/ResultTests.cpp` under a `[mocks]` tag. `MockServicesTests.cpp` is deleted. `tests/CMakeLists.txt` no longer references it.

**Requirements:**
1. All 7 test cases from `MockServicesTests.cpp` moved verbatim into `ResultTests.cpp` (with a section comment separator)
2. `MockServicesTests.cpp` deleted
3. `tests/CMakeLists.txt` updated to remove `MockServicesTests.cpp`
4. All 7 tests continue to pass after the merge
5. `project.pbxproj` updated to remove the deleted file reference

**Design Approach:**
Append, then delete. No test logic changes.

**Components Affected:**
- Modified: `ScriviCore/tests/unit/ResultTests.cpp`
- Deleted: `ScriviCore/tests/unit/MockServicesTests.cpp`
- Modified: `ScriviCore/tests/CMakeLists.txt`
- Modified: `ScriviCore.xcodeproj/project.pbxproj`

**Implementation Details:**
*To be filled in during implementation.*

**Test Steps:**
1. `cmake --build build --parallel` — zero errors
2. `ctest --test-dir build --output-on-failure` — all tests pass including the 7 moved mock tests

**Notes:**
Must be done after T-0014 and T-0015 to avoid compounding `project.pbxproj` conflicts.

---

## T-0017: Merge `SchemaTests.cpp` into `JsonSchemaTests.cpp`

**Status:** ✅ Implemented - Verified
**Component:** ScriviCore (C++ backend) — test suite
**Priority:** High
**Epic:** EP-007: Skeleton v0.2 Migration
**Date Requested:** 2026-05-22
**Date Implemented:** 2026-05-26
**Date Verified:** 2026-05-26
**Sprint Assigned:** SP-004

**Rationale:**
Companion to T-0016. `SchemaTests.cpp` contains schema round-trip tests for all 7 JSON schema types (project, manuscript, chapter, scene, members, personas, workspace state). These belong in `JsonSchemaTests.cpp`, which is the canonical home for JSON schema unit tests per skeleton v0.2 §12.1.

**Current Behavior:**
Schema round-trip tests live in `tests/unit/SchemaTests.cpp`. `tests/CMakeLists.txt` includes this file.

**Desired Behavior:**
All schema tests merged into `tests/unit/JsonSchemaTests.cpp`. `SchemaTests.cpp` deleted. `tests/CMakeLists.txt` updated.

**Requirements:**
1. All test cases from `SchemaTests.cpp` moved verbatim into `JsonSchemaTests.cpp`
2. `SchemaTests.cpp` deleted
3. `tests/CMakeLists.txt` no longer references `SchemaTests.cpp`
4. All tests continue to pass after merge
5. `project.pbxproj` updated

**Design Approach:**
Append, then delete. No test logic changes. Verify no duplicate `TEST_CASE` names after merge.

**Components Affected:**
- Modified: `ScriviCore/tests/unit/JsonSchemaTests.cpp`
- Deleted: `ScriviCore/tests/unit/SchemaTests.cpp`
- Modified: `ScriviCore/tests/CMakeLists.txt`
- Modified: `ScriviCore.xcodeproj/project.pbxproj`

**Implementation Details:**
*To be filled in during implementation.*

**Test Steps:**
1. `cmake --build build --parallel` — zero errors
2. `ctest --test-dir build --output-on-failure` — all tests pass, including all merged schema tests

**Notes:**
After this task, the unit test directory contains exactly the six canonical files from skeleton v0.2 §12.1: `ResultTests.cpp`, `IDTests.cpp`, `SlugTests.cpp`, `PathUtilsTests.cpp`, `TextStatsTests.cpp`, `JsonSchemaTests.cpp`.

---

## T-0018: First Green Build and Full Test Suite Verification

**Status:** ✅ Implemented - Verified
**Component:** ScriviCore (C++ backend) — build system
**Priority:** High
**Epic:** EP-007: Skeleton v0.2 Migration
**Date Requested:** 2026-05-22
**Date Implemented:** 2026-05-26
**Date Verified:** 2026-05-26
**Sprint Assigned:** SP-004

**Rationale:**
T-0014 through T-0017 make structural changes across multiple files. This task is an explicit verification gate: after all Sprint A structural tasks are complete, a clean CMake configure + build + full test run must produce zero errors and zero test failures. Until this passes, Sprint A is not done.

**Current Behavior:**
The codebase has never been built or tested since the architecture change to v0.3. No baseline passing state exists.

**Desired Behavior:**
`cmake -S . -B build -DSCRIVI_BUILD_TESTS=ON && cmake --build build --parallel && ctest --test-dir build --output-on-failure` exits with zero. Every test case in every test file passes.

**Requirements:**
1. CMake configure succeeds with no warnings that indicate structural problems
2. `ScriviCore` static library builds with zero errors
3. `ScriviCoreTests` executable builds with zero errors
4. All unit tests pass (ResultTests, IDTests, SlugTests, PathUtilsTests, TextStatsTests, JsonSchemaTests — including the merged mock and schema tests)
5. All integration tests pass (AppSupportLayoutTests, IdentityTests, CreateProjectTests, OpenProjectTests, SaveSceneTests, ExternalChangeTests, GitSnapshotTests)
6. Any test failures are diagnosed and fixed before Sprint A is marked complete

**Design Approach:**
Run, diagnose, fix. Likely failure modes: missing `#include` updates from T-0014/T-0015, missing `JsonDoc` methods (`appendToArray`, `arraySize`, `arrayItem`) referenced by `SnapshotService` and `GitSnapshotTests`, or `util::replaceExtension` missing from `PathUtils`.

**Components Affected:**
- Whatever files the build reveals as broken

**Implementation Details:**
*To be filled in during implementation.*

**Test Steps:**
1. `cmake -S . -B build -DSCRIVI_BUILD_TESTS=ON` — exits 0
2. `cmake --build build --parallel` — exits 0, zero error lines
3. `ctest --test-dir build --output-on-failure` — all tests pass

**Notes:**
This task is a gate. Sprint A cannot be closed until this task is marked Implemented. If failures are found, they are fixed inline as part of this task — they are build/compile issues arising from the structural changes, not new feature work.

---

## T-0019: Add `SnapshotMetadataJson` Schema

**Status:** ✅ Implemented - Verified
**Component:** ScriviCore (C++ backend) — schemas
**Priority:** High
**Epic:** EP-007: Skeleton v0.2 Migration
**Date Requested:** 2026-05-22
**Date Implemented:** 2026-05-26
**Date Verified:** 2026-05-26
**Sprint Assigned:** SP-005

**Rationale:**
The skeleton v0.2 (Section 6) lists `SnapshotMetadataJson.hpp/.cpp` in `src/schemas/`. This is the only schema file specified by the skeleton that does not yet exist. `SnapshotService` currently writes snapshot metadata by assembling `JsonDoc` objects inline in `SnapshotService.cpp` rather than through a typed schema module. This is inconsistent with how all other JSON files are written (each has a dedicated schema module with `parse*` and `serialize*` functions). Adding the schema module makes the snapshot metadata file consistent and testable via `JsonSchemaTests`.

**Current Behavior:**
`SnapshotService.cpp` writes `scrivi-snapshots.json` by directly calling `JsonDoc` methods inline. No typed parse/serialize functions exist for this schema.

**Desired Behavior:**
`src/schemas/SnapshotMetadataJson.hpp/.cpp` exists with a `SnapshotEntryData` struct and `parseSnapshotMetadata`/`serializeSnapshotMetadata` functions following the same pattern as the other 7 schema modules. `SnapshotService.cpp` updated to use these functions. `CMakeLists.txt` and `project.pbxproj` updated.

**Requirements:**
1. `SnapshotMetadataJson.hpp` defines `SnapshotEntryData` (fields: `snapshotID`, `commitID`, `label`, `note`, `createdAt`, `createdByIdentityID`, `createdByPersonaID`, `createdByDisplayName`) and `SnapshotMetadataData` (wraps a `std::vector<SnapshotEntryData>`)
2. `serializeSnapshotMetadata(const SnapshotMetadataData&)` produces valid JSON matching the on-disk format written by `SnapshotService`
3. `parseSnapshotMetadata(const std::string&)` parses that JSON and returns `Result<SnapshotMetadataData>`
4. `SnapshotService.cpp` updated to use `serializeSnapshotMetadata` / `parseSnapshotMetadata` for reading and writing `scrivi-snapshots.json`
5. `CMakeLists.txt` and `project.pbxproj` updated
6. Build and tests pass after the change

**Design Approach:**
Follow the exact pattern of `WorkspaceStateJson.hpp/.cpp` — struct definition, serialize function, parse function, schema tag validation. The `scrivi-snapshots.json` format is: `{"schema":"scrivi-snapshots","schemaVersion":1,"snapshots":[...]}`.

**Components Affected:**
- New: `ScriviCore/src/schemas/SnapshotMetadataJson.hpp/.cpp`
- Modified: `ScriviCore/src/git/SnapshotService.cpp`
- Modified: `ScriviCore/CMakeLists.txt`
- Modified: `ScriviCore.xcodeproj/project.pbxproj`

**Implementation Details:**
*To be filled in during implementation.*

**Test Steps:**
1. `cmake --build build --parallel` — zero errors
2. `ctest --test-dir build --output-on-failure` — all tests pass
3. `JsonSchemaTests` includes a round-trip test for `SnapshotMetadataJson` (added in T-0020)

**Notes:**
Must be completed before T-0020 (schema test coverage) and T-0021 (M10/M11 verification).

---

## T-0020: Add `SnapshotMetadataJson` Test Coverage

**Status:** ✅ Implemented - Verified
**Component:** ScriviCore (C++ backend) — test suite
**Priority:** Medium
**Epic:** EP-007: Skeleton v0.2 Migration
**Date Requested:** 2026-05-22
**Date Implemented:** 2026-05-26
**Date Verified:** 2026-05-26
**Sprint Assigned:** SP-005

**Rationale:**
Every other schema module has round-trip and error tests in `JsonSchemaTests.cpp`. `SnapshotMetadataJson` (added in T-0019) must have the same coverage. Without tests, any future change to the schema or serialization logic has no safety net.

**Current Behavior:**
No tests exist for snapshot metadata JSON serialization/parsing.

**Desired Behavior:**
`JsonSchemaTests.cpp` contains at least three tests for `SnapshotMetadataJson`: round-trip with one snapshot entry, round-trip with zero entries, and rejection of corrupt JSON.

**Requirements:**
1. `TEST_CASE("SnapshotMetadataJson round-trips with one entry", "[schemas]")` — creates a `SnapshotMetadataData`, serializes it, parses the result, verifies fields match
2. `TEST_CASE("SnapshotMetadataJson round-trips empty snapshots array", "[schemas]")` — zero entries, no error
3. `TEST_CASE("SnapshotMetadataJson rejects corrupt JSON", "[schemas]")` — parse error returned
4. All three tests pass in `ctest`

**Design Approach:**
Append to `JsonSchemaTests.cpp` following the exact style of the existing schema tests.

**Components Affected:**
- Modified: `ScriviCore/tests/unit/JsonSchemaTests.cpp`

**Implementation Details:**
*To be filled in during implementation.*

**Test Steps:**
1. `ctest --test-dir build --output-on-failure` — three new snapshot metadata tests pass

---

## T-0021: Milestone 10 Verification — External Change Scan

**Status:** ✅ Implemented - Verified
**Component:** ScriviCore (C++ backend) — repair layer
**Priority:** High
**Epic:** EP-007: Skeleton v0.2 Migration
**Date Requested:** 2026-05-22
**Date Implemented:** 2026-05-26
**Date Verified:** 2026-05-26
**Sprint Assigned:** SP-005

**Rationale:**
`ExternalChangeScanner`, `RepairClassifier`, and the 6 integration tests in `ExternalChangeTests.cpp` were written prior to any build verification. They are code-complete but have never been compiled or run. This task runs the tests and resolves any failures.

**Current Behavior:**
`ExternalChangeTests.cpp` exists with 6 integration tests. None have been compiled or run.

**Desired Behavior:**
All 6 `ExternalChangeTests` tests pass in `ctest`. `scanForExternalChanges()` correctly detects: clean project (0 issues), missing `.md` (missingContent), missing `.meta.json` (missingMetadata), corrupt `.meta.json` (corruptMetadata), unregistered `.md` (unregisteredManuscriptFile), Git status check via `MockGitProvider`.

**Requirements:**
1. All 6 tests in `ExternalChangeTests.cpp` pass without modification (if changes are needed, they are fixes to implementation, not test relaxations)
2. `ExternalChangeScanner::scan()` correctly traverses the manuscript tree using `FileSystem` service
3. `util::replaceExtension()` exists in `PathUtils` and correctly strips `.meta.json` → `.md`
4. No test-only code compiled into `ScriviCore` production target

**Design Approach:**
Run the tests. Diagnose failures. Fix in the implementation files (`ExternalChangeScanner.cpp`, `PathUtils.cpp`) as needed. The tests are the specification.

**Components Affected:**
- Potentially modified: `ScriviCore/src/repair/ExternalChangeScanner.cpp`
- Potentially modified: `ScriviCore/src/util/PathUtils.cpp/.hpp`

**Implementation Details:**
*To be filled in during implementation.*

**Test Steps:**
1. `ctest --test-dir build --output-on-failure -R ExternalChange` — all 6 tests pass
2. No changes to test assertions (only implementation fixes if required)

**Notes:**
The key risk is `util::replaceExtension()`. `ExternalChangeScanner.cpp:74` calls `util::replaceExtension(util::replaceExtension(sceneRef.metadataPath, ""), ".md")` to derive the content path from the metadata path. If this function is absent or behaves differently, the clean-project test will fail with a missing-content false positive.

---

## T-0022: Milestone 11 Verification — Git Snapshots

**Status:** ✅ Implemented - Verified
**Component:** ScriviCore (C++ backend) — git layer
**Priority:** High
**Epic:** EP-007: Skeleton v0.2 Migration
**Date Requested:** 2026-05-22
**Date Implemented:** 2026-05-26
**Date Verified:** 2026-05-26
**Sprint Assigned:** SP-005

**Rationale:**
`SystemGitProvider`, `SnapshotService`, and the integration tests in `GitSnapshotTests.cpp` were written prior to any build verification. They are code-complete but unrun. This task compiles and runs all Git snapshot tests and resolves any failures.

**Current Behavior:**
`GitSnapshotTests.cpp` exists with 4 mock-based tests and 2 real-git tests (skip-guarded). None have been compiled or run.

**Desired Behavior:**
All 4 mock-based Git snapshot tests pass in `ctest`. Real-git tests pass if `git` is on PATH (otherwise skipped gracefully). `enableGitSnapshots()` and `createSnapshot()` work end-to-end with `MockGitProvider`.

**Requirements:**
1. All 4 mock-based tests pass (`enableGitSnapshots` mock, `alreadyRepository=true` case, `createSnapshot` mock, normal project without Git)
2. Real-git tests pass if `git` is available; skip gracefully if not
3. `JsonDoc::appendToArray`, `JsonDoc::arraySize`, and `JsonDoc::arrayItem` exist and work correctly (used by `SnapshotService` and tests)
4. `SnapshotService` uses `SnapshotMetadataJson` schema functions (after T-0019)

**Design Approach:**
Run the tests. Fix any `JsonDoc` API gaps (missing `appendToArray`/`arraySize`/`arrayItem`) in `Json.hpp/.cpp` if needed. Fix any `SystemGitProvider` failures discovered by the real-git tests.

**Components Affected:**
- Potentially modified: `ScriviCore/src/util/Json.hpp/.cpp` (if `appendToArray`/`arraySize`/`arrayItem` are missing)
- Potentially modified: `ScriviCore/src/git/SystemGitProvider.cpp` (if real-git tests fail)
- Potentially modified: `ScriviCore/src/git/SnapshotService.cpp`

**Implementation Details:**
*To be filled in during implementation.*

**Test Steps:**
1. `ctest --test-dir build --output-on-failure -R GitSnapshot` — all mock-based tests pass
2. Real-git tests pass or are correctly skipped with `SKIP("git not available in PATH")`
3. `ctest --test-dir build --output-on-failure` — full suite still passes after fixes

**Notes:**
The key risk is `JsonDoc` API completeness. `GitSnapshotTests.cpp` calls `parsed.value().arraySize("snapshots")` and `parsed.value().arrayItem("snapshots", 0)` — these must exist on `JsonDoc`. If they don't, this task adds them to `Json.hpp/.cpp`.

Real-git test risk: `SystemGitProvider::commit()` uses inline `-c user.name=` / `-c user.email=` git config. This should work on macOS without a global git identity, but if GPG signing is configured globally it may fail. The fix is to add `--no-gpg-sign` to the commit command args.

---

## T-0023: Extract `ScriviError.swift` as a Separate File

**Status:** ✅ Implemented - Verified
**Component:** Apple platform wrapper
**Priority:** Medium
**Epic:** EP-007: Skeleton v0.2 Migration
**Date Requested:** 2026-05-22
**Date Implemented:** 2026-05-26
**Date Verified:** 2026-05-26
**Sprint Assigned:** SP-006

**Rationale:**
The skeleton v0.2 (Section 7) specifies two separate Swift source files: `ScriviEngine.swift` and `ScriviError.swift`. Currently the error type, envelope types, and `decode()` helper all live inline in `ScriviEngine.swift`. Separating them matches the approved layout and keeps each file focused: `ScriviEngine` is the entry point, `ScriviError` is the error model and envelope decoder.

**Current Behavior:**
`Sources/Scrivi/ScriviEngine.swift` contains: the `ScriviEngine` class, all Swift result types, `ScriviError`, `Envelope<T>`, `ErrorPayload`, and the `decode()` free function.

**Desired Behavior:**
`Sources/Scrivi/ScriviError.swift` contains: `ScriviError`, `Envelope<T>`, `ErrorPayload`, `decode()`. `ScriviEngine.swift` contains: `ScriviEngine` class and all Swift result types only.

**Requirements:**
1. `Sources/Scrivi/ScriviError.swift` created with `ScriviError`, `Envelope`, `ErrorPayload`, `decode()`
2. Those types removed from `ScriviEngine.swift`
3. `swift build` succeeds
4. `swift test` passes all existing interop tests

**Design Approach:**
Extract, delete from original, verify build.

**Components Affected:**
- New: `platforms/apple/Sources/Scrivi/ScriviError.swift`
- Modified: `platforms/apple/Sources/Scrivi/ScriviEngine.swift`

**Implementation Details:**
- Created `platforms/apple/Sources/Scrivi/ScriviError.swift` with `ScriviError`, `Envelope<T>`, `ErrorPayload`, `decode()`.
- Removed those types from `ScriviEngine.swift`, replaced with a comment pointing to the new file.
- `swift build` clean (0 errors, 0 warnings). `swift test` all 5 existing tests pass.

**Test Steps:**
1. `swift build` from `platforms/apple/` — succeeds ✅
2. `swift test` from `platforms/apple/` — all existing tests pass ✅

---

## T-0024: Add Adapter Methods for `scanForExternalChanges`, `enableGitSnapshots`, `createSnapshot`

**Status:** ✅ Implemented - Verified
**Component:** ScriviCoreAdapter (C++) / Apple platform wrapper
**Priority:** High
**Epic:** EP-007: Skeleton v0.2 Migration
**Date Requested:** 2026-05-22
**Date Implemented:** 2026-05-26
**Date Verified:** 2026-05-26
**Sprint Assigned:** SP-006

**Rationale:**
`ScriviCoreAdapter` currently exposes four of the seven ScriviCore facade methods: `ensureLocalIdentity`, `createProject`, `openProject`, `saveScene`. The remaining three — `scanForExternalChanges`, `enableGitSnapshots`, `createSnapshot` — are wired in `ScriviCore.cpp` but have no adapter surface. They cannot be called from Swift. This task adds the three missing adapter methods, completing the adapter's coverage of all MVP operations.

**Current Behavior:**
`ScriviCoreAdapter` has 4 methods. `scanForExternalChanges`, `enableGitSnapshots`, `createSnapshot` are unreachable from Swift.

**Desired Behavior:**
`ScriviCoreAdapter` has 7 methods. All three new methods accept `const char*` inputs and return `std::string` JSON envelopes following the established contract.

**Requirements:**
1. `ScriviCoreAdapter.hpp` declares `scanForExternalChanges(const char* projectRootPath, const char* appSupportRoot)`, `enableGitSnapshots(const char* projectRootPath, const char* identityID, const char* personaID, const char* displayName)`, `createSnapshot(const char* projectRootPath, const char* label, const char* note, const char* identityID, const char* personaID, const char* displayName)`
2. `ScriviCoreAdapter.cpp` implements all three, delegating to `impl_->core`
3. JSON result schemas:
   - `scanForExternalChanges`: `{"projectID":"...","indexesDirty":false,"gitStatusChecked":false,"hasUnsnapshottedChanges":false,"repairIssueCount":0}`
   - `enableGitSnapshots`: `{"gitInitialized":true,"alreadyRepository":false,"initialSnapshotID":"...","initialCommitID":"..."}`
   - `createSnapshot`: `{"snapshotID":"...","commitID":"...","createdAt":"...","created":true}`
4. No ScriviCore types appear in `ScriviCoreAdapter.hpp`
5. `swift build` from `platforms/apple/` succeeds after the change

**Design Approach:**
Follow the exact pattern of the four existing adapter methods: build a request struct from `const char*` inputs, call `impl_->core->method(request)`, serialize the result via `util::JsonDoc`, return `std::string`.

**Components Affected:**
- Modified: `platforms/apple/Sources/ScriviCoreAdapter/ScriviCoreAdapter.hpp`
- Modified: `platforms/apple/Sources/ScriviCoreAdapter/ScriviCoreAdapter.cpp`

**Implementation Details:**
- Added `SystemGitProvider` include to `ScriviCoreAdapter.cpp`.
- Wired `scrivi::git::SystemGitProvider gitProvider` into `Impl` (replaces `nullptr`).
- Added `openProject` mode guard: returns `errorEnvelope` when `mode == repairRequired` or `cannotOpen`, fixing pre-existing test failure.
- Added 3 methods to `ScriviCoreAdapter.hpp` and `.cpp`: `scanForExternalChanges`, `enableGitSnapshots`, `createSnapshot`. JSON schemas match requirement spec.
- `swift build` clean. `swift test` all 8 tests pass.

**Test Steps:**
1. `swift build` from `platforms/apple/` — zero errors ✅
2. `swift test` — all 8 tests pass ✅

---

## T-0025: Add Swift Engine Methods for `scanForExternalChanges`, `enableGitSnapshots`, `createSnapshot`

**Status:** ✅ Implemented - Verified
**Component:** Apple platform wrapper (Swift)
**Priority:** High
**Epic:** EP-007: Skeleton v0.2 Migration
**Date Requested:** 2026-05-22
**Date Implemented:** 2026-05-26
**Date Verified:** 2026-05-26
**Sprint Assigned:** SP-006

**Rationale:**
After T-0024 adds the three adapter methods, `ScriviEngine.swift` must expose them to Swift callers with proper type conversion, envelope decoding, and Swift result types. Without this, the adapter surface exists but Swift code cannot call it.

**Current Behavior:**
`ScriviEngine.swift` has 4 methods. `scanForExternalChanges`, `enableGitSnapshots`, `createSnapshot` are not accessible from Swift.

**Desired Behavior:**
`ScriviEngine.swift` has 7 public methods. Swift callers can call `scanForExternalChanges(projectRootPath:appSupportRoot:)`, `enableGitSnapshots(projectRootPath:authorshipRef:)`, `createSnapshot(projectRootPath:label:note:authorshipRef:)` and receive typed Swift result structs.

**Requirements:**
1. Three new methods added to `ScriviEngine` with `withCString` input conversion and `decode()` envelope decoding
2. Three new `Decodable` Swift result structs: `ScanResult`, `EnableGitResult`, `CreateSnapshotResult` — fields matching the JSON schemas defined in T-0024
3. `swift build` succeeds
4. `swift test` passes all tests

**Design Approach:**
Follow the exact pattern of the existing four Swift methods. No logic — type conversion in, JSON decode out.

**Components Affected:**
- Modified: `platforms/apple/Sources/Scrivi/ScriviEngine.swift`

**Implementation Details:**
- Added 3 public methods to `ScriviEngine`: `scanForExternalChanges(projectRootPath:appSupportRoot:includeGitStatus:)`, `enableGitSnapshots(projectRootPath:authorshipRef:initialSnapshotLabel:)`, `createSnapshot(projectRootPath:authorshipRef:label:note:)`.
- Added 3 `Decodable` result structs: `ScanResult`, `EnableGitResult`, `CreateSnapshotResult`.
- `swift build` clean. All 8 interop tests pass.

**Test Steps:**
1. `swift build` — zero errors ✅
2. All 8 interop tests pass via `swift test` ✅

---

## T-0026: Add Swift Interop Tests for New Adapter Methods

**Status:** ✅ Implemented - Verified
**Component:** Apple platform wrapper (Swift) — test suite
**Priority:** Medium
**Epic:** EP-007: Skeleton v0.2 Migration
**Date Requested:** 2026-05-22
**Date Implemented:** 2026-05-26
**Date Verified:** 2026-05-26
**Sprint Assigned:** SP-006

**Rationale:**
The existing `ScriviInteropTests.swift` covers the original four adapter methods end-to-end. The three new adapter methods added in T-0024/T-0025 need equivalent coverage to confirm the JSON envelope contract works at the Swift boundary.

**Current Behavior:**
`ScriviInteropTests.swift` has tests for `ensureLocalIdentity`, `createProject`, `openProject`, `saveScene`. No tests for the three new methods.

**Desired Behavior:**
`ScriviInteropTests.swift` has tests for all 7 adapter methods. Three new tests cover: `scanForExternalChanges` on a freshly-created project returns zero issues, `enableGitSnapshots` returns `gitInitialized=true` (using `MockGitProvider`-backed adapter), `createSnapshot` returns `created=true`.

**Requirements:**
1. Test `scanForExternalChanges` — creates a project, runs scan, result has `repairIssueCount == 0`
2. Test `enableGitSnapshots` — calls enable on a created project, result has `gitInitialized == true`
3. Test `createSnapshot` — after `enableGitSnapshots`, calls `createSnapshot`, result has `created == true`
4. All 8 total interop tests pass via `swift test`

**Design Approach:**
Append to `ScriviInteropTests.swift`. Use the established `tmpDir()` helper pattern. Each test is self-contained.

**Components Affected:**
- Modified: `platforms/apple/Tests/ScriviInteropTests/ScriviInteropTests.swift`

**Implementation Details:**
- Added `gitAvailable()` helper (uses `Process` to run `git --version`).
- Added `makeProjectFixture()` shared setup helper.
- Added Test 6: `scanForExternalChanges` — fresh project, zero issues.
- Added Test 7: `enableGitSnapshots` — skip-guarded via `withKnownIssue` if no git; asserts `gitInitialized == true`, non-empty `initialSnapshotID`/`initialCommitID`.
- Added Test 8: `createSnapshot` — skip-guarded; creates project, enables git, saves scene, creates snapshot; asserts `created == true`.
- All 8 interop tests pass via `swift test`.

**Test Steps:**
1. `swift test` from `platforms/apple/` — all 8 tests pass ✅

---

---

## T-0027: Full MVP Loop Integration Test

**Status:** ✅ Implemented - Verified
**Component:** ScriviCore (C++ backend) — integration tests
**Priority:** High
**Epic:** EP-002: ScriviCore Services
**Date Requested:** 2026-05-26
**Date Implemented:** 2026-05-26
**Date Verified:** 2026-05-26
**Sprint Assigned:** SP-007

**Rationale:**
EP-002 acceptance criterion 1 requires the full MVP loop from Section 3 to work end-to-end in integration tests. All 13 steps are individually covered across T-0006–T-0010/T-0012/T-0013, but no single test chains them in sequence. This task adds that test, making the criterion unambiguous.

**Current Behavior:**
No single C++ integration test exercises the complete Section 3 loop: identity → createProject → openProject → saveScene → reopen + restore → scan → enableGit → createSnapshot.

**Desired Behavior:**
`integration/MvpLoopTests.cpp` contains one `TEST_CASE` that chains all 13 Section 3 steps in order using `MockGitProvider` and `MockSecureStore`. The test passes via `ctest`.

**Requirements:**
1. New file `ScriviCore/tests/integration/MvpLoopTests.cpp`
2. Single `TEST_CASE("MVP loop — Section 3 end-to-end", ...)` covering all 13 steps
3. `CMakeLists.txt` updated with the new file
4. `ScriviCore.xcodeproj/project.pbxproj` updated in the same step
5. `ctest --test-dir build --output-on-failure` passes

**Components Affected:**
- New: `ScriviCore/tests/integration/MvpLoopTests.cpp`
- Modified: `ScriviCore/tests/CMakeLists.txt`
- Modified: `ScriviCore.xcodeproj/project.pbxproj`

**Implementation Details:**
- Created `ScriviCore/tests/integration/MvpLoopTests.cpp` with one `TEST_CASE` tagged `[integration][EP-002][T-0027]`.
- Chains all 13 Section 3 steps: identity, create, open, save, reopen+restore, scan, missing-file detection (repair mode), enable git, create snapshot.
- Uses `MockSecureStore`, `DeterministicUUIDProvider`, `MockGitProvider`, `FixedClock`, `LocalFileSystem`.
- Added `#include <fstream>` (required for restore-file step).
- `CMakeLists.txt` and `project.pbxproj` updated in same step.
- 96/96 tests pass.

**Test Steps:**
1. `cmake --build build --parallel` — zero errors ✅
2. `ctest --test-dir build --output-on-failure` — 96/96 pass, test #96 is the new MVP loop test ✅

---

---

## T-0028: `RepairIssueJson` Schema Module

**Status:** 🟠 Implemented - Not Verified
**Component:** ScriviCore (C++ backend) — schemas
**Priority:** High
**Epic:** EP-004: Repair and Recovery
**Date Requested:** 2026-05-26
**Date Implemented:** 2026-05-26
**Date Verified:** —
**Sprint Assigned:** SP-008

**Rationale:**
`RepairIssue` and `RepairAction` must cross the C++/Swift boundary as JSON. They are currently dropped at the adapter boundary (only `issueCount` is serialized). A typed schema module — parallel to `SnapshotMetadataJson` — is needed to serialize/deserialize the full issue list including `suggestedActions`. This unblocks T-0033 (adapter serialization) and is required before any repair data can reach Swift.

**Current Behavior:**
No schema module for `RepairIssue`. The `scanForExternalChanges` and `openProject` adapter methods serialize only `issueCount: int`. The full `RepairIssue` list is discarded.

**Desired Behavior:**
`src/schemas/RepairIssueJson.hpp/.cpp` exists. `serializeRepairIssues(vector<RepairIssue>) -> string` and `parseRepairIssues(string_view) -> Result<vector<RepairIssue>>` are implemented and covered by round-trip tests in `JsonSchemaTests.cpp`.

**Requirements:**
1. `ScriviCore/src/schemas/RepairIssueJson.hpp` declares `serializeRepairIssues` and `parseRepairIssues`
2. `ScriviCore/src/schemas/RepairIssueJson.cpp` implements both — all `RepairIssue` fields serialized including the `suggestedActions` array
3. `JsonSchemaTests.cpp` extended with round-trip tests: at least one test per `RepairCategory` variant used in practice, plus an empty list test
4. `CMakeLists.txt` and `project.pbxproj` updated
5. `ctest` passes

**Components Affected:**
- New: `ScriviCore/src/schemas/RepairIssueJson.hpp/.cpp`
- Modified: `ScriviCore/tests/unit/JsonSchemaTests.cpp`
- Modified: `ScriviCore/CMakeLists.txt`
- Modified: `ScriviCore.xcodeproj/project.pbxproj`

**Implementation Details:**
- Created `ScriviCore/src/schemas/RepairIssueJson.hpp` with `serializeRepairIssues` and `parseRepairIssues`.
- Created `ScriviCore/src/schemas/RepairIssueJson.cpp` with full enum ↔ string converters for `RepairSeverity`, `RepairCategory`, and `RepairActionKind` (all variants). Strong ID types (`ProjectID`, `ChapterID`, `SceneID`) accessed via `.value`.
- Added 6 new tests to `JsonSchemaTests.cpp` (#56–#61): empty list, `missingContent` with action, `corruptMetadata` with multiple actions, `unregisteredManuscriptFile`, corrupt JSON rejection, wrong schema tag rejection.
- `CMakeLists.txt` and `project.pbxproj` (IDs `D103`/`D104` in schemas group) updated in same step.
- Build clean, 102/102 tests pass.

**Test Steps:**
1. `cmake --build build --parallel` — zero errors ✅
2. `ctest --test-dir build --output-on-failure` — 102/102 pass, tests #56–#61 are new RepairIssueJson tests ✅

---

## T-0029: `applyRepair` Facade Method — Request, Result, and Dispatch

**Status:** 🟠 Implemented - Not Verified
**Component:** ScriviCore (C++ backend) — public API
**Priority:** High
**Epic:** EP-004: Repair and Recovery
**Date Requested:** 2026-05-26
**Date Implemented:** 2026-05-26
**Date Verified:** —
**Sprint Assigned:** SP-008

**Rationale:**
The facade has no method for applying a repair action. This task adds `ApplyRepairRequest`, `ApplyRepairResult`, and `ScriviCore::applyRepair()`. The method is stateless: it re-reads project state, validates the issue still applies, and dispatches to the correct handler (implemented in T-0030).

**Current Behavior:**
`ScriviCore` has no `applyRepair` method. Callers can detect issues but cannot act on them through the facade.

**Desired Behavior:**
`ScriviCore::applyRepair(const ApplyRepairRequest&) -> Result<ApplyRepairResult>` exists and routes to handlers. Stubs may be used for unimplemented action kinds during this task; T-0030 fills them in.

**Requirements:**
1. `ApplyRepairRequest` added to `Requests.hpp`:
   - `issueID: string`, `projectRootPath`, `appSupportRoot`, `actionKind: RepairActionKind`
   - `targetPath: string` (optional — for relink, move operations)
   - `author: AuthorshipRef`
2. `ApplyRepairResult` added to `Results.hpp`:
   - `issueID: string`, `actionApplied: RepairActionKind`, `resolved: bool`
   - `detail: string` (human-readable outcome), `warnings: vector<RepairIssue>`
3. `ScriviCore::applyRepair()` added to `ScriviCore.hpp` and `ScriviCore.cpp`
4. Dispatch table routes each `RepairActionKind` to its handler (T-0030 stubs acceptable)
5. Returns `ErrorCode::invalidArgument` for unknown `issueID` or unsupported `actionKind`
6. Build passes

**Components Affected:**
- Modified: `ScriviCore/include/scrivi/Requests.hpp`
- Modified: `ScriviCore/include/scrivi/Results.hpp`
- Modified: `ScriviCore/include/scrivi/ScriviCore.hpp`
- Modified: `ScriviCore/src/public_api/ScriviCore.cpp`

**Implementation Details:**
- `ApplyRepairRequest` added to `Requests.hpp` (includes `RepairIssue.hpp` for `RepairActionKind`): `issueID`, `projectRootPath`, `appSupportRoot`, `actionKind`, `targetPath`, `author`.
- `ApplyRepairResult` added to `Results.hpp`: `issueID`, `actionApplied`, `resolved`, `detail`, `warnings`.
- New `RepairDispatcher` service created at `ScriviCore/src/repair/RepairDispatcher.hpp/.cpp`. Stateless: re-scans project on each call via `ExternalChangeScanner`, validates `issueID` exists in current scan result, verifies `actionKind` is in `suggestedActions`, then dispatches to named stub functions (returning `internalError` / "not yet implemented") for each of the 16 action kinds.
- `ScriviCore::applyRepair()` added to `ScriviCore.hpp` and wired in `ScriviCore.cpp`.
- `CMakeLists.txt` updated; `project.pbxproj` updated with IDs `A009`/`A010` in repair group.
- Build clean, 102/102 tests pass.

**Test Steps:**
1. `cmake --build build --parallel` — zero errors ✅
2. Dispatch table compiles with stubs for all action kinds ✅

---

## T-0030: Repair Handlers — Manuscript File Operations

**Status:** 🔵 Backlog
**Component:** ScriviCore (C++ backend) — repair
**Priority:** High
**Epic:** EP-004: Repair and Recovery
**Date Requested:** 2026-05-26
**Date Implemented:** —
**Date Verified:** —
**Sprint Assigned:** SP-009

**Rationale:**
The actual repair logic for each action kind. Eight actions operate on manuscript content and metadata files. Each must be implemented as a named handler in `src/repair/`, called by the `applyRepair` dispatcher.

**Current Behavior:**
No repair handlers exist. `applyRepair` dispatcher (T-0029) has stubs.

**Desired Behavior:**
All 8 handlers implemented and wired into the dispatcher:
1. `relinkToFile` — updates `sceneMetadataPath` / `contentPath` in scene metadata to point to `targetPath`
2. `createEmptyContentFile` — writes an empty `.md` file at the expected content path
3. `markMissing` — sets a `missingContent: true` flag in scene metadata
4. `removeFromProject` — removes the scene entry from chapter metadata; optionally moves the content file to inbox
5. `moveToInbox` — moves the file at `path` to `projectRoot/inbox/dropped-files/`
6. `reloadExternalVersion` — returns the current on-disk content (no write; caller uses the returned markdown)
7. `regenerateMetadata (scene)` — reconstructs `.meta.json` from content file name + inferred title/slug + new `sceneID`
8. `regenerateMetadata (chapter)` — reconstructs `chapter.meta.json` from folder contents

**Requirements:**
1. `src/repair/` contains named handler functions for each of the 8 actions
2. Handlers use `FileSystem&` — no direct `std::filesystem` calls (testable via mock)
3. Backup-required actions (regenerate, rewrite) create a `.bak` copy before modifying
4. Each handler returns `Result<ApplyRepairResult>`
5. Build passes

**Components Affected:**
- Modified/New: `ScriviCore/src/repair/RepairHandlers.hpp/.cpp`
- Modified: `ScriviCore/src/public_api/ScriviCore.cpp` (fill stubs from T-0029)

**Implementation Details:**
*To be filled in during implementation.*

**Test Steps:**
1. `cmake --build build --parallel` — zero errors
2. `ctest` — existing tests still pass (no regressions)
3. T-0032 tests cover all 8 handlers

---

## T-0031: Automatic Rename Detection in `RepairClassifier`

**Status:** 🔵 Backlog
**Component:** ScriviCore (C++ backend) — repair
**Priority:** Medium
**Epic:** EP-004: Repair and Recovery
**Date Requested:** 2026-05-26
**Date Implemented:** —
**Date Verified:** —
**Sprint Assigned:** SP-009

**Rationale:**
Per repair matrix §6.6–6.8: metadata rename and paired rename can be auto-applied when the `sceneID` inside the renamed file matches exactly and only one candidate exists. The `RepairClassifier` needs `possibleRename` and `possiblePairedRename` issue builders, and the scanner needs to attempt auto-repair for the high-confidence cases.

**Current Behavior:**
`RepairClassifier` has 4 classifiers: `missingContent`, `missingMetadata`, `corruptMetadata`, `unregisteredFile`. No rename detection.

**Desired Behavior:**
`RepairClassifier` adds `possibleRename`, `possibleMetadataRename`, `possiblePairedRename`, `possibleChapterFolderRename` issue builders. `ExternalChangeScanner` attempts auto-apply for exact-ID, single-candidate renames; stages an issue for ambiguous cases.

**Requirements:**
1. `RepairClassifier` gains 4 new static methods (issue builders for each rename condition)
2. `ExternalChangeScanner` scans for orphan metadata / unmatched content files and cross-references by `sceneID` / `chapterID`
3. Auto-apply path: `RelativePath` fields updated in metadata when ID match is unambiguous + single candidate
4. Stage-only path: issue added to result when multiple candidates or confidence is low
5. Integration tests cover both paths

**Components Affected:**
- Modified: `ScriviCore/src/repair/RepairClassifier.hpp/.cpp`
- Modified: `ScriviCore/src/scan/ExternalChangeScanner.cpp`

**Implementation Details:**
*To be filled in during implementation.*

**Test Steps:**
1. Auto-rename test: rename a scene's `.meta.json` → scanner auto-relinks, no issue staged
2. Ambiguous rename test: two candidates → issue staged with `possibleRename` category
3. `ctest` passes

---

## T-0032: Integration Tests for `applyRepair`

**Status:** 🔵 Backlog
**Component:** ScriviCore (C++ backend) — integration tests
**Priority:** High
**Epic:** EP-004: Repair and Recovery
**Date Requested:** 2026-05-26
**Date Implemented:** —
**Date Verified:** —
**Sprint Assigned:** SP-009

**Rationale:**
Each repair action kind needs an integration test that creates a project, induces the failure condition, scans to get the `RepairIssue`, then calls `applyRepair` and verifies the outcome on disk.

**Current Behavior:**
No `applyRepair` integration tests exist.

**Desired Behavior:**
`integration/ApplyRepairTests.cpp` contains ≥1 test per action kind (8 handler tests + 2 rename detection tests = 10 tests minimum). All pass via `ctest`.

**Requirements:**
1. New file `ScriviCore/tests/integration/ApplyRepairTests.cpp`
2. One `TEST_CASE` per action kind, each creating a fresh `TempDir`, inducing the condition, scanning, applying, and verifying
3. `CMakeLists.txt` and `project.pbxproj` updated
4. All existing tests continue to pass

**Components Affected:**
- New: `ScriviCore/tests/integration/ApplyRepairTests.cpp`
- Modified: `ScriviCore/tests/CMakeLists.txt`
- Modified: `ScriviCore.xcodeproj/project.pbxproj`

**Implementation Details:**
*To be filled in during implementation.*

**Test Steps:**
1. `cmake --build build --parallel` — zero errors
2. `ctest --test-dir build --output-on-failure` — all tests pass

---

## T-0033: Adapter — `applyRepair` Method + Full `RepairIssue` Serialization in Scan/Open Results

**Status:** 🔵 Backlog
**Component:** ScriviCoreAdapter (C++) / Apple platform wrapper
**Priority:** High
**Epic:** EP-004: Repair and Recovery
**Date Requested:** 2026-05-26
**Date Implemented:** —
**Date Verified:** —
**Sprint Assigned:** SP-009

**Rationale:**
For `applyRepair` to be callable from Swift, two things are needed: (1) the adapter must expose `applyRepair`, and (2) the `scanForExternalChanges` and `openProject` adapter methods must serialize the full `RepairIssue` list (including `issueID` and `suggestedActions`) so the caller has the `issueID` to pass back. Currently both adapters drop the issue list and emit only `issueCount`.

**Current Behavior:**
`ScriviCoreAdapter` has no `applyRepair` method. `scanForExternalChanges` and `openProject` JSON results contain `issueCount: int` only.

**Desired Behavior:**
`ScriviCoreAdapter` exposes `applyRepair(const char* issueID, const char* projectRootPath, const char* appSupportRoot, int actionKind, const char* targetPath, const char* identityID, const char* personaID, const char* displayName) -> std::string`. `scanForExternalChanges` and `openProject` JSON results include a `repairIssues` array using `RepairIssueJson::serializeRepairIssues`. `swift test` still passes all existing 8 interop tests.

**Requirements:**
1. `ScriviCoreAdapter.hpp` declares `applyRepair`
2. `ScriviCoreAdapter.cpp` implements it, using `RepairIssueJson` for serialization
3. `scanForExternalChanges` adapter method updated: `repairIssues` array in JSON (replaces `issueCount`)
4. `openProject` adapter method updated: `repairIssues` array in JSON when issues exist
5. `swift build` passes
6. `swift test` — all 8 existing interop tests pass
7. `ScanResult` Swift struct updated: `repairIssues` array replaces `issueCount`

**Components Affected:**
- Modified: `platforms/apple/Sources/ScriviCoreAdapter/ScriviCoreAdapter.hpp`
- Modified: `platforms/apple/Sources/ScriviCoreAdapter/ScriviCoreAdapter.cpp`
- Modified: `platforms/apple/Sources/Scrivi/ScriviEngine.swift`

**Implementation Details:**
*To be filled in during implementation.*

**Test Steps:**
1. `swift build` — zero errors
2. `swift test` — all 8 existing tests pass
3. New interop test (optional, in T-0034 or future) verifies `repairIssues` array content

---

*Last Updated: 2026-05-26 (T-0028 through T-0033 added for EP-004)*
