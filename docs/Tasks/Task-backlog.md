# Task Backlog

Tasks listed here are documented and ready for Sprint assignment. All items are 🔵 Backlog.

> T-0001, T-0002, T-0003 have been moved to Task-active.md (assigned to SP-001).

---

## T-0001: Repository Skeleton

**Status:** 🟡 Active → see Task-active.md
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

**Status:** 🟡 Active → see Task-active.md
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

**Status:** 🟡 Active → see Task-active.md
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

## T-0004: Mock Services

**Status:** 🔵 Backlog
**Component:** ScriviCore (C++ backend)
**Priority:** Critical
**Epic:** EP-001: ScriviCore Foundation
**Date Requested:** 2026-05-19
**Date Implemented:** —
**Date Verified:** —
**Sprint Assigned:** Not Assigned

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
*To be filled in during implementation.*

**Test Steps:**
1. A test using `DeterministicUUIDProvider` produces the same IDs on every run
2. A test using `FixedClock` produces stable timestamps in JSON output
3. `MockGitProvider` can be configured to succeed or fail; callers see correct Result<T>
4. `MockSecureStore` round-trips arbitrary byte sequences
5. Production CMake target does not link MockSecureStore or MockGitProvider

**Notes:**
See Section 13 of the doc for the design intent behind each mock. DeterministicUUIDProvider is especially important for golden-output tests.

---

## T-0005: Schema Read/Write

**Status:** 🔵 Backlog
**Component:** ScriviCore (C++ backend)
**Priority:** Critical
**Epic:** EP-001: ScriviCore Foundation
**Date Requested:** 2026-05-19
**Date Implemented:** —
**Date Verified:** —
**Sprint Assigned:** Not Assigned

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
*To be filled in during implementation.*

**Test Steps:**
1. `unit/JsonSchemaTests.cpp` — for each schema: write a known struct, read it back, assert field equality
2. Corrupt JSON input returns `Result::failure` with `ErrorCode::parseError`
3. Missing required fields return `Result::failure` with `ErrorCode::validationError`
4. Round-trip through `minimal-valid` fixture produces identical output
5. Schema version field is present in all written files

**Notes:**
The `minimal-valid` test fixture (Section 12.1) is used as the canonical valid input for read tests. Fixture files should be checked in under `ScriviCore/tests/fixtures/`.

---

## T-0006: Project Creation

**Status:** 🔵 Backlog
**Component:** ScriviCore (C++ backend)
**Priority:** High
**Epic:** EP-002: ScriviCore Services
**Date Requested:** 2026-05-19
**Date Implemented:** —
**Date Verified:** —
**Sprint Assigned:** Not Assigned

**Rationale:**
Project creation is the entry point for the entire writer workflow. Without it, no project exists to open, save, or scan.

**Current Behavior:**
No project creation logic exists.

**Desired Behavior:**
`ScriviCore::createProject()` creates a minimal `.scrivi` project package on disk with first scene Markdown, paired metadata, and workspace state.

**Requirements:**
1. `ProjectCreator` creates the full `.scrivi` directory structure
2. Initial `project.json`, `manuscript.meta.json`, `chapter.meta.json`, `scene.meta.json` written
3. First scene `.md` content file created (empty or with placeholder)
4. `workspace-state.json` written to app-local path
5. `project-members.json` and `project-personas.json` written with author identity
6. If `enableGitSnapshots = true`: initialize local Git repo and create initial snapshot
7. Returns `CreateProjectResult` with all generated IDs and paths

**Design Approach:**
Implement `ProjectCreator` in `ScriviCore/src/project_package/`. `ScriviCore::createProject()` in `src/public_api/ScriviCore.cpp` delegates to `ProjectCreator` with injected services. All file I/O through `FileSystem` service. All ID generation through `UUIDProvider`. All timestamps through `Clock`.

**Components Affected:**
- ScriviCore/src/project_package/: ProjectCreator.hpp/cpp
- ScriviCore/src/public_api/ScriviCore.cpp
- ScriviCore/tests/integration/CreateProjectTests.cpp

**Implementation Details:**
*To be filled in during implementation.*

**Test Steps:**
1. `integration/CreateProjectTests.cpp` — create project in temp dir, verify directory structure matches Section 12.1 fixture layout
2. All JSON files are valid and round-trip through schema readers
3. `CreateProjectResult` contains correct IDs and paths
4. Git-enabled creation: temp dir contains `.git`, initial commit exists
5. Non-Git creation: no `.git` directory present

**Notes:**
Uses `LocalFileSystem`, `DeterministicUUIDProvider`, `FixedClock`, and `MockGitProvider` (or `SystemGitProvider` for integration) from T-0004.

---

## T-0007: Open/Resume

**Status:** 🔵 Backlog
**Component:** ScriviCore (C++ backend)
**Priority:** High
**Epic:** EP-002: ScriviCore Services
**Date Requested:** 2026-05-19
**Date Implemented:** —
**Date Verified:** —
**Sprint Assigned:** Not Assigned

**Rationale:**
Writers must be able to return to their project and resume exactly where they left off. This is the core daily-use workflow.

**Current Behavior:**
No open/resume logic exists.

**Desired Behavior:**
`ScriviCore::openProject()` opens a `.scrivi` project, restores workspace state, and returns editor-ready Markdown for the last active scene (or first scene as fallback).

**Requirements:**
1. `ProjectOpener` validates and opens the project package
2. `ManuscriptOrderResolver` resolves the correct manuscript/chapter/scene order
3. `SceneReader` reads the active scene's Markdown content
4. `WorkspaceStateService` reads app-local workspace state
5. Falls back to first scene if no workspace state exists
6. Returns `OpenProjectResult` with `OpenMode`, active scene Markdown, restored selection and scroll

**Design Approach:**
Implement in `ScriviCore/src/project_package/` and `ScriviCore/src/manuscript/`. `ProjectOpener` calls `ProjectValidator` to detect repair issues before opening. `OpenMode` reflects the project health state.

**Components Affected:**
- ScriviCore/src/project_package/: ProjectOpener.hpp/cpp, ProjectValidator.hpp/cpp
- ScriviCore/src/manuscript/: SceneReader.hpp/cpp, ManuscriptOrderResolver.hpp/cpp
- ScriviCore/src/workspace/: WorkspaceStateService.hpp/cpp (read path)
- ScriviCore/tests/integration/OpenProjectTests.cpp

**Implementation Details:**
*To be filled in during implementation.*

**Test Steps:**
1. Open project created by T-0006 — returns `OpenMode::normalEdit`, correct Markdown
2. Open with saved workspace state — restores last scene, selection, and scroll
3. Open with no workspace state — falls back to first scene
4. Open `missing-scene-md` fixture — returns `OpenMode::repairRequired`, repairIssues populated
5. Open `corrupt-scene-meta` fixture — returns repairIssues with `corruptMetadata`

**Notes:**
Uses fixtures from Section 12 for error-path tests. Fixtures must be checked in under `ScriviCore/tests/fixtures/`.

---

## T-0008: Save Scene

**Status:** 🔵 Backlog
**Component:** ScriviCore (C++ backend)
**Priority:** High
**Epic:** EP-002: ScriviCore Services
**Date Requested:** 2026-05-19
**Date Implemented:** —
**Date Verified:** —
**Sprint Assigned:** Not Assigned

**Rationale:**
Saving is the most frequent operation a writer performs. It must be atomic, correct, and restore cursor state after reopen.

**Current Behavior:**
No save logic exists.

**Desired Behavior:**
`ScriviCore::saveScene()` atomically writes scene Markdown, updates scene metadata (modifiedAt, modifiedBy, text stats), and updates workspace state (cursor, scroll). After save + reopen, cursor position is restored.

**Requirements:**
1. `SceneWriter` atomically writes Markdown to scene content path
2. Scene metadata `modifiedAt` and `modifiedBy` updated
3. `TextStats` (word count, character count) recalculated and stored in metadata
4. `WorkspaceStateService` write path updates app-local workspace state
5. `SaveSceneResult` reports whether save occurred, metadata updated, and current stats
6. If `previouslyLoadedContentHash` is provided and content is unchanged, skip write (idempotent)

**Design Approach:**
Implement `SceneWriter` in `ScriviCore/src/manuscript/`. WorkspaceStateService write path in `ScriviCore/src/workspace/`. AtomicWrite utility from T-0003 used for all file writes.

**Components Affected:**
- ScriviCore/src/manuscript/: SceneWriter.hpp/cpp
- ScriviCore/src/workspace/: WorkspaceStateService.hpp/cpp (write path)
- ScriviCore/tests/integration/SaveSceneTests.cpp

**Implementation Details:**
*To be filled in during implementation.*

**Test Steps:**
1. Save Markdown → read back → content identical
2. Metadata `modifiedAt` updated after save
3. Word count in metadata matches actual word count
4. Open after save restores cursor selection and scroll position
5. Save with unchanged content hash → `SaveSceneResult.saved = false`

**Notes:**
AtomicWrite (write to temp, then rename) ensures no partial writes on crash. See T-0003.

---

## T-0009: External Change Scan

**Status:** 🔵 Backlog
**Component:** ScriviCore (C++ backend)
**Priority:** High
**Epic:** EP-002: ScriviCore Services
**Date Requested:** 2026-05-19
**Date Implemented:** —
**Date Verified:** —
**Sprint Assigned:** Not Assigned

**Rationale:**
Writers may edit files in the `.scrivi` package outside of Scrivi (e.g., in a text editor or via Git merge). Scrivi must detect these changes and surface repair options rather than silently overwriting or corrupting data.

**Current Behavior:**
No external change detection exists.

**Desired Behavior:**
`ScriviCore::scanForExternalChanges()` detects missing content, missing metadata, corrupt metadata, and unregistered Markdown files. Returns a list of `RepairIssue` items with suggested actions.

**Requirements:**
1. `ExternalChangeScanner` walks the project package and compares against registered metadata
2. `RepairClassifier` categorizes each detected issue into a `RepairCategory`
3. Detect: missing `.md` (missingContent), missing `.meta.json` (missingMetadata), corrupt `.meta.json` (corruptMetadata), `.md` file with no matching metadata entry (unregisteredManuscriptFile)
4. Optionally check Git status if `includeGitStatus = true`
5. Returns `ExternalChangeScanResult` with all issues

**Design Approach:**
Implement in `ScriviCore/src/repair/`. Scanner uses `FileSystem` service to walk directory tree. Classifier uses `RepairIssue.hpp` categories from T-0002.

**Components Affected:**
- ScriviCore/src/repair/: ExternalChangeScanner.hpp/cpp, RepairClassifier.hpp/cpp
- ScriviCore/tests/integration/ExternalChangeTests.cpp
- ScriviCore/tests/fixtures/: missing-scene-md, missing-scene-meta, corrupt-scene-meta, external-edit

**Implementation Details:**
*To be filled in during implementation.*

**Test Steps:**
1. Scan `minimal-valid` fixture — zero RepairIssues
2. Scan `missing-scene-md` fixture — one issue, category `missingContent`
3. Scan `missing-scene-meta` fixture — one issue, category `missingMetadata`
4. Scan `corrupt-scene-meta` fixture — one issue, category `corruptMetadata`
5. Scan `external-edit` fixture — issue category `safeExternalEdit` or `indexesDirty = true`

**Notes:**
All four error fixtures from Section 12 are exercised here. This task depends on fixtures being checked in (initially created in T-0007).

---

## T-0010: Git Snapshots

**Status:** 🔵 Backlog
**Component:** ScriviCore (C++ backend)
**Priority:** High
**Epic:** EP-002: ScriviCore Services
**Date Requested:** 2026-05-19
**Date Implemented:** —
**Date Verified:** —
**Sprint Assigned:** Not Assigned

**Rationale:**
Git-backed snapshots give writers a safety net and version history without requiring them to understand Git. This is an opt-in feature that must not affect projects where it is disabled.

**Current Behavior:**
No Git snapshot logic exists.

**Desired Behavior:**
`ScriviCore::enableGitSnapshots()` initializes a local Git repo in the project and creates an initial snapshot. `ScriviCore::createSnapshot()` creates a named snapshot. Projects without Git enabled are unaffected.

**Requirements:**
1. `SystemGitProvider` wraps `git` command-line tool via `Process` utility
2. `SnapshotService` implements enable, create snapshot, and metadata write
3. `.gitignore` generator produces the user project `.gitignore` from Section 15
4. `enableGitSnapshots()`: init repo, write `.gitignore`, stage all, initial commit
5. `createSnapshot()`: stage all, commit with label and author
6. `SnapshotMetadataJson` (from T-0005) written after each snapshot
7. Normal project (no Git) must work without `GitProvider` being invoked

**Design Approach:**
`SystemGitProvider` in `ScriviCore/src/git/` uses the `Process` utility to invoke `git`. `MockGitProvider` (from T-0004) used in unit tests. Integration tests use `SystemGitProvider` only when system Git is available; tests must skip gracefully otherwise.

**Components Affected:**
- ScriviCore/src/git/: SystemGitProvider.hpp/cpp, SnapshotService.hpp/cpp
- ScriviCore/src/util/: Process.hpp/cpp
- ScriviCore/tests/integration/GitSnapshotTests.cpp

**Implementation Details:**
*To be filled in during implementation.*

**Test Steps:**
1. Enable Git on a project in temp dir — `.git` directory created, initial commit exists
2. Create named snapshot — new commit with correct message and author
3. `SnapshotMetadataJson` written and readable
4. Normal (non-Git) project creation and open — `GitProvider` never called
5. Git integration tests skip gracefully if `git` is not in PATH

**Notes:**
`Process` utility wraps `std::system` or `popen`-equivalent. Must handle Git unavailability cleanly. See Section 13.2–13.3 for MockGitProvider vs SystemGitProvider test policy.

---

## T-0011: Swift Interop Prototype

**Status:** 🔵 Backlog
**Component:** ScriviCore (C++ backend) / Apple platform wrapper
**Priority:** High
**Epic:** EP-002: ScriviCore Services
**Date Requested:** 2026-05-19
**Date Implemented:** —
**Date Verified:** —
**Sprint Assigned:** Not Assigned

**Rationale:**
The C++ core has no value if the Apple platform cannot call it. A minimal Swift wrapper that exercises `createProject()`, `openProject()`, and `saveScene()` proves the interop boundary works and establishes the pattern for the full Apple app shell.

**Current Behavior:**
No Swift/C++ interop wrapper exists.

**Desired Behavior:**
A minimal Apple-side wrapper can call `createProject()`, `openProject()`, and `saveScene()` through the C++ core. No backend behavior is reimplemented in Swift.

**Requirements:**
1. Swift/C++ direct interop (not Objective-C bridging) per approved approach in Repository Skeleton doc Section 2
2. Swift wrapper exposes `createProject()`, `openProject()`, `saveScene()` to Swift callers
3. Swift wrapper handles `Result<T>` conversion to Swift error/value pattern
4. No ScriviCore backend logic duplicated in Swift
5. Wrapper compiles and links against ScriviCore static library

**Design Approach:**
Use direct Swift/C++ interop. The wrapper lives outside `ScriviCore/` (it is a platform adapter, not part of the core). A minimal test target or playground can exercise the three entry points. The full Apple app shell is out of scope for this task.

**Components Affected:**
- New directory: `apple/` or `platforms/apple/` (wrapper target)
- ScriviCore/include/scrivi/: public headers consumed by Swift
- CMakeLists.txt: may need XCFramework or xcbuild integration for Xcode targets

**Implementation Details:**
*To be filled in during implementation.*

**Test Steps:**
1. Swift code calls `createProject()` — project created on disk, no crash
2. Swift code calls `openProject()` on that project — returns active scene Markdown
3. Swift code calls `saveScene()` — Markdown persists and is readable
4. No backend logic (JSON parsing, path resolution, etc.) exists in Swift layer
5. Build succeeds on macOS 26+ with Xcode 26+

**Notes:**
Swift/C++ interop requires C++17 minimum; C++24 features in headers exposed to Swift must be compatible with the interop bridge. This may require careful header hygiene. Confirm interop toolchain behavior before implementation.

---

*Last Updated: 2026-05-19*
