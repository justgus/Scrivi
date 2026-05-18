# Scrivi C++24 Core API Sketch v0.2

**Project:** Scrivi  
**Document:** C++24 Core API Sketch  
**Version:** 0.2  
**Status:** Approved Direction / Implementation Sketch  
**Date:** 2026-05-18

---

## 1. Purpose

This document sketches the first public API shape for the Scrivi C++24 shared backend core.

It defines the implementation-facing boundary between:

```text
ScriviCore C++ backend
platform wrappers
native UI shells
tests
future command-line tooling
```

The API is designed around coarse backend operations, not fine-grained UI property access.

Version 0.2 incorporates the approved implementation decisions from the v0.1 open questions.

---

## 2. Approved Section 29 Decisions

The following implementation decisions are approved for this API sketch:

| # | Topic | Decision |
|---:|---|---|
| 1 | Result type | Project-local `Result<T>`, may internally use/reference `std::expected` where available |
| 2 | JSON | `nlohmann/json` |
| 3 | ID generation | UUID v7-style IDs |
| 4 | Build system | CMake acceptable, though Automake is preferred by the project lead |
| 5 | Test framework | Catch2, development/test-only dependency |
| 6 | Swift/C++ boundary | Direct Swift/C++ interop |
| 7 | Git provider | `GitProvider` abstraction; first implementation is `SystemGitProvider` calling installed `git` |
| 8 | Secure storage | `SecureStore` abstraction now; `MockSecureStore` for tests; Apple Keychain first; other platforms later |
| 9 | App-local paths | MVP passes UTF-8 `appSupportRoot`; core owns subfolders beneath it; possible `PathProvider` later |
| 10 | Paths | UTF-8 strings at wrapper/project boundary; `std::filesystem::path` internally; conservative generated filenames |

---

## 3. Git Context Clarification

Scrivi has two different Git contexts.

### 3.1 Development Git

This is the Git repository for Scrivi's own source code.

Approved direction:

```text
Scrivi development repository will be hosted on GitHub.
```

This is not what `GitProvider` means.

### 3.2 User Project Git

This is optional Git-backed history inside a writer's `.scrivi` project.

Example:

```text
MyNovel.scrivi/
  .git/
  .gitignore
  snapshots/
    scrivi-snapshots.json
```

`GitProvider` refers to this second context.

For v1, Scrivi will use:

```text
GitProvider abstraction
SystemGitProvider first implementation
```

`SystemGitProvider` means ScriviCore calls the installed `git` command-line executable to support optional writer-facing local snapshots.

Example operations:

```text
git init
git add .
git commit -m "Initial project"
git status --porcelain
```

If Git is not installed, normal Scrivi projects still work. Only optional Git-backed snapshots are unavailable.

---

## 4. Design Context

This API sketch follows the approved direction:

```text
C++24 shared backend core from the start
native/platform-appropriate UI shells
thin platform wrappers
Markdown manuscript files
visible JSON metadata files
optional Git-backed snapshots
app-local workspace state
app-local rebuildable caches
no mandatory SQLite in MVP
future-compatible signing/encryption/group-persona hooks
```

This API sketch is based on:

```text
Scrivi Backend Runtime Trade Study v0.2
Scrivi C++24 Backend Core Plan v0.1
Scrivi Backend MVP Slice v0.1
Scrivi Minimum Schema Set v0.1
Scrivi Project Creation and Open Flow v0.1
Scrivi External Change Repair Matrix v0.1
```

---

## 5. API Design Goals

The C++ API should:

1. Expose coarse workflow-level operations.
2. Avoid UI framework dependencies.
3. Return structured results and repair issues.
4. Avoid throwing exceptions across wrapper boundaries.
5. Keep ownership simple.
6. Use platform services through injected interfaces.
7. Preserve platform-neutral project formats.
8. Be testable without UI.
9. Be wrapper-friendly for Swift, JNI/Android, Windows, and Linux shells.
10. Keep MVP scope narrow.

---

## 6. Non-Goals

The API should not expose:

```text
UI view types
rich text editor internals
Swift/AppKit/UIKit/RealityKit concepts
Android View/Compose concepts
Windows/Linux UI concepts
SQLite implementation details
platform keychain details
Git command-line details
low-level JSON library types
third-party dependency types
```

The API should not implement:

```text
complete object graph
timeline engine
comments
assets
exports
remote Git
real-time collaboration
encryption
signature validation
group personas
```

---

## 7. Namespace

All public C++ types should live under:

```cpp
namespace scrivi { }
```

Internal implementation details should live under sub-namespaces or private source directories.

Example:

```cpp
namespace scrivi {
    // public API
}

namespace scrivi::detail {
    // implementation details, not wrapper-facing
}
```

---

## 8. ID Types

### 8.1 Design

Use lightweight strong-ish ID wrappers over strings.

IDs are UUID v7-style opaque strings with type prefixes.

Examples:

```text
project_01J8Z7V6B5W4Q3R2N1M0K9J8H7
manuscript_01J8Z8D4C3B2A1N0M9K8J7H6G5
chapter_01J8Z8H9K7M6N5B4V3C2X1Z0A9
scene_01J8Z8P4N3M2K1J0H9G8F7E6D5
identity_01J8Z7Y9R6T5E4W3Q2A1S0D9F8
persona_01J8Z801V6B5N4M3K2J1H0G9F8
```

### 8.2 Sketch

```cpp
#pragma once

#include <string>
#include <string_view>

namespace scrivi {

struct ProjectID {
    std::string value;
};

struct ManuscriptID {
    std::string value;
};

struct ChapterID {
    std::string value;
};

struct SceneID {
    std::string value;
};

struct IdentityID {
    std::string value;
};

struct PersonaID {
    std::string value;
};

struct SnapshotID {
    std::string value;
};

struct CommitID {
    std::string value;
};

}
```

### 8.3 MVP Rule

IDs are authoritative.

Paths, titles, slugs, and filenames are not identity.

---

## 9. Common Scalar Types

Wrapper-facing paths should be UTF-8 strings.

Internally, ScriviCore may convert to `std::filesystem::path`.

```cpp
namespace scrivi {

using Utf8Text = std::string;
using ISO8601Timestamp = std::string;
using Slug = std::string;
using RelativePath = std::string;
using AbsolutePath = std::string;

}
```

### 9.1 UTF-8 Path Policy

Approved direction:

```text
Wrapper-facing paths: UTF-8 strings
Project metadata paths: UTF-8 relative paths
Internal file operations: std::filesystem::path
Generated filenames: conservative ASCII slugs by default
```

ScriviCore must not assume that every Unicode string is valid on every target filesystem.

Generated filenames should be conservative:

```text
chapter-001
001-opening-scene.md
001-opening-scene.meta.json
```

Titles may contain Unicode even when generated filenames are conservative.

---

## 10. Result and Error Model

### 10.1 Principle

The public API should return explicit `Result<T>` values.

Do not rely on exceptions crossing wrapper boundaries.

### 10.2 Approved Direction

Use a project-local `Result<T>` type.

Internally, the implementation may use or reference `std::expected` where toolchain support is reliable.

The public API should not require platform wrappers to depend directly on `std::expected`.

### 10.3 Error categories

```cpp
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

}
```

### 10.4 Error type

```cpp
namespace scrivi {

struct Error {
    ErrorCode code = ErrorCode::ok;
    std::string message;
    std::string path;
    std::string detail;
};

}
```

### 10.5 Result type

```cpp
namespace scrivi {

template <typename T>
class Result {
public:
    static Result<T> success(T value);
    static Result<T> failure(Error error);

    bool ok() const;
    const T& value() const;
    T& value();
    const Error& error() const;

private:
    // Implementation may use std::expected internally where available.
};

template <>
class Result<void> {
public:
    static Result<void> success();
    static Result<void> failure(Error error);

    bool ok() const;
    const Error& error() const;

private:
    // Implementation intentionally omitted.
};

}
```

### 10.6 Wrapper Rule

Wrappers should convert `Result<T>` into platform-native success/failure models.

They should not parse plain error strings to determine behavior.

---

## 11. Repair Issue Model

### 11.1 Severity

```cpp
namespace scrivi {

enum class RepairSeverity {
    info,
    warning,
    blocking
};

}
```

### 11.2 Repair category

```cpp
namespace scrivi {

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

}
```

### 11.3 Suggested action

```cpp
namespace scrivi {

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

}
```

### 11.4 Repair issue

```cpp
namespace scrivi {

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

### 11.5 Rule

The scanner classifies issues.

It does not silently repair ambiguous issues.

---

## 12. Authorship and Persona Types

### 12.1 Authorship reference

```cpp
namespace scrivi {

struct AuthorshipRef {
    IdentityID identityID;
    PersonaID personaID;
    std::string displayName;
};

struct AuthorshipAtCreation {
    IdentityID identityID;
    PersonaID personaID;
    std::string displayNameAtCreation;
};

struct AuthorshipAtModification {
    IdentityID identityID;
    PersonaID personaID;
    std::string displayNameAtModification;
};

}
```

### 12.2 Persona kind

```cpp
namespace scrivi {

enum class PersonaKind {
    individual,
    group // future hook; not implemented in MVP
};

}
```

### 12.3 Project role

```cpp
namespace scrivi {

enum class ProjectRole {
    owner,
    editor,
    reader
};

enum class MemberStatus {
    active,
    removed
};

}
```

---

## 13. Workspace Types

### 13.1 Cursor and scroll

```cpp
namespace scrivi {

struct TextSelection {
    std::size_t anchor = 0;
    std::size_t focus = 0;
};

struct ScrollPosition {
    double value = 0.0; // 0.0 top, 1.0 bottom; UI may refine later.
};

}
```

### 13.2 Last writing surface

```cpp
namespace scrivi {

struct LastWritingSurface {
    SceneID sceneID;
    RelativePath contentPath;

    TextSelection selection;
    ScrollPosition scroll;
};

}
```

### 13.3 Workspace state

```cpp
namespace scrivi {

struct WorkspaceState {
    ProjectID projectID;

    std::string deviceID;
    IdentityID identityID;
    PersonaID activePersonaID;

    std::optional<LastWritingSurface> lastWritingSurface;

    ISO8601Timestamp lastOpenedAt;
};

}
```

---

## 14. Open Modes

```cpp
namespace scrivi {

enum class OpenMode {
    normalEdit,
    editWithWarnings,
    repairRequired,
    readOnly,
    cannotOpen
};

}
```

---

## 15. Project Summary Types

### 15.1 Project summary

```cpp
namespace scrivi {

struct ProjectSummary {
    ProjectID projectID;
    std::string title;
    Slug slug;

    AbsolutePath rootPath;

    bool gitSnapshotsEnabled = false;
};

}
```

### 15.2 Scene summary

```cpp
namespace scrivi {

enum class ManuscriptStatus {
    draft,
    revised,
    final,
    archived
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

## 16. Service Interfaces

Platform-specific services should be injected.

---

### 16.1 Clock

```cpp
namespace scrivi {

class Clock {
public:
    virtual ~Clock() = default;
    virtual ISO8601Timestamp nowUTC() = 0;
};

}
```

---

### 16.2 UUID Provider

Approved direction:

```text
UUID v7-style IDs
```

```cpp
namespace scrivi {

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

}
```

---

### 16.3 FileSystem

```cpp
namespace scrivi {

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

}
```

### Note

`removeFile` exists as a primitive but should not be used for destructive repair without confirmation.

---

### 16.4 SecureStore

Approved direction:

```text
SecureStore abstraction now
MockSecureStore for tests
Apple Keychain first production implementation
Windows/Linux/Android implementations later
```

```cpp
namespace scrivi {

using SecretBytes = std::vector<std::byte>;

class SecureStore {
public:
    virtual ~SecureStore() = default;

    virtual Result<bool> containsSecret(std::string_view key) = 0;
    virtual Result<void> putSecret(std::string_view key, const SecretBytes& value) = 0;
    virtual Result<SecretBytes> getSecret(std::string_view key) = 0;
};

}
```

### Rule

The project package must never store private identity keys.

Production builds must not use `MockSecureStore` or plain app-local private key storage.

---

### 16.5 GitProvider

Approved direction:

```text
GitProvider abstraction
SystemGitProvider first implementation
```

`SystemGitProvider` calls the installed `git` command-line executable for optional user-project snapshots.

```cpp
namespace scrivi {

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

}
```

### MVP Behavior

If Git is unavailable, Scrivi should return `gitUnavailable` for Git-backed snapshot operations while leaving ordinary Scrivi projects functional.

---

### 16.6 Logger

```cpp
namespace scrivi {

enum class LogLevel {
    debug,
    info,
    warning,
    error
};

class Logger {
public:
    virtual ~Logger() = default;

    virtual void log(
        LogLevel level,
        std::string_view message) = 0;
};

}
```

---

## 17. Core Dependencies Container

### 17.1 Purpose

The facade receives platform services through a dependency container.

```cpp
namespace scrivi {

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

### 17.2 Rule

`ScriviCore` does not own these services unless explicitly designed otherwise.

Wrappers must ensure service lifetimes exceed the `ScriviCore` instance.

---

## 18. App-Local Path Policy

Approved direction:

```text
MVP request structs pass appSupportRoot as a UTF-8 string.
ScriviCore owns subfolder conventions beneath appSupportRoot.
PathProvider may be introduced later if needed.
```

Initial app-local layout under `appSupportRoot`:

```text
<appSupportRoot>/
  identity/
  state/
    projects/
      <projectID>/
        workspace-state.json
  cache/
    projects/
      <projectID>/
  logs/
  tmp/
```

The platform wrapper decides the correct `appSupportRoot`.

Examples:

```text
Apple wrapper: platform-appropriate Application Support location
Windows wrapper: platform-appropriate AppData location
Linux wrapper: platform-appropriate XDG location
Android wrapper: app-private files directory
```

ScriviCore should not hardcode those platform roots.

---

## 19. Public Facade

```cpp
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
};

}
```

---

## 20. Identity API

### 20.1 Ensure identity request

```cpp
namespace scrivi {

struct EnsureIdentityRequest {
    std::string requestedDisplayName;

    // App-local root supplied by wrapper/platform.
    AbsolutePath appSupportRoot;
};

}
```

### 20.2 Ensure identity result

```cpp
namespace scrivi {

struct EnsureIdentityResult {
    IdentityID identityID;
    PersonaID defaultPersonaID;

    std::string displayName;

    bool createdNewIdentity = false;
};

}
```

### 20.3 MVP behavior

If identity exists, return it.

If identity does not exist, create:

```text
device identity
Scrivi identity
default persona
certificate-ready identity material
secure-store private material
```

---

## 21. Create Project API

### 21.1 Request

```cpp
namespace scrivi {

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

}
```

### 21.2 Result

```cpp
namespace scrivi {

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

}
```

### 21.3 Behavior

Creates the minimum package:

```text
project.json
manuscript/manuscript.meta.json
manuscript/chapter-001/chapter.meta.json
manuscript/chapter-001/001-opening-scene.md
manuscript/chapter-001/001-opening-scene.meta.json
identities/project-members.json
identities/project-personas.json
```

If Git is enabled:

```text
.git/
.gitignore
snapshots/scrivi-snapshots.json
initial commit/snapshot
```

---

## 22. Open Project API

### 22.1 Request

```cpp
namespace scrivi {

struct OpenProjectRequest {
    AbsolutePath projectRootPath;
    AbsolutePath appSupportRoot;

    std::optional<IdentityID> currentIdentityID;
};

}
```

### 22.2 Result

```cpp
namespace scrivi {

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

}
```

### 22.3 Fast path

```text
read project.json
read project-members.json
read project-personas.json
read workspace-state.json
resolve last scene
read scene.meta.json
read scene.md
return editor-ready result
```

### 22.4 Fallback path

If workspace state is invalid:

```text
read manuscript.meta.json
resolve first chapter
read chapter.meta.json
resolve first scene
read scene.meta.json
read scene.md
return editor-ready result
```

---

## 23. Save Scene API

### 23.1 Request

```cpp
namespace scrivi {

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

    // Optional optimistic conflict detection.
    std::optional<std::string> previouslyLoadedContentHash;
};

}
```

### 23.2 Result

```cpp
namespace scrivi {

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

}
```

### 23.3 Behavior

```text
safe-write Markdown
update modifiedAt
update modifiedBy
update derived stats
update workspace-state.json
mark app-local indexes dirty if present
return conflict issue if disk changed unexpectedly
```

---

## 24. External Change Scan API

### 24.1 Request

```cpp
namespace scrivi {

struct ExternalChangeScanRequest {
    AbsolutePath projectRootPath;
    AbsolutePath appSupportRoot;

    bool includeGitStatus = true;
};

}
```

### 24.2 Result

```cpp
namespace scrivi {

struct ExternalChangeScanResult {
    ProjectID projectID;

    std::vector<RepairIssue> repairIssues;

    bool indexesDirty = false;
    bool gitStatusChecked = false;
    bool hasUnsnapshottedChanges = false;
};

}
```

### 24.3 MVP detection scope

Must detect:

```text
scene.md changed externally
scene.md missing
scene.meta.json missing
scene.meta.json corrupt/unreadable
workspace last scene invalid
new unregistered .md in manuscript folder
```

---

## 25. Enable Git Snapshots API

### 25.1 Request

```cpp
namespace scrivi {

struct EnableGitRequest {
    AbsolutePath projectRootPath;

    AuthorshipRef author;

    std::string initialSnapshotLabel = "Initial project";
};

}
```

### 25.2 Result

```cpp
namespace scrivi {

struct EnableGitResult {
    bool gitInitialized = false;
    bool alreadyRepository = false;

    SnapshotID initialSnapshotID;
    CommitID initialCommitID;

    std::vector<RepairIssue> warnings;
};

}
```

### 25.3 Behavior

```text
detect existing Git repo
initialize if needed
write/update .gitignore
create snapshots/scrivi-snapshots.json
create initial snapshot
```

---

## 26. Create Snapshot API

### 26.1 Request

```cpp
namespace scrivi {

struct CreateSnapshotRequest {
    AbsolutePath projectRootPath;

    AuthorshipRef author;

    std::string label;
    std::string note;
};

}
```

### 26.2 Result

```cpp
namespace scrivi {

struct CreateSnapshotResult {
    SnapshotID snapshotID;
    CommitID commitID;

    ISO8601Timestamp createdAt;

    bool created = false;
};

}
```

### 26.3 Behavior

```text
git add canonical project files
git commit with persona-mapped author
append snapshot metadata
return snapshot ID and commit ID
```

---

## 27. Swift/C++ Interop

Approved direction:

```text
Direct Swift/C++ interop
```

### 27.1 Swift wrapper responsibilities

The Swift wrapper should:

```text
call C++ APIs directly where practical
convert Swift path/string/model types to ScriviCore types
convert ScriviCore results to Swift result models
provide AppleKeychainSecureStore
provide platform appSupportRoot
map RepairIssue values to UI states
avoid reimplementing backend behavior
```

### 27.2 Guardrail

If direct interop becomes awkward for specific APIs, a narrow C ABI bridge may be added later.

However, v0.2 does not choose a JSON-over-C ABI boundary as the primary Swift strategy.

---

## 28. C ABI Boundary Option

A C ABI may still be useful later for Android/JNI, scripting tools, or non-Swift wrappers.

### 28.1 Strategy

Expose coarse JSON request/result functions at the C ABI boundary only if needed.

Sketch:

```c
typedef struct ScriviCoreHandle ScriviCoreHandle;

ScriviCoreHandle* scrivi_core_create(/* service wiring opaque for platform */);
void scrivi_core_destroy(ScriviCoreHandle* handle);

char* scrivi_create_project(
    ScriviCoreHandle* handle,
    const char* json_request_utf8);

char* scrivi_open_project(
    ScriviCoreHandle* handle,
    const char* json_request_utf8);

char* scrivi_save_scene(
    ScriviCoreHandle* handle,
    const char* json_request_utf8);

void scrivi_free_string(char* value);
```

### 28.2 Status

Not the primary Apple wrapper strategy.

Possible future support boundary.

---

## 29. JSON Strategy

Approved direction:

```text
nlohmann/json
```

### 29.1 Use

Use `nlohmann/json` for MVP schema loading and writing.

### 29.2 Guardrail

Do not expose `nlohmann::json` through the public ScriviCore API.

It should remain behind:

```text
schema readers
schema writers
JSON utility wrappers
tests
```

This preserves the option to replace the JSON implementation later.

---

## 30. Build and Test Strategy

### 30.1 Build system

Approved direction:

```text
CMake acceptable
Automake preferred by project lead, but not selected as required for ScriviCore v0.2
```

CMake is acceptable because it provides a practical cross-platform path for:

```text
macOS
Windows
Linux
Android/NDK
test targets
future wrapper integration
```

### 30.2 Test framework

Approved direction:

```text
Catch2
```

Catch2 is a development/test-only dependency.

It should not be linked into production Scrivi app targets.

Example build policy:

```cmake
option(SCRIVI_BUILD_TESTS "Build ScriviCore tests" ON)
```

Production app targets link ScriviCore, not ScriviCoreTests or Catch2.

---

## 31. Example Create Project Flow

```cpp
scrivi::CoreServices services {
    .fileSystem = &fileSystem,
    .secureStore = &secureStore,
    .clock = &clock,
    .uuidProvider = &uuidProvider,
    .gitProvider = &gitProvider,
    .logger = &logger
};

scrivi::ScriviCore core { services };

scrivi::CreateProjectRequest request;
request.projectRootPath = "/Users/julie/Documents/MyNovel.scrivi";
request.appSupportRoot = "/Users/julie/Library/Application Support/Scrivi";
request.title = "My Novel";
request.slug = "my-novel";
request.author = {
    .identityID = { "identity_01J8Z7Y9R6T5E4W3Q2A1S0D9F8" },
    .personaID = { "persona_01J8Z801V6B5N4M3K2J1H0G9F8" },
    .displayName = "Julie Stoddard"
};
request.enableGitSnapshots = true;

auto result = core.createProject(request);

if (!result.ok()) {
    // Map result.error() to UI.
} else {
    // Open result.value().firstSceneContentPath.
}
```

---

## 32. Example Open Project Flow

```cpp
scrivi::OpenProjectRequest request;
request.projectRootPath = "/Users/julie/Documents/MyNovel.scrivi";
request.appSupportRoot = "/Users/julie/Library/Application Support/Scrivi";
request.currentIdentityID = scrivi::IdentityID {
    "identity_01J8Z7Y9R6T5E4W3Q2A1S0D9F8"
};

auto result = core.openProject(request);

if (!result.ok()) {
    // Fatal open error.
}

auto open = result.value();

switch (open.mode) {
    case scrivi::OpenMode::normalEdit:
    case scrivi::OpenMode::editWithWarnings:
        // Show editor with open.activeSceneMarkdown.
        break;

    case scrivi::OpenMode::repairRequired:
        // Show repair UI.
        break;

    case scrivi::OpenMode::readOnly:
        // Show read-only editor.
        break;

    case scrivi::OpenMode::cannotOpen:
        // Show cannot-open state.
        break;
}
```

---

## 33. API Stability Rule

This is v0.2.

The API is expected to change during implementation.

However, the following should remain stable unless the architecture changes:

```text
coarse operation boundary
project-local Result<T>
structured Error model
RepairIssue model
platform service injection
SecureStore abstraction
GitProvider abstraction
appSupportRoot MVP path strategy
direct Swift/C++ interop preference
UTF-8 wrapper/project paths
no UI dependencies
project package neutrality
```

---

## 34. MVP Acceptance Criteria

The API sketch is successful if it can support:

1. First launch identity creation.
2. Project creation.
3. Project opening.
4. Resume last writing surface.
5. Save scene.
6. Workspace state update.
7. Missing `.md` detection.
8. Missing `.meta.json` detection.
9. Corrupt metadata detection.
10. External `.md` edit detection.
11. Optional Git initialization through `SystemGitProvider`.
12. Optional snapshot creation.
13. Swift wrapper prototype using direct Swift/C++ interop.
14. Platform-neutral tests using Catch2.
15. No production dependency on test framework.
16. No private identity material in `.scrivi` project packages.

---

## 35. Remaining Open Questions

The Section 29 questions are now closed.

Remaining implementation questions belong to the next document:

```text
Scrivi C++24 Core Repository Skeleton v0.1
```

Topics to resolve there:

1. Final directory layout.
2. Exact CMake target names.
3. Dependency acquisition strategy.
4. Initial header/source file list.
5. Catch2 integration.
6. Fixture layout.
7. Initial mock service implementations.
8. Initial Swift wrapper target strategy.
9. SystemGitProvider process execution details.
10. AppleKeychainSecureStore wrapper placement.
11. CI target matrix.

---

## 36. Recommended Next Document

The next implementation-facing document should be:

```text
Scrivi C++24 Core Repository Skeleton v0.1
```

It should define:

```text
directory layout
CMake targets
dependency choices
initial headers
test framework integration
fixture layout
mock services
first implementation milestones
```

After that, implementation can begin.
