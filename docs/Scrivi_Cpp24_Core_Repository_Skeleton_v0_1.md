# Scrivi C++24 Core Repository Skeleton v0.1

**Project:** Scrivi  
**Document:** C++24 Core Repository Skeleton  
**Version:** 0.1  
**Status:** Draft / Approved Direction Pending Review  
**Date:** 2026-05-18

---

## 1. Purpose

This document defines the initial repository skeleton for the Scrivi C++24 shared backend core.

It translates the approved backend design into a practical source tree, build targets, dependency approach, test layout, and first implementation milestones.

This document is the bridge between design documentation and the initial codebase.

---

## 2. Approved Context

This repository skeleton follows the approved direction from:

```text
Scrivi Backend Architecture v0.2
Scrivi Project Package Structure v0.1
Scrivi Minimum Schema Set v0.1
Scrivi Backend Behavior Spec v0.1
Scrivi External Change Repair Matrix v0.1
Scrivi Project Creation and Open Flow v0.1
Scrivi Backend MVP Slice v0.1
Scrivi Backend Runtime Trade Study v0.2
Scrivi C++24 Backend Core Plan v0.1
Scrivi C++24 Core API Sketch v0.2
```

Approved implementation assumptions:

```text
Language: C++24
Backend model: shared core library
UI dependency: none
Build system: CMake acceptable
JSON: nlohmann/json
Test framework: Catch2
Result type: project-local Result<T>
Git: GitProvider abstraction; SystemGitProvider first
Secure storage: SecureStore abstraction; MockSecureStore for tests; Apple Keychain first production implementation
Swift boundary: direct Swift/C++ interop
Paths: UTF-8 strings at wrapper/project boundary, std::filesystem::path internally
App-local paths: appSupportRoot passed into MVP request structs
```

---

## 3. Repository Goal

The initial repository should support this MVP loop:

```text
create local identity/persona
create .scrivi project
open .scrivi project
resume last writing surface
save scene Markdown
update metadata
restore workspace state
detect simple external changes
optionally initialize Git-backed snapshots
optionally create Git-backed snapshot
```

The repository must not expand into the full app too early.

---

## 4. Non-Goals for Repository v0.1

Do not include implementation for:

```text
full object graph
characters/locations/items/rules/timelines
comments
assets metadata
SQLite
full-text search
relationship graph
exports
remote Git
GitHub/GitLab integration
branch UI
merge UI
real-time collaboration
identity transfer
QR transfer
project encryption
signature verification
group personas
SwiftUI app shell
Android app shell
Windows/Linux UI shell
```

The repository skeleton may include future folders only where needed to keep boundaries clear, but empty speculative subsystems should be avoided.

---

## 5. Recommended Top-Level Layout

```text
Scrivi/
  README.md
  LICENSE
  CMakeLists.txt
  CMakePresets.json

  docs/
    architecture/
    backend/
    decisions/

  ScriviCore/
    CMakeLists.txt

    include/
      scrivi/

    src/

    tests/
      CMakeLists.txt
      fixtures/
      unit/
      integration/

  third_party/
    README.md

  tools/
    README.md

  .gitignore
```

### Notes

- `ScriviCore/` is the backend core.
- `docs/` contains approved design documents.
- `third_party/` documents dependency strategy but should not become a dumping ground.
- `tools/` is for future developer scripts; it can remain almost empty at first.
- UI shells are not part of this skeleton yet.

---

## 6. `docs/` Layout

Recommended documentation layout:

```text
docs/
  architecture/
    Scrivi_Backend_Architecture_v0_2.md
    Scrivi_Project_Package_Structure_v0_1.md
    Scrivi_Minimum_Schema_Set_v0_1.md
    Scrivi_Backend_Behavior_Spec_v0_1.md
    Scrivi_External_Change_Repair_Matrix_v0_1.md
    Scrivi_Project_Creation_and_Open_Flow_v0_1.md
    Scrivi_Backend_MVP_Slice_v0_1.md
    Scrivi_Backend_Runtime_Trade_Study_v0_2.md
    Scrivi_Cpp24_Backend_Core_Plan_v0_1.md
    Scrivi_Cpp24_Core_API_Sketch_v0_2.md
    Scrivi_Cpp24_Core_Repository_Skeleton_v0_1.md

  decisions/
    README.md
```

### `docs/decisions/`

Later, this can hold short decision records if needed.

Example:

```text
docs/decisions/
  DR-0001-use-cpp24-backend-core.md
  DR-0002-visible-manuscript-metadata.md
  DR-0003-use-system-git-provider-first.md
```

This is optional for v0.1.

---

## 7. `ScriviCore/` Layout

Recommended core layout:

```text
ScriviCore/
  CMakeLists.txt

  include/
    scrivi/
      ScriviCore.hpp
      Result.hpp
      Error.hpp
      IDs.hpp
      Types.hpp
      RepairIssue.hpp
      Services.hpp
      Requests.hpp
      Results.hpp

  src/
    public_api/
      ScriviCore.cpp

    domain/
      IDs.cpp
      Slug.cpp
      TextStats.cpp

    schemas/
      ProjectJson.hpp
      ProjectJson.cpp
      ManuscriptMetaJson.hpp
      ManuscriptMetaJson.cpp
      ChapterMetaJson.hpp
      ChapterMetaJson.cpp
      SceneMetaJson.hpp
      SceneMetaJson.cpp
      ProjectMembersJson.hpp
      ProjectMembersJson.cpp
      ProjectPersonasJson.hpp
      ProjectPersonasJson.cpp
      WorkspaceStateJson.hpp
      WorkspaceStateJson.cpp
      SnapshotMetadataJson.hpp
      SnapshotMetadataJson.cpp

    project_package/
      PackagePaths.hpp
      PackagePaths.cpp
      ProjectCreator.hpp
      ProjectCreator.cpp
      ProjectOpener.hpp
      ProjectOpener.cpp
      ProjectValidator.hpp
      ProjectValidator.cpp

    manuscript/
      SceneReader.hpp
      SceneReader.cpp
      SceneWriter.hpp
      SceneWriter.cpp
      ManuscriptOrderResolver.hpp
      ManuscriptOrderResolver.cpp

    identity/
      IdentityService.hpp
      IdentityService.cpp
      PersonaService.hpp
      PersonaService.cpp

    workspace/
      WorkspaceStateService.hpp
      WorkspaceStateService.cpp

    repair/
      ExternalChangeScanner.hpp
      ExternalChangeScanner.cpp
      RepairClassifier.hpp
      RepairClassifier.cpp

    git/
      GitProvider.hpp
      SystemGitProvider.hpp
      SystemGitProvider.cpp
      MockGitProvider.hpp
      SnapshotService.hpp
      SnapshotService.cpp

    platform/
      FileSystem.hpp
      LocalFileSystem.hpp
      LocalFileSystem.cpp
      SecureStore.hpp
      MockSecureStore.hpp
      MockSecureStore.cpp
      Clock.hpp
      SystemClock.hpp
      SystemClock.cpp
      UUIDProvider.hpp
      UUIDv7Provider.hpp
      UUIDv7Provider.cpp
      Logger.hpp
      NullLogger.hpp

    util/
      Json.hpp
      Json.cpp
      AtomicWrite.hpp
      AtomicWrite.cpp
      PathUtils.hpp
      PathUtils.cpp
      Hash.hpp
      Hash.cpp
      Process.hpp
      Process.cpp

  tests/
    CMakeLists.txt
    fixtures/
    unit/
    integration/
```

### Guardrail

This is a starting skeleton. If implementation shows a file or subsystem is unnecessary for the MVP, do not create it yet.

Minimum code beats pretty empty folders.

---

## 8. Public Headers

### 8.1 `ScriviCore.hpp`

Owns the public facade:

```cpp
#pragma once

#include "scrivi/Result.hpp"
#include "scrivi/Requests.hpp"
#include "scrivi/Results.hpp"
#include "scrivi/Services.hpp"

namespace scrivi {

class ScriviCore {
public:
    explicit ScriviCore(CoreServices services);

    Result<EnsureIdentityResult> ensureLocalIdentity(
        const EnsureIdentityRequest& request);

    Result<CreateProjectResult> createProject(
        const CreateProjectRequest& request);

    Result<OpenProjectResult> openProject(
        const OpenProjectRequest& request);

    Result<SaveSceneResult> saveScene(
        const SaveSceneRequest& request);

    Result<ExternalChangeScanResult> scanForExternalChanges(
        const ExternalChangeScanRequest& request);

    Result<EnableGitResult> enableGitSnapshots(
        const EnableGitRequest& request);

    Result<CreateSnapshotResult> createSnapshot(
        const CreateSnapshotRequest& request);

private:
    CoreServices services_;
};

}
```

---

### 8.2 `Result.hpp`

Project-local `Result<T>`.

May internally use `std::expected` where available.

MVP may begin with a simple implementation.

```cpp
#pragma once

#include "scrivi/Error.hpp"
#include <optional>
#include <utility>

namespace scrivi {

template <typename T>
class Result {
public:
    static Result success(T value) {
        Result result;
        result.value_ = std::move(value);
        return result;
    }

    static Result failure(Error error) {
        Result result;
        result.error_ = std::move(error);
        return result;
    }

    bool ok() const {
        return value_.has_value();
    }

    const T& value() const {
        return *value_;
    }

    T& value() {
        return *value_;
    }

    const Error& error() const {
        return *error_;
    }

private:
    std::optional<T> value_;
    std::optional<Error> error_;
};

template <>
class Result<void> {
public:
    static Result success() {
        return Result(true, {});
    }

    static Result failure(Error error) {
        return Result(false, std::move(error));
    }

    bool ok() const {
        return ok_;
    }

    const Error& error() const {
        return error_;
    }

private:
    Result(bool ok, Error error)
        : ok_(ok), error_(std::move(error)) {}

    bool ok_ = false;
    Error error_;
};

}
```

This is illustrative; actual implementation should include defensive checks or assertions.

---

### 8.3 `Error.hpp`

```cpp
#pragma once

#include <string>

namespace scrivi {

enum class ErrorCode {
    ok,

    invalidArgument,
    unsupportedVersion,

    ioError,
    permissionDenied,
    parseError,
    validationError,

    repairRequired,

    gitUnavailable,
    gitError,

    secureStoreUnavailable,
    secureStoreError,

    identityError,

    internalError
};

struct Error {
    ErrorCode code = ErrorCode::ok;
    std::string message;
    std::string path;
    std::string detail;
};

}
```

---

### 8.4 `IDs.hpp`

```cpp
#pragma once

#include <string>

namespace scrivi {

struct ProjectID { std::string value; };
struct ManuscriptID { std::string value; };
struct ChapterID { std::string value; };
struct SceneID { std::string value; };
struct IdentityID { std::string value; };
struct PersonaID { std::string value; };
struct SnapshotID { std::string value; };
struct CommitID { std::string value; };

}
```

---

### 8.5 `Types.hpp`

Common scalar and domain types.

```cpp
#pragma once

#include "scrivi/IDs.hpp"

#include <cstddef>
#include <optional>
#include <string>
#include <vector>

namespace scrivi {

using Utf8Text = std::string;
using ISO8601Timestamp = std::string;
using Slug = std::string;
using RelativePath = std::string;
using AbsolutePath = std::string;

enum class ManuscriptStatus {
    draft,
    revised,
    final,
    archived
};

enum class PersonaKind {
    individual,
    group
};

enum class ProjectRole {
    owner,
    editor,
    reader
};

enum class MemberStatus {
    active,
    removed
};

enum class OpenMode {
    normalEdit,
    editWithWarnings,
    repairRequired,
    readOnly,
    cannotOpen
};

struct TextSelection {
    std::size_t anchor = 0;
    std::size_t focus = 0;
};

struct ScrollPosition {
    double value = 0.0;
};

struct AuthorshipRef {
    IdentityID identityID;
    PersonaID personaID;
    std::string displayName;
};

struct LastWritingSurface {
    SceneID sceneID;
    RelativePath contentPath;
    TextSelection selection;
    ScrollPosition scroll;
};

struct WorkspaceState {
    ProjectID projectID;
    std::string deviceID;
    IdentityID identityID;
    PersonaID activePersonaID;
    std::optional<LastWritingSurface> lastWritingSurface;
    ISO8601Timestamp lastOpenedAt;
};

struct ProjectSummary {
    ProjectID projectID;
    std::string title;
    Slug slug;
    AbsolutePath rootPath;
    bool gitSnapshotsEnabled = false;
};

struct SceneSummary {
    SceneID sceneID;
    ChapterID chapterID;
    std::string title;
    Slug slug;
    ManuscriptStatus status = ManuscriptStatus::draft;
    RelativePath metadataPath;
    RelativePath contentPath;
};

}
```

---

### 8.6 `RepairIssue.hpp`

```cpp
#pragma once

#include "scrivi/IDs.hpp"

#include <string>
#include <vector>

namespace scrivi {

enum class RepairSeverity {
    info,
    warning,
    blocking
};

enum class RepairCategory {
    none,

    safeExternalEdit,
    unregisteredManuscriptFile,

    missingContent,
    missingMetadata,
    possibleRename,

    orphanMetadata,
    corruptMetadata,
    unsupportedSchema,

    gitStateChanged,
    mergeConflict,

    unknownFile,
    unknownIssue
};

enum class RepairActionKind {
    none,

    reloadExternalVersion,
    keepCurrentVersion,
    saveCurrentVersionAsCopy,

    importAsNewScene,
    attachToExistingScene,
    regenerateMetadata,

    restoreFromSnapshot,
    createEmptyContentFile,
    relinkToFile,

    markMissing,
    removeFromProject,

    moveToInbox,
    ignore,
    deleteAfterConfirmation,

    openReadOnly,
    cancelOpen
};

struct RepairAction {
    RepairActionKind kind = RepairActionKind::none;
    std::string label;
    std::string detail;
};

struct RepairIssue {
    std::string issueID;

    RepairSeverity severity = RepairSeverity::info;
    RepairCategory category = RepairCategory::none;

    std::string title;
    std::string message;

    std::string path;
    std::string relatedPath;

    ProjectID projectID;
    ChapterID chapterID;
    SceneID sceneID;

    std::vector<RepairAction> suggestedActions;
};

}
```

---

### 8.7 `Services.hpp`

```cpp
#pragma once

#include "scrivi/IDs.hpp"
#include "scrivi/Result.hpp"
#include "scrivi/Types.hpp"

#include <cstddef>
#include <string_view>
#include <vector>

namespace scrivi {

using SecretBytes = std::vector<std::byte>;

class Clock {
public:
    virtual ~Clock() = default;
    virtual ISO8601Timestamp nowUTC() = 0;
};

class UUIDProvider {
public:
    virtual ~UUIDProvider() = default;

    virtual ProjectID newProjectID() = 0;
    virtual ManuscriptID newManuscriptID() = 0;
    virtual ChapterID newChapterID() = 0;
    virtual SceneID newSceneID() = 0;

    virtual IdentityID newIdentityID() = 0;
    virtual PersonaID newPersonaID() = 0;

    virtual SnapshotID newSnapshotID() = 0;
};

class FileSystem {
public:
    virtual ~FileSystem() = default;

    virtual Result<bool> exists(const AbsolutePath& path) = 0;
    virtual Result<bool> isDirectory(const AbsolutePath& path) = 0;

    virtual Result<void> createDirectories(const AbsolutePath& path) = 0;

    virtual Result<Utf8Text> readTextFile(const AbsolutePath& path) = 0;
    virtual Result<void> atomicWriteTextFile(
        const AbsolutePath& path,
        std::string_view utf8Text) = 0;

    virtual Result<std::vector<AbsolutePath>> listDirectory(
        const AbsolutePath& path) = 0;

    virtual Result<void> removeFile(const AbsolutePath& path) = 0;
};

class SecureStore {
public:
    virtual ~SecureStore() = default;

    virtual Result<bool> containsSecret(std::string_view key) = 0;
    virtual Result<void> putSecret(std::string_view key, const SecretBytes& value) = 0;
    virtual Result<SecretBytes> getSecret(std::string_view key) = 0;
};

struct GitAuthor {
    std::string name;
    std::string email;
};

struct CommitRequest {
    std::string message;
    GitAuthor author;
};

struct GitStatus {
    bool isRepository = false;
    bool hasUncommittedChanges = false;
    std::vector<RelativePath> changedFiles;
    std::vector<RelativePath> untrackedFiles;
};

class GitProvider {
public:
    virtual ~GitProvider() = default;

    virtual Result<bool> isRepository(const AbsolutePath& rootPath) = 0;
    virtual Result<void> initRepository(const AbsolutePath& rootPath) = 0;
    virtual Result<void> addAll(const AbsolutePath& rootPath) = 0;

    virtual Result<CommitID> commit(
        const AbsolutePath& rootPath,
        const CommitRequest& request) = 0;

    virtual Result<GitStatus> status(const AbsolutePath& rootPath) = 0;
};

enum class LogLevel {
    debug,
    info,
    warning,
    error
};

class Logger {
public:
    virtual ~Logger() = default;
    virtual void log(LogLevel level, std::string_view message) = 0;
};

struct CoreServices {
    FileSystem* fileSystem = nullptr;
    SecureStore* secureStore = nullptr;
    Clock* clock = nullptr;
    UUIDProvider* uuidProvider = nullptr;
    GitProvider* gitProvider = nullptr;
    Logger* logger = nullptr;
};

}
```

---

### 8.8 `Requests.hpp`

```cpp
#pragma once

#include "scrivi/Types.hpp"

#include <optional>
#include <string>

namespace scrivi {

struct EnsureIdentityRequest {
    std::string requestedDisplayName;
    AbsolutePath appSupportRoot;
};

struct CreateProjectRequest {
    AbsolutePath projectRootPath;

    std::string title;
    Slug slug;

    AuthorshipRef author;

    std::string initialChapterTitle = "Chapter 1";
    std::string initialChapterSlug = "chapter-001";

    std::string initialSceneTitle = "Opening Scene";
    std::string initialSceneSlug = "001-opening-scene";

    bool enableGitSnapshots = false;

    AbsolutePath appSupportRoot;
};

struct OpenProjectRequest {
    AbsolutePath projectRootPath;
    AbsolutePath appSupportRoot;

    std::optional<IdentityID> currentIdentityID;
};

struct SaveSceneRequest {
    ProjectID projectID;
    AbsolutePath projectRootPath;
    AbsolutePath appSupportRoot;

    SceneID sceneID;
    RelativePath sceneMetadataPath;
    RelativePath sceneContentPath;

    Utf8Text markdown;

    TextSelection selection;
    ScrollPosition scroll;

    AuthorshipRef author;

    std::optional<std::string> previouslyLoadedContentHash;
};

struct ExternalChangeScanRequest {
    AbsolutePath projectRootPath;
    AbsolutePath appSupportRoot;

    bool includeGitStatus = true;
};

struct EnableGitRequest {
    AbsolutePath projectRootPath;

    AuthorshipRef author;

    std::string initialSnapshotLabel = "Initial project";
};

struct CreateSnapshotRequest {
    AbsolutePath projectRootPath;

    AuthorshipRef author;

    std::string label;
    std::string note;
};

}
```

---

### 8.9 `Results.hpp`

```cpp
#pragma once

#include "scrivi/RepairIssue.hpp"
#include "scrivi/Types.hpp"

#include <optional>
#include <vector>

namespace scrivi {

struct EnsureIdentityResult {
    IdentityID identityID;
    PersonaID defaultPersonaID;

    std::string displayName;

    bool createdNewIdentity = false;
};

struct CreateProjectResult {
    ProjectSummary project;

    ManuscriptID manuscriptID;
    ChapterID firstChapterID;
    SceneID firstSceneID;

    RelativePath firstSceneMetadataPath;
    RelativePath firstSceneContentPath;

    WorkspaceState workspaceState;

    bool gitInitialized = false;
    std::optional<SnapshotID> initialSnapshotID;

    std::vector<RepairIssue> warnings;
};

struct OpenProjectResult {
    OpenMode mode = OpenMode::cannotOpen;

    ProjectSummary project;

    std::optional<WorkspaceState> workspaceState;

    std::optional<SceneSummary> activeScene;
    Utf8Text activeSceneMarkdown;

    TextSelection restoredSelection;
    ScrollPosition restoredScroll;

    std::vector<RepairIssue> repairIssues;
};

struct SaveSceneResult {
    SceneID sceneID;

    bool saved = false;
    bool metadataUpdated = false;
    bool workspaceStateUpdated = false;

    std::size_t wordCount = 0;
    std::size_t characterCount = 0;

    bool hasUnsnapshottedChanges = false;

    std::vector<RepairIssue> repairIssues;
};

struct ExternalChangeScanResult {
    ProjectID projectID;

    std::vector<RepairIssue> repairIssues;

    bool indexesDirty = false;
    bool gitStatusChecked = false;
    bool hasUnsnapshottedChanges = false;
};

struct EnableGitResult {
    bool gitInitialized = false;
    bool alreadyRepository = false;

    SnapshotID initialSnapshotID;
    CommitID initialCommitID;

    std::vector<RepairIssue> warnings;
};

struct CreateSnapshotResult {
    SnapshotID snapshotID;
    CommitID commitID;

    ISO8601Timestamp createdAt;

    bool created = false;
};

}
```

---

## 9. CMake Target Plan

### 9.1 Root `CMakeLists.txt`

Initial concept:

```cmake
cmake_minimum_required(VERSION 3.24)

project(Scrivi LANGUAGES CXX)

option(SCRIVI_BUILD_TESTS "Build ScriviCore tests" ON)

add_subdirectory(ScriviCore)
```

### 9.2 `ScriviCore/CMakeLists.txt`

Initial concept:

```cmake
add_library(ScriviCore STATIC)

target_compile_features(ScriviCore PUBLIC cxx_std_23)

target_include_directories(ScriviCore
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_sources(ScriviCore
    PRIVATE
        src/public_api/ScriviCore.cpp
)

# nlohmann/json integration to be added after dependency strategy is finalized.

if(SCRIVI_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
```

### 9.3 C++24 note

CMake's compile feature support may use `cxx_std_23` until `cxx_std_26` or newer feature gates are broadly available.

For Scrivi, the policy is:

```text
Use C++24-capable compiler/toolchain.
Avoid relying on unstable library features unless guarded.
```

If a C++24 feature is not consistently available, prefer a small project-local abstraction.

---

## 10. Dependency Plan

### 10.1 Approved dependencies

```text
nlohmann/json
Catch2
```

### 10.2 Dependency acquisition strategy

Options:

```text
CMake FetchContent
Git submodules
system packages
vendored release archives
package managers such as vcpkg/conan
```

### 10.3 Recommendation for v0.1

Use **CMake FetchContent** initially.

Reasons:

```text
simple bootstrap
works well for small dependency set
does not require global system packages
keeps CI straightforward
```

### 10.4 Guardrail

Third-party APIs should not leak through public ScriviCore headers.

---

## 11. Catch2 Test Layout

```text
ScriviCore/
  tests/
    CMakeLists.txt

    unit/
      ResultTests.cpp
      IDTests.cpp
      SlugTests.cpp
      PathUtilsTests.cpp
      TextStatsTests.cpp
      JsonSchemaTests.cpp

    integration/
      CreateProjectTests.cpp
      OpenProjectTests.cpp
      SaveSceneTests.cpp
      ExternalChangeTests.cpp
      GitSnapshotTests.cpp

    fixtures/
      minimal-valid/
      missing-scene-md/
      missing-scene-meta/
      corrupt-scene-meta/
      external-edit/
```

### 11.1 `ScriviCore/tests/CMakeLists.txt`

Concept:

```cmake
include(FetchContent)

FetchContent_Declare(
  Catch2
  URL https://github.com/catchorg/Catch2/archive/refs/tags/v3.6.0.tar.gz
)

FetchContent_MakeAvailable(Catch2)

add_executable(ScriviCoreTests
    unit/ResultTests.cpp
    unit/IDTests.cpp
    integration/CreateProjectTests.cpp
)

target_link_libraries(ScriviCoreTests
    PRIVATE
        ScriviCore
        Catch2::Catch2WithMain
)

include(CTest)
include(Catch)
catch_discover_tests(ScriviCoreTests)
```

### 11.2 Test dependency policy

Catch2 is development/test-only.

Production targets must not link Catch2.

---

## 12. Fixture Layout

### 12.1 `minimal-valid`

```text
fixtures/
  minimal-valid/
    MyNovel.scrivi/
      project.json

      manuscript/
        manuscript.meta.json

        chapter-001/
          chapter.meta.json
          001-opening-scene.md
          001-opening-scene.meta.json

      identities/
        project-members.json
        project-personas.json
```

### 12.2 `missing-scene-md`

Same as minimal valid, but:

```text
001-opening-scene.md
```

is missing.

Expected result:

```text
RepairIssue category: missingContent
Severity: blocking or warning depending active scene context
```

### 12.3 `missing-scene-meta`

Same as minimal valid, but:

```text
001-opening-scene.meta.json
```

is missing.

Expected result:

```text
RepairIssue category: missingMetadata
```

### 12.4 `corrupt-scene-meta`

Same as minimal valid, but scene metadata contains invalid JSON.

Expected result:

```text
RepairIssue category: corruptMetadata
```

### 12.5 `external-edit`

Fixture used to simulate changed Markdown content.

Expected result:

```text
safeExternalEdit or index dirty state
```

---

## 13. Mock Services

Tests should use mock or local services.

### 13.1 `MockSecureStore`

Used for tests only.

Must not be compiled into production app targets unless explicitly marked test-only.

### 13.2 `MockGitProvider`

Used for unit tests of snapshot logic without invoking real Git.

### 13.3 `SystemGitProvider`

Used for integration tests only when system Git is available.

Tests should skip Git integration tests gracefully if Git is unavailable.

### 13.4 `FixedClock`

Returns deterministic timestamps.

Useful for stable JSON fixture comparisons.

### 13.5 `DeterministicUUIDProvider`

Returns known IDs.

Useful for predictable fixture generation and golden output tests.

---

## 14. Initial `.gitignore`

For Scrivi development repository:

```gitignore
# Build outputs
/build/
/out/
/cmake-build-*/

# CMake
CMakeCache.txt
CMakeFiles/
cmake_install.cmake
CTestTestfile.cmake

# IDE/editor
.vscode/
.idea/
*.swp
*.swo

# macOS
.DS_Store

# Windows
Thumbs.db

# Test output
ScriviCore/tests/tmp/
ScriviCore/tests/output/

# Generated coverage/profiling
*.gcda
*.gcno
coverage/
```

Note: this is for the **Scrivi source repository**, not for user `.scrivi` projects.

User project `.gitignore` is generated separately by ScriviCore when Git-backed snapshots are enabled.

---

## 15. User Project `.gitignore`

When Scrivi enables Git snapshots inside a writer's project, it should generate a different `.gitignore`.

Initial concept:

```gitignore
# OS noise
.DS_Store
Thumbs.db

# Editor temp files
*.tmp
*.swp
*.swo
~$*

# Defensive cache/index exclusions
.cache/
indexes/
*.sqlite
*.sqlite-shm
*.sqlite-wal
```

Since indexes are app-local by default, these are mostly defensive.

---

## 16. Initial Implementation Milestones

### Milestone 1 — Repository skeleton

Create:

```text
top-level CMakeLists.txt
ScriviCore/CMakeLists.txt
public headers
empty ScriviCore.cpp
Catch2 test target
README.md
.gitignore
```

Success:

```text
project configures
project builds
empty test executable runs
```

---

### Milestone 2 — Core value types

Implement:

```text
IDs.hpp
Types.hpp
Error.hpp
Result.hpp
RepairIssue.hpp
Requests.hpp
Results.hpp
Services.hpp
```

Success:

```text
headers compile
basic Result tests pass
no UI dependencies
```

---

### Milestone 3 — Utility foundation

Implement:

```text
PathUtils
Slug
TextStats
Json wrapper
AtomicWrite
Hash
```

Success:

```text
unit tests pass
UTF-8 strings preserved
conservative slug generation works
```

---

### Milestone 4 — Mock services

Implement:

```text
FixedClock
DeterministicUUIDProvider
MockSecureStore
MockGitProvider
NullLogger
LocalFileSystem
```

Success:

```text
tests can run deterministically
project creation tests can use temp directories
```

---

### Milestone 5 — Schema read/write

Implement schema readers/writers for:

```text
project.json
manuscript.meta.json
chapter.meta.json
scene.meta.json
project-members.json
project-personas.json
workspace-state.json
```

Success:

```text
write minimal project metadata
read it back
validate required fields
reject corrupt JSON
```

---

### Milestone 6 — Project creation

Implement:

```text
ProjectCreator
ScriviCore::createProject()
```

Success:

```text
minimum .scrivi project created
first scene Markdown exists
paired metadata exists
workspace-state written app-locally
```

---

### Milestone 7 — Open/resume

Implement:

```text
ProjectOpener
ManuscriptOrderResolver
SceneReader
WorkspaceStateService read path
ScriviCore::openProject()
```

Success:

```text
opens last workspace scene
falls back to first scene
returns editor-ready Markdown
```

---

### Milestone 8 — Save scene

Implement:

```text
SceneWriter
metadata modifiedAt/modifiedBy update
workspace-state update
text stats update
ScriviCore::saveScene()
```

Success:

```text
text persists
metadata updates
cursor restored after reopen
```

---

### Milestone 9 — External change scan

Implement:

```text
ExternalChangeScanner
RepairClassifier
ScriviCore::scanForExternalChanges()
```

Success:

```text
detect missing .md
detect missing .meta.json
detect corrupt metadata
detect unregistered .md
```

---

### Milestone 10 — Git snapshots

Implement:

```text
SystemGitProvider
SnapshotService
.gitignore generator
enableGitSnapshots()
createSnapshot()
```

Success:

```text
Git-enabled project initializes local repo
initial snapshot created
named snapshot created
snapshot metadata written
normal project works without Git
```

---

### Milestone 11 — Swift interop prototype

Create a minimal Apple-side wrapper that can call:

```text
createProject()
openProject()
saveScene()
```

Success:

```text
Swift can create/open/save through C++ core
no backend behavior reimplemented in Swift
```

---

## 17. CI / Build Matrix Proposal

Initial CI should build and test:

```text
macOS
Ubuntu Linux
Windows
```

Minimum checks:

```text
configure CMake
build ScriviCore
run Catch2 tests
```

Later:

```text
Apple wrapper build
Android NDK build
sanitizer builds
coverage
static analysis
```

---

## 18. README.md Outline

Initial repository README should include:

```text
Scrivi overview
backend core status
build requirements
configure/build/test instructions
documentation links
MVP scope
non-goals
```

Example commands:

```bash
cmake -S . -B build -DSCRIVI_BUILD_TESTS=ON
cmake --build build
ctest --test-dir build
```

---

## 19. Success Criteria

The repository skeleton is successful if:

1. The source tree clearly separates backend core, tests, docs, and future wrappers.
2. ScriviCore can build as a standalone library.
3. Tests can build and run without UI.
4. Catch2 is test-only.
5. nlohmann/json is hidden behind schema/json utilities.
6. Public headers do not expose UI framework types.
7. Git is behind `GitProvider`.
8. Secure storage is behind `SecureStore`.
9. App-local paths are supplied by request structs in MVP.
10. The repo can proceed directly into Milestone 1 implementation.

---

## 20. Open Questions

Before coding, confirm:

1. CMake is acceptable for the initial repository despite Automake preference.
2. Dependency acquisition via CMake FetchContent is acceptable.
3. Initial source repository name.
4. Whether the development repo should start private or public on GitHub.
5. Whether CI should be added immediately or after Milestone 1.

---

## 21. Recommended Next Step

After this skeleton is approved:

```text
Create initial repository skeleton
Add CMake build
Add public headers
Add Catch2 test target
Add first Result<T> tests
```

This is the first actual coding step.
