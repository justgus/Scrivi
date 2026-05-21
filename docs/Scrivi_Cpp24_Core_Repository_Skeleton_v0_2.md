# Scrivi C++24 Core Repository Skeleton v0.2

**Project:** Scrivi  
**Document:** C++24 Core Repository Skeleton  
**Version:** 0.2  
**Status:** Approved Direction  
**Date:** 2026-05-21  
**Supersedes:** `Scrivi_Cpp24_Core_Repository_Skeleton_v0_1.md`

---

## 1. Purpose

This document defines the repository skeleton for the Scrivi C++24 shared backend core and its Apple platform wrapper.

It translates the approved architecture (v0.3) into a practical source tree, build targets, dependency approach, test layout, and platform wrapper layout.

The primary change from v0.1 is the formalization of:

1. The `ScriviCoreAdapter` layer as a permanent, load-bearing C++ shim between ScriviCore and Swift.
2. The `platforms/apple/` SPM package as the canonical Apple platform wrapper location.
3. The JSON-over-`std::string` boundary protocol as the permanent cross-language mechanism.

---

## 2. Approved Context

This skeleton follows the approved direction from:

```text
Scrivi Architecture v0.3
Scrivi Apple Wrapper Design v0.1
Scrivi Swift Interop Trade Study v0.1
Scrivi Project Package Structure v0.1
Scrivi Minimum Schema Set v0.1
Scrivi Backend Behavior Spec v0.2
Scrivi External Change Repair Matrix v0.2
Scrivi Project Creation and Open Flow v0.2
Scrivi Backend MVP Slice v0.1
Scrivi C++24 Core API Sketch v0.3
```

Approved implementation assumptions:

```text
Language: C++23 (targeting C++24 features as compiler support matures)
Backend model: shared C++ static library (ScriviCore)
Boundary protocol: JSON-over-std::string at all cross-language boundaries
Adapter layer: ScriviCoreAdapter — permanent, not temporary
Apple wrapper: SPM package at platforms/apple/
UI dependency: none in ScriviCore or ScriviCoreAdapter
Build system: CMake for ScriviCore; SPM for Apple wrapper
JSON: nlohmann/json (hidden behind schema wrappers; also used in adapter serialization)
Test framework: Catch2 (C++), Swift Testing (Apple wrapper tests)
Result type: project-local Result<T>
Git: GitProvider abstraction; SystemGitProvider first
Secure storage: SecureStore abstraction; PrototypeSecureStore (MVP); KeychainSecureStore (production)
Paths: UTF-8 strings at wrapper/project boundary; std::filesystem::path internally
App-local paths: appSupportRoot passed into MVP request structs
Pass-by-value: all data transmission between layers produces an independent copy in the receiver
```

---

## 3. Repository Goal

The repository supports this MVP loop:

```text
create local identity/persona
create .scrivi project
open .scrivi project
resume last writing surface (Markdown by value across C++/Swift boundary)
save scene Markdown
update metadata
restore workspace state
detect simple external changes
optionally initialize Git-backed snapshots
optionally create Git-backed snapshot
```

The Apple wrapper can call all of the above through `ScriviEngine.swift` → `ScriviCoreAdapter` → `ScriviCore`.

---

## 4. Non-Goals for v0.2

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
KeychainSecureStore (deferred to post-MVP sprint)
```

---

## 5. Top-Level Layout

```text
Scrivi/
  README.md
  LICENSE
  CMakeLists.txt
  CMakePresets.json

  docs/
    Epics/
    Sprints/
    Tasks/
    Issues/
    (design documents — all versions)

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
      mocks/

  platforms/
    apple/
      Package.swift
      Sources/
        ScriviCoreAdapter/
        Scrivi/
      Tests/
        ScriviInteropTests/

  .gitignore
```

### Notes

- `ScriviCore/` is the C++23 backend core static library. CMake-managed.
- `platforms/apple/` is the Apple platform SPM package. Contains the C++ adapter shim and Swift engine.
- `docs/` contains all design documents, agile tracking, and issues.
- No `third_party/` directory — dependencies are acquired via CMake FetchContent.
- No `tools/` directory until a real script is needed.

---

## 6. `ScriviCore/` Layout

```text
ScriviCore/
  CMakeLists.txt

  include/
    scrivi/
      ScriviCore.hpp        ← public facade class
      Result.hpp            ← Result<T> type
      Error.hpp             ← ErrorCode + Error struct
      IDs.hpp               ← typed ID wrappers
      Types.hpp             ← common domain types
      RepairIssue.hpp       ← repair issue model
      Services.hpp          ← service interfaces + CoreServices
      Requests.hpp          ← all request structs
      Results.hpp           ← all result structs

  src/
    public_api/
      ScriviCore.cpp        ← facade implementation

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
      SystemGitProvider.hpp
      SystemGitProvider.cpp
      SnapshotService.hpp
      SnapshotService.cpp

    platform/
      LocalFileSystem.hpp
      LocalFileSystem.cpp
      SystemUUIDProvider.hpp
      SystemUUIDProvider.cpp
      NullLogger.hpp
      AppSupportLayout.hpp
      AppSupportLayout.cpp

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

    mocks/
      MockSecureStore.hpp
      MockSecureStore.cpp
      MockGitProvider.hpp
      MockGitProvider.cpp
      FixedClock.hpp
      DeterministicUUIDProvider.hpp

    fixtures/
      minimal-valid/
      missing-scene-md/
      missing-scene-meta/
      corrupt-scene-meta/
      external-edit/

    unit/
      ResultTests.cpp
      IDTests.cpp
      SlugTests.cpp
      PathUtilsTests.cpp
      TextStatsTests.cpp
      JsonSchemaTests.cpp

    integration/
      IdentityTests.cpp
      AppSupportLayoutTests.cpp
      CreateProjectTests.cpp
      OpenProjectTests.cpp
      SaveSceneTests.cpp
      ExternalChangeTests.cpp
      GitSnapshotTests.cpp
```

### Guardrail

Service interface headers (`FileSystem`, `Clock`, `UUIDProvider`, `SecureStore`, `GitProvider`, `Logger`) are defined in `include/scrivi/Services.hpp` — the public header. Concrete implementations (`LocalFileSystem`, `SystemUUIDProvider`, etc.) live in `src/platform/` and are not public API.

Mock implementations used only in tests live in `tests/mocks/` and must not be compiled into production targets.

---

## 7. `platforms/apple/` Layout

```text
platforms/
  apple/
    Package.swift

    Sources/
      ScriviCoreAdapter/
        ScriviCoreAdapter.hpp   ← adapter class declaration (import_reference)
        ScriviCoreAdapter.cpp   ← adapter implementation + Impl struct + serialization
        module.modulemap        ← exposes ScriviCoreAdapter to Swift

      Scrivi/
        ScriviEngine.swift      ← Swift entry point; type conversion + envelope decoding
        ScriviError.swift       ← ScriviError type thrown on ok=false

    Tests/
      ScriviInteropTests/
        ScriviInteropTests.swift
```

### Notes

- `ScriviCoreAdapter/` is a C++ SPM target. It links `libScriviCore.a` and includes the ScriviCore public headers.
- `Scrivi/` is the Swift target. It depends on `ScriviCoreAdapter`. It does not link ScriviCore directly.
- `ScriviInteropTests/` uses Swift Testing (`import Testing`).
- No Objective-C files. No `.h` bridging headers. No `@objc`.

---

## 8. Public Headers

### 8.1 `ScriviCore.hpp`

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

    Result<EnsureIdentityResult>     ensureLocalIdentity(const EnsureIdentityRequest&);
    Result<CreateProjectResult>      createProject(const CreateProjectRequest&);
    Result<OpenProjectResult>        openProject(const OpenProjectRequest&);
    Result<SaveSceneResult>          saveScene(const SaveSceneRequest&);
    Result<ExternalChangeScanResult> scanForExternalChanges(const ExternalChangeScanRequest&);
    Result<EnableGitResult>          enableGitSnapshots(const EnableGitRequest&);
    Result<CreateSnapshotResult>     createSnapshot(const CreateSnapshotRequest&);

private:
    CoreServices services_;
};

}
```

---

### 8.2 `Result.hpp`

Project-local `Result<T>`. May internally use `std::expected` where available.

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
        Result r;
        r.value_ = std::move(value);
        return r;
    }

    static Result failure(Error error) {
        Result r;
        r.error_ = std::move(error);
        return r;
    }

    bool ok() const { return value_.has_value(); }

    const T& value() const { return *value_; }
    T&       value()       { return *value_; }

    const Error& error() const { return *error_; }

private:
    std::optional<T>     value_;
    std::optional<Error> error_;
};

template <>
class Result<void> {
public:
    static Result success()            { return Result(true,  {}); }
    static Result failure(Error error) { return Result(false, std::move(error)); }

    bool         ok()    const { return ok_; }
    const Error& error() const { return error_; }

private:
    Result(bool ok, Error error) : ok_(ok), error_(std::move(error)) {}
    bool  ok_ = false;
    Error error_;
};

}
```

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
    ErrorCode   code    = ErrorCode::ok;
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

struct ProjectID    { std::string value; };
struct ManuscriptID { std::string value; };
struct ChapterID    { std::string value; };
struct SceneID      { std::string value; };
struct IdentityID   { std::string value; };
struct PersonaID    { std::string value; };
struct SnapshotID   { std::string value; };
struct CommitID     { std::string value; };

}
```

---

### 8.5 `Types.hpp`

```cpp
#pragma once

#include "scrivi/IDs.hpp"
#include <cstddef>
#include <optional>
#include <string>
#include <vector>

namespace scrivi {

using Utf8Text         = std::string;
using ISO8601Timestamp = std::string;
using Slug             = std::string;
using RelativePath     = std::string;
using AbsolutePath     = std::string;

enum class ManuscriptStatus { draft, revised, final, archived };
enum class PersonaKind      { individual, group };
enum class ProjectRole      { owner, editor, reader };
enum class MemberStatus     { active, removed };

enum class OpenMode {
    normalEdit,
    editWithWarnings,
    repairRequired,
    readOnly,
    cannotOpen
};

struct TextSelection  { std::size_t anchor = 0; std::size_t focus = 0; };
struct ScrollPosition { double value = 0.0; };

struct AuthorshipRef {
    IdentityID  identityID;
    PersonaID   personaID;
    std::string displayName;
};

struct LastWritingSurface {
    SceneID       sceneID;
    RelativePath  contentPath;
    TextSelection selection;
    ScrollPosition scroll;
};

struct WorkspaceState {
    ProjectID   projectID;
    std::string deviceID;
    IdentityID  identityID;
    PersonaID   activePersonaID;
    std::optional<LastWritingSurface> lastWritingSurface;
    ISO8601Timestamp lastOpenedAt;
};

struct ProjectSummary {
    ProjectID    projectID;
    std::string  title;
    Slug         slug;
    AbsolutePath rootPath;
    bool         gitSnapshotsEnabled = false;
};

struct SceneSummary {
    SceneID          sceneID;
    ChapterID        chapterID;
    std::string      title;
    Slug             slug;
    ManuscriptStatus status = ManuscriptStatus::draft;
    RelativePath     metadataPath;
    RelativePath     contentPath;
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
    info, warning, blocking
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
    std::string      label;
    std::string      detail;
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
    SceneID   sceneID;

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
    virtual ProjectID    newProjectID()    = 0;
    virtual ManuscriptID newManuscriptID() = 0;
    virtual ChapterID    newChapterID()    = 0;
    virtual SceneID      newSceneID()      = 0;
    virtual IdentityID   newIdentityID()   = 0;
    virtual PersonaID    newPersonaID()    = 0;
    virtual SnapshotID   newSnapshotID()   = 0;
};

class FileSystem {
public:
    virtual ~FileSystem() = default;
    virtual Result<bool>     exists(const AbsolutePath& path)          = 0;
    virtual Result<bool>     isDirectory(const AbsolutePath& path)     = 0;
    virtual Result<void>     createDirectories(const AbsolutePath& path) = 0;
    virtual Result<Utf8Text> readTextFile(const AbsolutePath& path)    = 0;
    virtual Result<void>     atomicWriteTextFile(const AbsolutePath& path,
                                                  std::string_view utf8Text) = 0;
    virtual Result<std::vector<AbsolutePath>> listDirectory(const AbsolutePath& path) = 0;
    virtual Result<void>     removeFile(const AbsolutePath& path)      = 0;
};

class SecureStore {
public:
    virtual ~SecureStore() = default;
    virtual Result<bool>        containsSecret(std::string_view key)                   = 0;
    virtual Result<void>        putSecret(std::string_view key, const SecretBytes& value) = 0;
    virtual Result<SecretBytes> getSecret(std::string_view key)                        = 0;
};

struct GitAuthor    { std::string name; std::string email; };
struct CommitRequest { std::string message; GitAuthor author; };

struct GitStatus {
    bool isRepository            = false;
    bool hasUncommittedChanges   = false;
    std::vector<RelativePath> changedFiles;
    std::vector<RelativePath> untrackedFiles;
};

class GitProvider {
public:
    virtual ~GitProvider() = default;
    virtual Result<bool>     isRepository(const AbsolutePath& rootPath)                         = 0;
    virtual Result<void>     initRepository(const AbsolutePath& rootPath)                       = 0;
    virtual Result<void>     addAll(const AbsolutePath& rootPath)                               = 0;
    virtual Result<CommitID> commit(const AbsolutePath& rootPath, const CommitRequest& request) = 0;
    virtual Result<GitStatus> status(const AbsolutePath& rootPath)                              = 0;
};

enum class LogLevel { debug, info, warning, error };

class Logger {
public:
    virtual ~Logger() = default;
    virtual void log(LogLevel level, std::string_view message) = 0;
};

struct CoreServices {
    FileSystem*    fileSystem    = nullptr;
    SecureStore*   secureStore   = nullptr;
    Clock*         clock         = nullptr;
    UUIDProvider*  uuidProvider  = nullptr;
    GitProvider*   gitProvider   = nullptr;
    Logger*        logger        = nullptr;
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
    std::string  requestedDisplayName;
    AbsolutePath appSupportRoot;
};

struct CreateProjectRequest {
    AbsolutePath projectRootPath;
    std::string  title;
    Slug         slug;
    AuthorshipRef author;
    std::string  initialChapterTitle = "Chapter 1";
    std::string  initialChapterSlug  = "chapter-001";
    std::string  initialSceneTitle   = "Opening Scene";
    std::string  initialSceneSlug    = "001-opening-scene";
    bool         enableGitSnapshots  = false;
    AbsolutePath appSupportRoot;
};

struct OpenProjectRequest {
    AbsolutePath              projectRootPath;
    AbsolutePath              appSupportRoot;
    std::optional<IdentityID> currentIdentityID;
};

struct SaveSceneRequest {
    ProjectID    projectID;
    AbsolutePath projectRootPath;
    AbsolutePath appSupportRoot;

    SceneID      sceneID;
    RelativePath sceneMetadataPath;
    RelativePath sceneContentPath;

    Utf8Text     markdown;
    TextSelection  selection;
    ScrollPosition scroll;

    AuthorshipRef author;
    std::optional<std::string> previouslyLoadedContentHash;
};

struct ExternalChangeScanRequest {
    AbsolutePath projectRootPath;
    AbsolutePath appSupportRoot;
    bool         includeGitStatus = true;
};

struct EnableGitRequest {
    AbsolutePath  projectRootPath;
    AuthorshipRef author;
    std::string   initialSnapshotLabel = "Initial project";
};

struct CreateSnapshotRequest {
    AbsolutePath  projectRootPath;
    AuthorshipRef author;
    std::string   label;
    std::string   note;
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
    IdentityID  identityID;
    PersonaID   defaultPersonaID;
    std::string displayName;
    bool        createdNewIdentity = false;
};

struct CreateProjectResult {
    ProjectSummary project;
    ManuscriptID   manuscriptID;
    ChapterID      firstChapterID;
    SceneID        firstSceneID;
    RelativePath   firstSceneMetadataPath;
    RelativePath   firstSceneContentPath;
    WorkspaceState workspaceState;
    bool           gitInitialized    = false;
    std::optional<SnapshotID> initialSnapshotID;
    std::vector<RepairIssue> warnings;
};

struct OpenProjectResult {
    OpenMode    mode = OpenMode::cannotOpen;
    ProjectSummary project;
    std::optional<WorkspaceState> workspaceState;
    std::optional<SceneSummary>   activeScene;
    Utf8Text    activeSceneMarkdown;
    TextSelection   restoredSelection;
    ScrollPosition  restoredScroll;
    std::vector<RepairIssue> repairIssues;
};

struct SaveSceneResult {
    SceneID     sceneID;
    bool        saved                 = false;
    bool        metadataUpdated       = false;
    bool        workspaceStateUpdated = false;
    std::size_t wordCount             = 0;
    std::size_t characterCount        = 0;
    bool        hasUnsnapshottedChanges = false;
    std::vector<RepairIssue> repairIssues;
};

struct ExternalChangeScanResult {
    ProjectID projectID;
    std::vector<RepairIssue> repairIssues;
    bool indexesDirty           = false;
    bool gitStatusChecked       = false;
    bool hasUnsnapshottedChanges = false;
};

struct EnableGitResult {
    bool       gitInitialized    = false;
    bool       alreadyRepository = false;
    SnapshotID initialSnapshotID;
    CommitID   initialCommitID;
    std::vector<RepairIssue> warnings;
};

struct CreateSnapshotResult {
    SnapshotID       snapshotID;
    CommitID         commitID;
    ISO8601Timestamp createdAt;
    bool             created = false;
};

}
```

---

## 9. `ScriviCoreAdapter` Design

### 9.1 Purpose

`ScriviCoreAdapter` is the permanent C++ shim between ScriviCore and all platform wrappers. On Apple platforms it is imported into Swift via Swift/C++ direct interop.

### 9.2 Contract

- **Inputs:** `const char*` parameters only. No ScriviCore types cross the boundary.
- **Outputs:** `std::string` JSON values, returned by value. No raw pointers as return values.
- **No ScriviCore types are in the adapter header.** The header depends only on `<string>`, `<memory>`, and `<atomic>`.
- **Service wiring** is owned by the adapter's `Impl`. Platform-specific services are instantiated inside `Impl`, not injected from Swift.

### 9.3 Header

```cpp
// ScriviCoreAdapter.hpp
#pragma once
#include <atomic>
#include <memory>
#include <string>

namespace scrivi::apple {
    class ScriviAdapter;
}

void scriviAdapterRetain(scrivi::apple::ScriviAdapter* p) noexcept;
void scriviAdapterRelease(scrivi::apple::ScriviAdapter* p) noexcept;

namespace scrivi::apple {

class __attribute__((swift_attr("import_reference")))
      __attribute__((swift_attr("retain:scriviAdapterRetain")))
      __attribute__((swift_attr("release:scriviAdapterRelease")))
ScriviAdapter {
public:
    static ScriviAdapter* create();

    void retain()  noexcept;
    void release() noexcept;

    std::string ensureLocalIdentity(
        const char* requestedDisplayName,
        const char* appSupportRoot);

    std::string createProject(
        const char* projectRootPath,
        const char* appSupportRoot,
        const char* title,
        const char* slug,
        const char* identityID,
        const char* personaID,
        const char* displayName);

    std::string openProject(
        const char* projectRootPath,
        const char* appSupportRoot,
        const char* identityID);

    std::string saveScene(
        const char* projectID,
        const char* projectRootPath,
        const char* appSupportRoot,
        const char* sceneID,
        const char* sceneMetadataPath,
        const char* sceneContentPath,
        const char* markdown,
        const char* identityID,
        const char* personaID,
        const char* displayName);

private:
    ScriviAdapter();
    ~ScriviAdapter();
    ScriviAdapter(const ScriviAdapter&)            = delete;
    ScriviAdapter& operator=(const ScriviAdapter&) = delete;

    std::atomic<int> refCount_{1};
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace scrivi::apple
```

### 9.4 Impl (in .cpp only)

```cpp
struct ScriviAdapter::Impl {
    scrivi::platform::LocalFileSystem    fileSystem;
    scrivi::platform::SystemUUIDProvider uuidProvider;
    PrototypeClock                       clock;
    PrototypeSecureStore                 secureStore;
    std::unique_ptr<scrivi::ScriviCore>  core;
};
```

`PrototypeClock` and `PrototypeSecureStore` are defined inside the `.cpp` file only. `PrototypeSecureStore` uses an in-memory `std::unordered_map`. The production replacement is `KeychainSecureStore`, deferred to a post-MVP sprint.

### 9.5 JSON envelope

Success:

```json
{ "ok": true, "result": { ... } }
```

Failure:

```json
{ "ok": false, "error": { "code": 7, "message": "project.json not found" } }
```

`code` values map to `scrivi::ErrorCode` cast to `int`. The Swift wrapper checks `ok` first; on failure it throws `ScriviError(code:message:)`.

---

## 10. CMake Target Plan

### 10.1 Root `CMakeLists.txt`

```cmake
cmake_minimum_required(VERSION 3.24)
project(Scrivi LANGUAGES CXX)
option(SCRIVI_BUILD_TESTS "Build ScriviCore tests" ON)
add_subdirectory(ScriviCore)
```

### 10.2 `ScriviCore/CMakeLists.txt`

Includes all source files; links nlohmann/json; enables tests if `SCRIVI_BUILD_TESTS` is set.

```cmake
add_library(ScriviCore STATIC)
target_compile_features(ScriviCore PUBLIC cxx_std_23)
target_include_directories(ScriviCore PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_sources(ScriviCore PRIVATE
    src/public_api/ScriviCore.cpp
    src/domain/IDs.cpp
    src/domain/Slug.cpp
    src/domain/TextStats.cpp
    # ... all other .cpp files
)
# nlohmann/json via FetchContent (hidden; not in public headers)

if(SCRIVI_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
```

### 10.3 `ScriviCore/tests/CMakeLists.txt`

```cmake
include(FetchContent)
FetchContent_Declare(Catch2
    URL https://github.com/catchorg/Catch2/archive/refs/tags/v3.6.0.tar.gz)
FetchContent_MakeAvailable(Catch2)

add_executable(ScriviCoreTests
    mocks/MockSecureStore.cpp
    mocks/MockGitProvider.cpp
    unit/ResultTests.cpp
    unit/IDTests.cpp
    unit/SlugTests.cpp
    unit/PathUtilsTests.cpp
    unit/TextStatsTests.cpp
    unit/JsonSchemaTests.cpp
    integration/IdentityTests.cpp
    integration/AppSupportLayoutTests.cpp
    integration/CreateProjectTests.cpp
    integration/OpenProjectTests.cpp
    integration/SaveSceneTests.cpp
    integration/ExternalChangeTests.cpp
    integration/GitSnapshotTests.cpp
)

target_link_libraries(ScriviCoreTests PRIVATE ScriviCore Catch2::Catch2WithMain)
include(CTest)
include(Catch)
catch_discover_tests(ScriviCoreTests)
```

Catch2 must not be linked into production targets.

### 10.4 C++23 note

CMake uses `cxx_std_23` until `cxx_std_26` is broadly available. If a C++24 feature is not consistently available across supported compilers, a project-local abstraction is preferred over a compiler guard.

---

## 11. Dependency Plan

### 11.1 Approved dependencies

| Dependency | Scope | Acquisition |
|---|---|---|
| `nlohmann/json` v3.11.3 | ScriviCore only | CMake FetchContent |
| `Catch2` v3.6.0 | Tests only | CMake FetchContent |

### 11.2 Guardrail

- `nlohmann/json` types must not appear in public ScriviCore headers.
- `Catch2` must not be linked into `ScriviCore` or any production target.
- Third-party types must not cross the `ScriviCoreAdapter` header.

---

## 12. Test Layout

### 12.1 Unit tests

```text
unit/
  ResultTests.cpp          ← Result<T> success/failure/void
  IDTests.cpp              ← ID prefix generation
  SlugTests.cpp            ← slug generation rules
  PathUtilsTests.cpp       ← path join, extension, normalization
  TextStatsTests.cpp       ← word/character counting
  JsonSchemaTests.cpp      ← JSON round-trip for each schema
```

### 12.2 Integration tests

```text
integration/
  IdentityTests.cpp        ← ensureLocalIdentity create + recover
  AppSupportLayoutTests.cpp ← AppSupportLayout directory creation
  CreateProjectTests.cpp   ← createProject minimum file set
  OpenProjectTests.cpp     ← openProject fast path + fallback
  SaveSceneTests.cpp       ← saveScene + reopen + content hash
  ExternalChangeTests.cpp  ← external edit detection
  GitSnapshotTests.cpp     ← enableGit + createSnapshot (skipped if git absent)
```

Integration tests operate against real temporary directories. They use `LocalFileSystem`, `SystemUUIDProvider`, `FixedClock`, and `MockSecureStore`. Git integration tests skip gracefully when the `git` executable is not on `PATH`.

### 12.3 Mock services

```text
mocks/
  MockSecureStore.hpp/cpp           ← in-memory SecureStore for tests
  MockGitProvider.hpp/cpp           ← deterministic GitProvider for unit tests
  FixedClock.hpp                    ← returns a fixed timestamp
  DeterministicUUIDProvider.hpp     ← returns known IDs for predictable fixtures
```

Mock services must not be compiled into production targets. They are compiled only into `ScriviCoreTests`.

---

## 13. Fixture Layout

### 13.1 `minimal-valid`

Complete valid project with one chapter and one scene. Used as the baseline for open/save tests.

### 13.2 `missing-scene-md`

Same as minimal-valid but `001-opening-scene.md` is absent.

Expected: `RepairIssue` with `category = missingContent`.

### 13.3 `missing-scene-meta`

Same as minimal-valid but `001-opening-scene.meta.json` is absent.

Expected: `RepairIssue` with `category = missingMetadata`.

### 13.4 `corrupt-scene-meta`

Same as minimal-valid but scene metadata contains invalid JSON.

Expected: `RepairIssue` with `category = corruptMetadata`.

### 13.5 `external-edit`

Baseline fixture plus a mechanism to swap the `.md` file content to simulate an external edit. Used to verify `safeExternalEdit` detection.

---

## 14. SPM Package Design

`platforms/apple/Package.swift` defines three targets:

### 14.1 `ScriviCoreAdapter` — C++ target

- Sources: `Sources/ScriviCoreAdapter/ScriviCoreAdapter.cpp`
- Public headers: `Sources/ScriviCoreAdapter/` (includes `ScriviCoreAdapter.hpp` and `module.modulemap`)
- C++ flags: `-std=c++2b`, `-I <ScriviCore/include>`, `-I <ScriviCore/src>`
- Linker: links `libScriviCore.a`

### 14.2 `Scrivi` — Swift target

- Sources: `Sources/Scrivi/ScriviEngine.swift`, `Sources/Scrivi/ScriviError.swift`
- Dependencies: `ScriviCoreAdapter`
- Swift settings: `.interoperabilityMode(.Cxx)`, `-Xcc -I<ScriviCore/include>`

### 14.3 `ScriviInteropTests` — test target

- Sources: `Tests/ScriviInteropTests/ScriviInteropTests.swift`
- Dependencies: `Scrivi`
- Framework: Swift Testing (`import Testing`)

---

## 15. Service Wiring by Platform

| Service | Apple MVP | Apple Production | Future Android |
|---|---|---|---|
| `FileSystem` | `LocalFileSystem` | same | platform equivalent |
| `UUIDProvider` | `SystemUUIDProvider` | same | platform equivalent |
| `Clock` | `PrototypeClock` (chrono) | same | same |
| `SecureStore` | `PrototypeSecureStore` (in-memory) | `KeychainSecureStore` | Android Keystore |
| `GitProvider` | `nullptr` | `SystemGitProvider` | same |
| `Logger` | `nullptr` | platform logger | platform logger |

Service wiring for Apple MVP lives inside `ScriviAdapter::Impl` in `ScriviCoreAdapter.cpp`. It is not configurable from Swift.

---

## 16. `.gitignore` Files

### 16.1 Source repository `.gitignore`

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

# SPM
.build/
.swiftpm/

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

### 16.2 User project `.gitignore`

Generated by ScriviCore when Git-backed snapshots are enabled inside a writer's project:

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

---

## 17. Implementation Milestones

The milestones below reflect the current implementation state and remaining work. Completed milestones are marked.

### Milestone 1 — Repository skeleton ✓

Source tree, CMakeLists.txt, public headers, empty ScriviCore.cpp, Catch2 test target.

### Milestone 2 — Core value types ✓

`IDs.hpp`, `Types.hpp`, `Error.hpp`, `Result.hpp`, `RepairIssue.hpp`, `Requests.hpp`, `Results.hpp`, `Services.hpp`.

### Milestone 3 — Utility foundation ✓

`PathUtils`, `Slug`, `TextStats`, `Json` wrapper, `AtomicWrite`, `Hash`.

### Milestone 4 — Mock services and platform services ✓

`FixedClock`, `DeterministicUUIDProvider`, `MockSecureStore`, `MockGitProvider`, `NullLogger`, `LocalFileSystem`, `SystemUUIDProvider`.

### Milestone 5 — Schema read/write ✓

Schema readers/writers for `project.json`, `manuscript.meta.json`, `chapter.meta.json`, `scene.meta.json`, `project-members.json`, `project-personas.json`, `workspace-state.json`.

### Milestone 6 — Project creation ✓

`ProjectCreator`, `AppSupportLayout`, `ScriviCore::createProject()`.

### Milestone 7 — Open/resume ✓

`ProjectOpener`, `ManuscriptOrderResolver`, `SceneReader`, `WorkspaceStateService` read path, `ScriviCore::openProject()`.

### Milestone 8 — Save scene ✓

`SceneWriter`, metadata update, workspace-state update, text stats, `ScriviCore::saveScene()`.

### Milestone 9 — Swift interop (ScriviCoreAdapter + ScriviEngine) ✓

`ScriviCoreAdapter` with JSON-over-`std::string` boundary. `ScriviEngine.swift`. Four adapter methods pass five integration tests.

### Milestone 10 — External change scan

`ExternalChangeScanner`, `RepairClassifier`, `ScriviCore::scanForExternalChanges()`.

Success:
- Detect missing `.md`
- Detect missing `.meta.json`
- Detect corrupt metadata
- Detect unregistered `.md`

### Milestone 11 — Git snapshots

`SystemGitProvider`, `SnapshotService`, `.gitignore` generator, `enableGitSnapshots()`, `createSnapshot()`.

Success:
- Git-enabled project initializes local repo
- Initial snapshot created
- Named snapshot created
- Snapshot metadata written
- Normal project works without Git

### Milestone 12 — KeychainSecureStore (production Apple)

Replace `PrototypeSecureStore` with a real Keychain implementation.

Success:
- Identity survives process restart on Apple platforms
- Private key material never leaves Keychain

---

## 18. CI / Build Matrix

Initial CI should build and test:

```text
macOS (CMake + Catch2 + swift test)
Ubuntu Linux (CMake + Catch2)
Windows (CMake + Catch2)
```

Minimum checks:

```text
configure CMake
build ScriviCore
run Catch2 tests (ctest)
swift build + swift test (macOS only)
```

Later additions:

```text
Android NDK build
sanitizer builds (ASan, UBSan)
coverage
static analysis
```

---

## 19. Success Criteria

The repository skeleton is successful if:

1. ScriviCore builds as a standalone static library via CMake.
2. Catch2 tests build and run without UI.
3. `platforms/apple/` builds and passes Swift interop tests via `swift test`.
4. `ScriviCoreAdapter` is the only code that crosses the C++/Swift boundary.
5. No ScriviCore types appear in `ScriviCoreAdapter.hpp`.
6. nlohmann/json is hidden behind schema wrappers in ScriviCore; used directly only in the adapter for result serialization.
7. Mock services are test-only; not compiled into production targets.
8. All data crossing the C++/Swift boundary moves by value — no raw pointers returned.
9. The boundary protocol (JSON-over-`std::string`) is sufficient for all four current adapter methods including variable-length Markdown content.
10. The repo is ready to proceed directly to Milestone 10.

---

## 20. Open Questions

1. **`appSupportRoot` on non-Apple platforms.** `AppSupportLayout` bootstraps the support directory on Apple. The path-determination mechanism for Windows, Linux, and Android is not yet defined.
2. **`JsonDoc` double support.** `ScrollPosition.value` is a `double`. The current `JsonDoc` wrapper has no `setDouble`/`getDouble`. Either add them or represent scroll as a fixed-point integer.
3. **`KeychainSecureStore` sprint timing.** `PrototypeSecureStore` does not survive process restart. This must be replaced before any real user testing. The sprint for this has not been scheduled.
4. **Cursor and scroll from Swift.** `ScriviEngine.saveScene` currently passes zero for `selection` and `scroll`. These will need wiring when the SwiftUI editor is implemented.
5. **Multi-scene `openProject` result.** The current result returns one `activeScene`. A future richer result will return a scene list for the project explorer. Schema not yet designed.
6. **Repair action operations.** `scanForExternalChanges` returns `RepairIssue` lists. Applying repair actions (relink, import, restore) is not yet in the API.
