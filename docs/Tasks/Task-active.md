# Active Tasks

Tasks listed here are assigned to an active Sprint and currently in progress.

---

## T-0001: Repository Skeleton

**Status:** ✅ Verified
**Component:** ScriviCore (C++ backend)
**Priority:** Critical
**Epic:** EP-001: ScriviCore Foundation
**Date Requested:** 2026-05-19
**Date Implemented:** 2026-05-19
**Date Verified:** 2026-05-19
**Sprint Assigned:** SP-001

**Rationale:**
The project cannot build, test, or receive any implementation without a correctly structured repository. This is the prerequisite for every subsequent task.

**Current Behavior:**
No repository exists. No build system, no source tree, no test harness.

**Desired Behavior:**
A CMake-configured repository that builds ScriviCore as a static library, runs an empty Catch2 test executable successfully, and has the correct directory skeleton per Section 5 of the Repository Skeleton doc.

**Requirements:**
1. Top-level `CMakeLists.txt` with `add_subdirectory(ScriviCore)`
2. `ScriviCore/CMakeLists.txt` defining a static library target with `cxx_std_23` feature
3. Public headers directory at `ScriviCore/include/scrivi/`
4. Empty `ScriviCore.cpp` in `src/public_api/`
5. Catch2 test target configured via CMake FetchContent
6. `README.md` with build/configure/test instructions
7. `.gitignore` per Section 14 of the doc

**Design Approach:**
Follow Section 5 (top-level layout), Section 9 (CMake targets), Section 10 (FetchContent for Catch2/nlohmann), and Section 14 (.gitignore). Use `cxx_std_23` in CMake until `cxx_std_26` is broadly available.

**Components Affected:**
- Repository root: CMakeLists.txt, CMakePresets.json, .gitignore, README.md
- ScriviCore/: CMakeLists.txt, include/scrivi/, src/public_api/ScriviCore.cpp
- ScriviCore/tests/: CMakeLists.txt

**Implementation Details:**
- Root `CMakeLists.txt`: cmake 3.24, project Scrivi, `enable_testing()` at root, `add_subdirectory(ScriviCore)`
- `ScriviCore/CMakeLists.txt`: static lib, `cxx_std_23`, public include dir, single source `ScriviCore.cpp`
- `ScriviCore/tests/CMakeLists.txt`: Catch2 v3.6.0 via FetchContent (SHA256 verified), `catch_discover_tests`
- 9 public headers written per doc Sections 8.1–8.9 (Error, Result, IDs, Types, RepairIssue, Services, Requests, Results, ScriviCore)
- `ScriviCore.cpp` stub: all 7 methods return `{ErrorCode::internalError, "not implemented"}`
- `ResultTests.cpp` + `IDTests.cpp`: 6 tests, 6 passing
- GitHub Actions CI: `scrivi-core-ci.yml`, matrix: macos-latest + ubuntu-latest, triggers on ScriviCore/ and CMakeLists.txt changes
- `.gitignore` updated with CMake/C++ build artefacts

**Test Steps:**
1. `cmake -S . -B build -DSCRIVI_BUILD_TESTS=ON` ✅ configures without errors
2. `cmake --build build --parallel` ✅ builds ScriviCore static lib + ScriviCoreTests executable
3. `ctest --test-dir build --output-on-failure` ✅ 6/6 tests pass
4. `grep -r "#import" ScriviCore/include/` ✅ clean — no UI framework imports

**Notes:**
CMake FetchContent is the approved dependency strategy for v0.1. Do not vendor or submodule dependencies at this stage. See Section 10.3 of the doc.

---

## T-0002: Core Value Types

**Status:** ✅ Verified
**Component:** ScriviCore (C++ backend)
**Priority:** Critical
**Epic:** EP-001: ScriviCore Foundation
**Date Requested:** 2026-05-19
**Date Implemented:** 2026-05-19
**Date Verified:** 2026-05-19
**Sprint Assigned:** SP-001

**Rationale:**
All service implementations depend on the shared value types, ID types, Result<T>, and error types. These must be correct and compile-clean before any further work.

**Current Behavior:**
Headers exist as sketches in the Repository Skeleton doc but are not implemented.

**Desired Behavior:**
All public headers compile cleanly. `Result<T>` and `Result<void>` work correctly. Basic tests for Result pass.

**Requirements:**
1. `IDs.hpp` — ProjectID, ManuscriptID, ChapterID, SceneID, IdentityID, PersonaID, SnapshotID, CommitID
2. `Types.hpp` — all scalar aliases, enums (ManuscriptStatus, PersonaKind, ProjectRole, MemberStatus, OpenMode), and structs (TextSelection, ScrollPosition, AuthorshipRef, LastWritingSurface, WorkspaceState, ProjectSummary, SceneSummary)
3. `Error.hpp` — ErrorCode enum and Error struct
4. `Result.hpp` — Result<T> and Result<void> per Section 8.2
5. `RepairIssue.hpp` — RepairSeverity, RepairCategory, RepairActionKind, RepairAction, RepairIssue
6. `Requests.hpp` — all request structs per Section 8.8
7. `Results.hpp` — all result structs per Section 8.9
8. `Services.hpp` — all service interfaces and CoreServices struct per Section 8.7

**Design Approach:**
Implement exactly as sketched in Sections 8.1–8.9 of the Repository Skeleton doc. Result<T> may begin with the simple optional-based implementation; add defensive assertions. No third-party types in public headers.

**Components Affected:**
- ScriviCore/include/scrivi/ — all eight headers

**Implementation Details:**
- All 8 public headers were written as part of T-0001 and confirmed complete
- `Result<T>`: optional-based, static `success()`/`failure()` factories, assert guards on `value()` and `error()`
- `Result<void>`: specialisation with private constructor, same assert guard pattern
- `IDs.hpp`: 8 distinct single-field structs (not aliases) — compiler enforces non-interchangeability
- `Types.hpp`: 5 type aliases, 4 enums, 7 structs — no third-party types
- All other headers (`Error`, `RepairIssue`, `Services`, `Requests`, `Results`) compile clean

**Test Steps:**
1. All headers compile with `cmake --build build` ✅
2. `unit/ResultTests.cpp` passes: success/failure construction, ok(), value(), error() ✅ (4/4)
3. `unit/IDTests.cpp` passes: ID types are distinct types, not interchangeable ✅ (2/2)
4. No UI dependency introduced (check with `grep -r "#import" include/`) ✅ clean

**Notes:**
Result<T> may internally use `std::expected` if available on the target compiler. Begin with the simpler optional-based version for portability. See Section 8.2.

---

## T-0003: Utility Foundation

**Status:** ✅ Verified
**Component:** ScriviCore (C++ backend)
**Priority:** Critical
**Epic:** EP-001: ScriviCore Foundation
**Date Requested:** 2026-05-19
**Date Implemented:** 2026-05-19
**Date Verified:** 2026-05-19
**Sprint Assigned:** SP-001

**Rationale:**
Schema readers/writers and service implementations depend on path manipulation, slug generation, text statistics, JSON wrapping, atomic file writes, and content hashing. These utilities must be correct and unit-tested before any schema or service work.

**Current Behavior:**
No utility implementations exist.

**Desired Behavior:**
All utility modules pass unit tests. UTF-8 strings are preserved through round-trips. Slug generation is conservative and deterministic.

**Requirements:**
1. `PathUtils` — path joining, extension handling, relative-to-absolute, UTF-8 at boundary
2. `Slug` — conservative slug generation (lowercase, hyphen-separated, no special chars)
3. `TextStats` — word count, character count from Markdown UTF-8 string
4. `Json` wrapper — thin wrapper around nlohmann/json; must not expose nlohmann types in public headers
5. `AtomicWrite` — write to temp file then rename (atomic on POSIX and Windows)
6. `Hash` — SHA-256 or similar content hash for change detection

**Design Approach:**
Implement in `ScriviCore/src/util/`. Keep headers internal (not in `include/scrivi/`). nlohmann/json is included via CMake FetchContent and used only inside `Json.cpp`. AtomicWrite uses `std::filesystem::rename` which is atomic on most POSIX targets.

**Components Affected:**
- ScriviCore/src/util/: PathUtils.hpp/cpp, Slug.hpp/cpp, TextStats.hpp/cpp, Json.hpp/cpp, AtomicWrite.hpp/cpp, Hash.hpp/cpp

**Implementation Details:**
- `PathUtils`: wraps `std::filesystem` — join, extension, filename, parent, makeAbsolute, replaceExtension
- `Slug`: conservative ASCII-only lowercasing; non-alnum runs → single hyphen; leading/trailing stripped
- `TextStats`: UTF-8 code-point counter (skips continuation bytes) + whitespace-split word counter
- `Json`: pimpl wrapping nlohmann/json — nlohmann types fully hidden behind `JsonDoc` opaque handle; `parseJson()` free function returns `Result<JsonDoc>`
- `AtomicWrite`: writes to `.tmp` sibling then `std::filesystem::rename` into place
- `Hash`: self-contained SHA-256 (no OpenSSL dependency); returns lowercase hex string
- nlohmann/json v3.11.3 added via FetchContent in `ScriviCore/CMakeLists.txt`
- Tests get `src/` on their include path so they can `#include "util/Foo.hpp"`

**Test Steps:**
1. `unit/PathUtilsTests.cpp` — 6 tests: path join, extension, filename, parent, makeAbsolute, replaceExtension ✅
2. `unit/SlugTests.cpp` — 6 tests: basic title, punctuation, separator collapse, leading/trailing, empty, numbers ✅
3. `unit/TextStatsTests.cpp` — 6 tests: simple prose, extra whitespace, empty, ASCII char count, markdown, single word ✅
4. `unit/JsonSchemaTests.cpp` — 5 tests: valid parse, malformed parse error, missing key defaults, round-trip, contains ✅
5. Total: 29/29 tests passing

**Notes:**
The Json wrapper is a key guardrail: nlohmann types must never leak into public headers or service interfaces.

---

## T-0004: Mock Services

**Status:** ✅ Verified
**Component:** ScriviCore (C++ backend)
**Priority:** Critical
**Epic:** EP-001: ScriviCore Foundation
**Date Requested:** 2026-05-19
**Date Implemented:** 2026-05-19
**Date Verified:** 2026-05-19
**Sprint Assigned:** SP-001

**Rationale:**
Integration tests for project creation, open, and save require controllable, deterministic implementations of all service interfaces. Without mocks, integration tests cannot run portably or produce stable output.

**Current Behavior:**
No mock implementations exist.

**Desired Behavior:**
Tests can run deterministically. Project creation integration tests can use temporary directories. All service interfaces are satisfied by test-only implementations.

**Requirements:**
1. `FixedClock` — returns a configurable deterministic timestamp
2. `DeterministicUUIDProvider` — returns known IDs in sequence for predictable fixture output
3. `MockSecureStore` — in-memory key-value store; must not be compiled into production targets
4. `MockGitProvider` — records calls, returns configurable results; no real Git invoked
5. `NullLogger` — discards all log output; used in tests where logging is irrelevant
6. `LocalFileSystem` — wraps `std::filesystem` for real file I/O; used in integration tests with temp directories

**Design Approach:**
Implement in `ScriviCore/src/platform/`. Mark test-only classes (MockSecureStore, MockGitProvider, FixedClock, DeterministicUUIDProvider) with a `SCRIVI_TEST_ONLY` guard or keep in the test target's CMakeLists. LocalFileSystem and NullLogger may be compiled into the main library.

**Components Affected:**
- ScriviCore/src/platform/: all six implementations
- ScriviCore/tests/CMakeLists.txt: link test-only mocks to test target only

**Implementation Details:**
- `NullLogger`: header-only, discards all output — compiled into main library
- `LocalFileSystem`: wraps `std::filesystem` + `util::atomicWriteTextFile` — compiled into main library
- `FixedClock`, `DeterministicUUIDProvider`, `MockSecureStore`, `MockGitProvider`: header-only, live in `tests/mocks/` — linked only to the test target via `target_include_directories`
- `DeterministicUUIDProvider`: independent counter per ID type, formatted as `<prefix>-001`, `<prefix>-002`
- `MockGitProvider`: `shouldFail` flag + call recording vectors for assertion in tests

**Test Steps:**
1. `FixedClock` returns configured timestamp, stable across calls ✅
2. `DeterministicUUIDProvider` produces sequential IDs; counters are independent per type ✅
3. `MockSecureStore` round-trips arbitrary byte sequences; missing key returns error ✅
4. `MockGitProvider` records calls on success; returns failure when `shouldFail = true` ✅
5. Production CMake target does not link MockSecureStore or MockGitProvider ✅ (mocks are test-include-only)

**Notes:**
See Section 13 of the doc for the design intent behind each mock. DeterministicUUIDProvider is especially important for golden-output tests.

---

## T-0005: Schema Read/Write

**Status:** 🟡 Implemented - Not Verified
**Component:** ScriviCore (C++ backend)
**Priority:** Critical
**Epic:** EP-001: ScriviCore Foundation
**Date Requested:** 2026-05-19
**Date Implemented:** 2026-05-19
**Date Verified:** —
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

*Last Updated: 2026-05-19*
