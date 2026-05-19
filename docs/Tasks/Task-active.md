# Active Tasks

Tasks listed here are assigned to an active Sprint and currently in progress.

---

## T-0001: Repository Skeleton

**Status:** 🟡 Active
**Component:** ScriviCore (C++ backend)
**Priority:** Critical
**Epic:** EP-001: ScriviCore Foundation
**Date Requested:** 2026-05-19
**Date Implemented:** —
**Date Verified:** —
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
*To be filled in during implementation.*

**Test Steps:**
1. `cmake -S . -B build -DSCRIVI_BUILD_TESTS=ON` — should configure without errors
2. `cmake --build build` — should build ScriviCore static library and test executable
3. `ctest --test-dir build` — empty test run should exit 0
4. Verify no UI framework headers appear in include/scrivi/

**Notes:**
CMake FetchContent is the approved dependency strategy for v0.1. Do not vendor or submodule dependencies at this stage. See Section 10.3 of the doc.

---

## T-0002: Core Value Types

**Status:** 🟡 Active
**Component:** ScriviCore (C++ backend)
**Priority:** Critical
**Epic:** EP-001: ScriviCore Foundation
**Date Requested:** 2026-05-19
**Date Implemented:** —
**Date Verified:** —
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
*To be filled in during implementation.*

**Test Steps:**
1. All headers compile with `cmake --build build`
2. `unit/ResultTests.cpp` passes: success/failure construction, ok(), value(), error()
3. `unit/IDTests.cpp` passes: ID types are distinct types, not interchangeable
4. No UI dependency introduced (check with `grep -r "#import" include/`)

**Notes:**
Result<T> may internally use `std::expected` if available on the target compiler. Begin with the simpler optional-based version for portability. See Section 8.2.

---

## T-0003: Utility Foundation

**Status:** 🟡 Active
**Component:** ScriviCore (C++ backend)
**Priority:** Critical
**Epic:** EP-001: ScriviCore Foundation
**Date Requested:** 2026-05-19
**Date Implemented:** —
**Date Verified:** —
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
*To be filled in during implementation.*

**Test Steps:**
1. `unit/PathUtilsTests.cpp` — path join, extension extract, UTF-8 round-trip
2. `unit/SlugTests.cpp` — known inputs produce expected slugs; no special characters survive
3. `unit/TextStatsTests.cpp` — word count matches expected values for known Markdown strings
4. `unit/JsonSchemaTests.cpp` — JSON round-trip preserves all fields; malformed JSON returns error

**Notes:**
The Json wrapper is a key guardrail: nlohmann types must never leak into public headers or service interfaces.

---

*Last Updated: 2026-05-19*
