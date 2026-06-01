# Scrivi C++24 Core API Sketch v0.3

**Project:** Scrivi  
**Document:** C++24 Core API Sketch  
**Version:** 0.3  
**Status:** Approved Direction  
**Date:** 2026-05-21  
**Supersedes:** `Scrivi_Cpp24_Core_API_Sketch_v0_2.md`

---

## 1. Purpose

This document defines the public API shape for the Scrivi C++24 shared backend core and the boundary design for all platform wrappers.

Version 0.3 incorporates the architectural decisions from `Scrivi_Architecture_v0_3.md`. The primary change from v0.2 is the reversal of the Swift/C++ boundary strategy: **JSON-over-`std::string` is the primary and permanent boundary protocol** for all platform wrappers, not a fallback. Direct struct interop with ScriviCore public headers is not the Apple strategy.

Everything else from v0.2 is confirmed unchanged.

---

## 2. Approved Decisions

| # | Topic | Decision |
|---:|---|---|
| 1 | Result type | Project-local `Result<T>`; may internally reference `std::expected` |
| 2 | JSON | `nlohmann/json` v3.11.3 |
| 3 | ID generation | UUID v7-style IDs with typed prefixes |
| 4 | Build system | CMake for ScriviCore; SPM for Apple wrapper |
| 5 | Test framework | Catch2 (C++); Swift Testing (Apple wrapper) |
| 6 | Cross-language boundary | JSON-over-`std::string` via `ScriviCoreAdapter` — permanent, not a workaround |
| 7 | Git provider | `GitProvider` abstraction; `SystemGitProvider` first implementation |
| 8 | Secure storage | `SecureStore` abstraction; `PrototypeSecureStore` for MVP; `KeychainSecureStore` for production Apple |
| 9 | App-local paths | `AppSupportLayout::platformDefault()` resolves the OS root at the C++ layer (Option A, decided 2026-05-30); callers may override with an explicit path for tests or non-standard installs |
| 10 | Paths | UTF-8 strings at wrapper/project boundary; `std::filesystem::path` internally |

---

## 3. Git Context Clarification

Scrivi has two different Git contexts.

**Development Git** — the source repository for Scrivi itself, hosted on GitHub.

**User Project Git** — optional Git-backed history inside a writer's `.scrivi` project. `GitProvider` refers to this context only. `SystemGitProvider` calls the installed `git` command-line executable for local user-project snapshots. If Git is not installed, only Git-backed snapshot operations are unavailable; normal Scrivi projects work regardless.

---

## 4. API Design Goals

The C++ API should:

1. Expose coarse workflow-level operations, not fine-grained object accessors.
2. Avoid UI framework dependencies.
3. Return structured results and repair issues.
4. Avoid throwing exceptions. All errors travel via `Result<T>`.
5. Use platform services through injected interfaces.
6. Preserve platform-neutral project formats.
7. Be testable without UI.
8. Be adaptable to any platform via the `ScriviCoreAdapter` pattern.
9. Keep MVP scope narrow.

---

## 5. Non-Goals

The API must not expose:

```text
UI view types
Swift/AppKit/UIKit/RealityKit concepts
Android View/Compose concepts
Windows/Linux UI concepts
SQLite implementation details
platform Keychain details
Git command-line details
nlohmann/json types in public headers
third-party dependency types
```

The API does not implement:

```text
full object graph (characters, locations, timelines)
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

## 6. Namespace

```cpp
namespace scrivi { }         // public API
namespace scrivi::detail { } // internal implementation, not wrapper-facing
namespace scrivi::apple { }  // adapter and Apple-specific shims
```

---

## 7. ID Types

IDs are UUID v7-style opaque strings with typed prefixes:

```text
project_01J8Z7V6B5W4Q3R2N1M0K9J8H7
manuscript_01J8Z8D4C3B2A1N0M9K8J7H6G5
chapter_01J8Z8H9K7M6N5B4V3C2X1Z0A9
scene_01J8Z8P4N3M2K1J0H9G8F7E6D5
identity_01J8Z7Y9R6T5E4W3Q2A1S0D9F8
persona_01J8Z801V6B5N4M3K2J1H0G9F8
```

IDs are authoritative. Paths, titles, slugs, and filenames are human-facing conveniences.

```cpp
struct ProjectID    { std::string value; };
struct ManuscriptID { std::string value; };
struct ChapterID    { std::string value; };
struct SceneID      { std::string value; };
struct IdentityID   { std::string value; };
struct PersonaID    { std::string value; };
struct SnapshotID   { std::string value; };
struct CommitID     { std::string value; };
```

---

## 8. Common Scalar Types

```cpp
using Utf8Text         = std::string;
using ISO8601Timestamp = std::string;
using Slug             = std::string;
using RelativePath     = std::string;
using AbsolutePath     = std::string;
```

**Path policy:** UTF-8 strings at the wrapper/project boundary; `std::filesystem::path` internally. Generated filenames use conservative ASCII slugs (`chapter-001`, `001-opening-scene.md`). Titles may contain Unicode.

---

## 9. Result and Error Model

### 9.1 Principle

All operations return `Result<T>`. Exceptions are not used and must not cross any boundary.

### 9.2 `ErrorCode`

```cpp
enum class ErrorCode {
    ok,
    invalidArgument, unsupportedVersion,
    ioError, permissionDenied, parseError, validationError,
    repairRequired,
    gitUnavailable, gitError,
    secureStoreUnavailable, secureStoreError,
    identityError,
    internalError
};
```

### 9.3 `Error`

```cpp
struct Error {
    ErrorCode   code    = ErrorCode::ok;
    std::string message;
    std::string path;
    std::string detail;
};
```

### 9.4 `Result<T>`

```cpp
template <typename T>
class Result {
public:
    static Result<T> success(T value);
    static Result<T> failure(Error error);
    bool         ok()    const;
    const T&     value() const;
    T&           value();
    const Error& error() const;
};

template <>
class Result<void> {
public:
    static Result<void> success();
    static Result<void> failure(Error error);
    bool         ok()    const;
    const Error& error() const;
};
```

### 9.5 Wrapper rule

Platform wrappers convert `Result<T>` into platform-native success/failure models. The JSON envelope at the adapter boundary uses `{ "ok": true/false, "error": { "code": N, "message": "..." } }`. Platform wrappers must not parse error message strings to determine control flow — they use `code`.

---

## 10. Repair Issue Model

```cpp
enum class RepairSeverity    { info, warning, blocking };
enum class RepairCategory    { none, safeExternalEdit, unregisteredManuscriptFile,
                               missingContent, missingMetadata, possibleRename,
                               orphanMetadata, corruptMetadata, unsupportedSchema,
                               gitStateChanged, mergeConflict, unknownFile, unknownIssue };
enum class RepairActionKind  { none, reloadExternalVersion, keepCurrentVersion,
                               saveCurrentVersionAsCopy, importAsNewScene,
                               attachToExistingScene, regenerateMetadata,
                               restoreFromSnapshot, createEmptyContentFile,
                               relinkToFile, markMissing, removeFromProject,
                               moveToInbox, ignore, deleteAfterConfirmation,
                               openReadOnly, cancelOpen };

struct RepairAction { RepairActionKind kind; std::string label; std::string detail; };
struct RepairIssue  {
    std::string issueID;
    RepairSeverity severity; RepairCategory category;
    std::string title; std::string message;
    std::string path;  std::string relatedPath;
    ProjectID projectID; ChapterID chapterID; SceneID sceneID;
    std::vector<RepairAction> suggestedActions;
};
```

The scanner classifies issues. It does not silently repair ambiguous conditions.

---

## 11. Authorship Types

```cpp
struct AuthorshipRef {
    IdentityID  identityID;
    PersonaID   personaID;
    std::string displayName;
};
```

---

## 12. Workspace Types

```cpp
struct TextSelection   { std::size_t anchor = 0; std::size_t focus = 0; };
struct ScrollPosition  { double value = 0.0; };

struct LastWritingSurface {
    SceneID       sceneID;
    RelativePath  contentPath;
    TextSelection  selection;
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
```

---

## 13. Open Modes

```cpp
enum class OpenMode { normalEdit, editWithWarnings, repairRequired, readOnly, cannotOpen };
```

---

## 14. Project Summary Types

```cpp
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
```

---

## 15. Service Interfaces

Platform-specific services are injected. ScriviCore never instantiates them.

### 15.1 `Clock`

```cpp
class Clock { public: virtual ISO8601Timestamp nowUTC() = 0; };
```

### 15.2 `UUIDProvider`

```cpp
class UUIDProvider {
public:
    virtual ProjectID    newProjectID()    = 0;
    virtual ManuscriptID newManuscriptID() = 0;
    virtual ChapterID    newChapterID()    = 0;
    virtual SceneID      newSceneID()      = 0;
    virtual IdentityID   newIdentityID()   = 0;
    virtual PersonaID    newPersonaID()    = 0;
    virtual SnapshotID   newSnapshotID()   = 0;
};
```

### 15.3 `FileSystem`

```cpp
class FileSystem {
public:
    virtual Result<bool>     exists(const AbsolutePath&)          = 0;
    virtual Result<bool>     isDirectory(const AbsolutePath&)     = 0;
    virtual Result<void>     createDirectories(const AbsolutePath&) = 0;
    virtual Result<Utf8Text> readTextFile(const AbsolutePath&)    = 0;
    virtual Result<void>     atomicWriteTextFile(const AbsolutePath&, std::string_view) = 0;
    virtual Result<std::vector<AbsolutePath>> listDirectory(const AbsolutePath&) = 0;
    virtual Result<void>     removeFile(const AbsolutePath&)      = 0;
};
```

`removeFile` is a low-level primitive. ScriviCore must not call it for destructive repair without caller confirmation.

### 15.4 `SecureStore`

```cpp
using SecretBytes = std::vector<std::byte>;

class SecureStore {
public:
    virtual Result<bool>        containsSecret(std::string_view key)                      = 0;
    virtual Result<void>        putSecret(std::string_view key, const SecretBytes& value) = 0;
    virtual Result<SecretBytes> getSecret(std::string_view key)                           = 0;
};
```

The project package must never store private identity keys. Production builds must not use `PrototypeSecureStore`.

### 15.5 `GitProvider`

```cpp
struct GitAuthor     { std::string name; std::string email; };
struct CommitRequest { std::string message; GitAuthor author; };
struct GitStatus {
    bool isRepository = false;
    bool hasUncommittedChanges = false;
    std::vector<RelativePath> changedFiles;
    std::vector<RelativePath> untrackedFiles;
};

class GitProvider {
public:
    virtual Result<bool>      isRepository(const AbsolutePath&) = 0;
    virtual Result<void>      initRepository(const AbsolutePath&) = 0;
    virtual Result<void>      addAll(const AbsolutePath&) = 0;
    virtual Result<CommitID>  commit(const AbsolutePath&, const CommitRequest&) = 0;
    virtual Result<GitStatus> status(const AbsolutePath&) = 0;
};
```

If Git is unavailable, `gitUnavailable` is returned for snapshot operations; ordinary projects remain functional.

### 15.6 `Logger`

```cpp
enum class LogLevel { debug, info, warning, error };
class Logger { public: virtual void log(LogLevel, std::string_view) = 0; };
```

### 15.7 `CoreServices`

```cpp
struct CoreServices {
    FileSystem*   fileSystem   = nullptr;
    SecureStore*  secureStore  = nullptr;
    Clock*        clock        = nullptr;
    UUIDProvider* uuidProvider = nullptr;
    GitProvider*  gitProvider  = nullptr;
    Logger*       logger       = nullptr;
};
```

ScriviCore does not own these services. The adapter (or test harness) must ensure service lifetimes exceed the `ScriviCore` instance.

---

## 16. App-Local Path Policy

```text
MVP: request structs pass appSupportRoot as a UTF-8 string.
ScriviCore owns subfolder layout beneath appSupportRoot.
PathProvider may be introduced later if needed.
```

App-local layout:

```text
<appSupportRoot>/
  identity/
  state/
    projects/<projectID>/workspace-state.json
  cache/
    projects/<projectID>/
  logs/
  tmp/
```

The platform wrapper (Swift, Android, etc.) determines the correct `appSupportRoot`. ScriviCore does not hardcode any platform path.

---

## 17. Public Facade

```cpp
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
};
```

---

## 18. Identity API

### 18.1 Request

```cpp
struct EnsureIdentityRequest {
    std::string  requestedDisplayName;
    AbsolutePath appSupportRoot;
};
```

### 18.2 Result

```cpp
struct EnsureIdentityResult {
    IdentityID  identityID;
    PersonaID   defaultPersonaID;
    std::string displayName;
    bool        createdNewIdentity = false;
};
```

### 18.3 Behavior

If identity exists, return it. If it does not exist, create: device identity, Scrivi identity, default persona, identity material, secure-store private material.

---

## 19. Create Project API

### 19.1 Request

```cpp
struct CreateProjectRequest {
    AbsolutePath  projectRootPath;
    std::string   title;
    Slug          slug;
    AuthorshipRef author;
    std::string   initialChapterTitle = "Chapter 1";
    std::string   initialChapterSlug  = "chapter-001";
    std::string   initialSceneTitle   = "Opening Scene";
    std::string   initialSceneSlug    = "001-opening-scene";
    bool          enableGitSnapshots  = false;
    AbsolutePath  appSupportRoot;
};
```

### 19.2 Result

```cpp
struct CreateProjectResult {
    ProjectSummary project;
    ManuscriptID   manuscriptID;
    ChapterID      firstChapterID;
    SceneID        firstSceneID;
    RelativePath   firstSceneMetadataPath;
    RelativePath   firstSceneContentPath;
    WorkspaceState workspaceState;
    bool           gitInitialized = false;
    std::optional<SnapshotID> initialSnapshotID;
    std::vector<RepairIssue>  warnings;
};
```

### 19.3 Behavior

Creates the minimum package: `project.json`, `manuscript/manuscript.meta.json`, first chapter folder and metadata, first scene `.md` and `.meta.json`, `identities/project-members.json`, `identities/project-personas.json`, app-local workspace state. If Git is enabled: `.git/`, `.gitignore`, `snapshots/scrivi-snapshots.json`, initial commit/snapshot.

---

## 20. Open Project API

### 20.1 Request

```cpp
struct OpenProjectRequest {
    AbsolutePath              projectRootPath;
    AbsolutePath              appSupportRoot;
    std::optional<IdentityID> currentIdentityID;
};
```

### 20.2 Result

```cpp
struct OpenProjectResult {
    OpenMode    mode = OpenMode::cannotOpen;
    ProjectSummary project;
    std::optional<WorkspaceState>  workspaceState;
    std::optional<SceneSummary>    activeScene;
    Utf8Text    activeSceneMarkdown;    // full scene content — travels by value
    TextSelection   restoredSelection;
    ScrollPosition  restoredScroll;
    std::vector<RepairIssue> repairIssues;
};
```

`activeSceneMarkdown` contains the full UTF-8 scene content. It is serialized into the JSON result string at the adapter boundary. Swift receives it as a value; no C++ pointer to this data escapes the call.

### 20.3 Fast path

```text
read project.json → validate
read project-members.json, project-personas.json
read workspace-state.json
resolve lastWritingSurface.sceneID
read scene.meta.json
read scene.md
return with mode=normalEdit
```

### 20.4 Fallback path

If workspace state is missing or invalid: resolve first chapter from `manuscript.meta.json`, resolve first scene from `chapter.meta.json`, open that scene, place cursor at document start.

---

## 21. Save Scene API

### 21.1 Request

```cpp
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
    AuthorshipRef  author;
    std::optional<std::string> previouslyLoadedContentHash;
};
```

`selection` and `scroll` default to zero in the current Swift call site; they will be wired when the SwiftUI editor is implemented.

### 21.2 Result

```cpp
struct SaveSceneResult {
    SceneID     sceneID;
    bool        saved                   = false;
    bool        metadataUpdated         = false;
    bool        workspaceStateUpdated   = false;
    std::size_t wordCount               = 0;
    std::size_t characterCount          = 0;
    bool        hasUnsnapshottedChanges = false;
    std::vector<RepairIssue> repairIssues;
};
```

### 21.3 Behavior

Safe-write Markdown, update `modifiedAt`/`modifiedBy`, update derived stats, update `workspace-state.json`, mark app-local indexes dirty, return conflict issue if disk changed unexpectedly.

---

## 22. External Change Scan API

### 22.1 Request

```cpp
struct ExternalChangeScanRequest {
    AbsolutePath projectRootPath;
    AbsolutePath appSupportRoot;
    bool         includeGitStatus = true;
};
```

### 22.2 Result

```cpp
struct ExternalChangeScanResult {
    ProjectID projectID;
    std::vector<RepairIssue> repairIssues;
    bool indexesDirty            = false;
    bool gitStatusChecked        = false;
    bool hasUnsnapshottedChanges = false;
};
```

### 22.3 MVP detection scope

Must detect: `scene.md` changed externally, `scene.md` missing, `scene.meta.json` missing, `scene.meta.json` corrupt, workspace last scene invalid, new unregistered `.md` in manuscript folder.

---

## 23. Enable Git Snapshots API

### 23.1 Request

```cpp
struct EnableGitRequest {
    AbsolutePath  projectRootPath;
    AuthorshipRef author;
    std::string   initialSnapshotLabel = "Initial project";
};
```

### 23.2 Result

```cpp
struct EnableGitResult {
    bool       gitInitialized    = false;
    bool       alreadyRepository = false;
    SnapshotID initialSnapshotID;
    CommitID   initialCommitID;
    std::vector<RepairIssue> warnings;
};
```

---

## 24. Create Snapshot API

### 24.1 Request

```cpp
struct CreateSnapshotRequest {
    AbsolutePath  projectRootPath;
    AuthorshipRef author;
    std::string   label;
    std::string   note;
};
```

### 24.2 Result

```cpp
struct CreateSnapshotResult {
    SnapshotID       snapshotID;
    CommitID         commitID;
    ISO8601Timestamp createdAt;
    bool             created = false;
};
```

---

## 25. Boundary Protocol: JSON-over-`std::string` (Permanent)

### 25.1 Direction

**JSON-over-`std::string` is the primary and permanent boundary protocol for all platform wrappers.**

This was the conclusion of `Scrivi_Swift_Interop_Trade_Study_v0_1.md` and is formalized in `Scrivi_Architecture_v0_3.md`. It applies not just to Swift but to every future platform (Android/JNI, Windows, Linux tooling, scripting).

This decision supersedes §27 of the v0.2 API sketch, which designated direct Swift/C++ struct interop as the primary Apple strategy with JSON as a possible C ABI option. That is reversed.

### 25.2 Why this is permanent

The pass-by-value principle (Architecture v0.3 §3) requires that all data transmission between layers produces an independent, owned copy in the receiver. `std::string`-based JSON is the only mechanism that safely transfers variable-length data (including Markdown scene content of arbitrary length) across a language boundary without shared memory ownership.

Fixed char arrays fail for variable-length content. Raw `const char*` return values fail because Swift cannot guarantee the lifetime of C++ heap memory after the call returns. `std::string` returned by value is safe: Swift receives a copy; C++ can free its storage immediately.

### 25.3 `ScriviCoreAdapter` is the canonical boundary

The `ScriviCoreAdapter` (`scrivi::apple::ScriviAdapter`) is not a temporary shim. It is a permanent, load-bearing layer with a well-defined contract:

- **Inputs:** `const char*` parameters. Universally safe. `withCString` in Swift guarantees lifetime within the call.
- **Outputs:** `std::string` JSON, returned by value. No raw pointers returned.
- **Header depends only on** `<string>`, `<memory>`, `<atomic>`. No ScriviCore types.
- **Service wiring** is owned by `Impl` inside the adapter `.cpp`. Not configurable from Swift.

### 25.4 Envelope contract

Every adapter method returns:

```json
// success
{ "ok": true, "result": { ... } }

// failure
{ "ok": false, "error": { "code": 7, "message": "human-readable message" } }
```

`code` maps to `scrivi::ErrorCode` cast to `int`. Platform wrappers check `ok` first and use `code` to distinguish error types — not message strings.

### 25.5 Result JSON schemas

The JSON in the `result` field aligns with the schema tables in `Scrivi_Architecture_v0_3.md` §7. Field names match on-disk schemas where the data overlaps.

| Method | Key result fields |
|---|---|
| `ensureLocalIdentity` | `identityID`, `defaultPersonaID`, `displayName`, `createdNewIdentity` |
| `createProject` | `projectID`, `firstScene.{sceneID, metadataPath, contentPath}` |
| `openProject` | `projectID`, `activeScene.{sceneID, metadataPath, contentPath, markdown}` |
| `saveScene` | `sceneID`, `saved`, `wordCount` |

`activeScene.markdown` carries the full scene Markdown as a JSON string value. There is no `getLastOpenMarkdown()` method, no buffer, no separate call.

### 25.6 Future platforms

Android/JNI, Windows, and Linux wrappers will use the same adapter pattern with the same JSON envelope. The C++ side is identical; only the platform wrapper code differs. A C ABI variant may be added if needed for non-C++ foreign function interfaces, but the JSON envelope contract is the same.

---

## 26. Swift Wrapper Responsibilities

`ScriviEngine.swift`:

- Holds `ScriviAdapter*` as an ARC-managed reference type (retain/release hooks).
- Converts `String` parameters to `const char*` via `withCString`.
- Calls adapter methods, receives `std::string` by value.
- Converts to Swift `String` via `String(cxxString)`.
- Decodes the JSON envelope; throws `ScriviError` on `ok == false`.
- Returns decoded result structs (`Decodable`, `Sendable`) to callers.

Swift does not implement any backend logic. `ScriviEngine` is pure type conversion and decoding.

---

## 27. JSON Strategy

`nlohmann/json` is used for schema I/O within ScriviCore and for result serialization in `ScriviCoreAdapter`.

`nlohmann/json` types must not appear in:
- Public ScriviCore headers (`include/scrivi/`)
- `ScriviCoreAdapter.hpp`

It is confined to:
- Schema reader/writer implementations (`src/schemas/`)
- `src/util/Json.hpp/cpp`
- `ScriviCoreAdapter.cpp` (for result serialization)

This preserves the option to replace the JSON implementation without breaking any public interface.

---

## 28. Build and Test Strategy

**ScriviCore:** CMake 3.24+. `cxx_std_23`. Catch2 v3.6.0 via FetchContent, test-only.

**Apple wrapper:** SPM package at `platforms/apple/`. Swift Testing for wrapper tests. `swift test` runs the interop tests.

Catch2 must not be linked into `ScriviCore` or the adapter. Production app targets link `ScriviCore` only (via the adapter library or directly).

---

## 29. API Stability

This is v0.3. The API will continue evolving during implementation. The following are stable unless architecture changes:

```text
coarse operation boundary (seven facade methods)
project-local Result<T>
structured Error model with ErrorCode
RepairIssue model
platform service injection via CoreServices
SecureStore abstraction
GitProvider abstraction
appSupportRoot MVP path strategy
JSON-over-std::string as the permanent boundary protocol
ScriviCoreAdapter as the canonical C++/platform boundary
UTF-8 wrapper/project paths
no UI dependencies in ScriviCore or ScriviCoreAdapter
project package neutrality
```

---

## 30. MVP Acceptance Criteria

The API is successful if it supports:

1. First-launch identity creation.
2. Project creation.
3. Project opening.
4. Resume last writing surface.
5. Save scene.
6. Workspace state update.
7. Missing `.md` detection.
8. Missing `.meta.json` detection.
9. Corrupt metadata detection.
10. External `.md` edit detection.
11. Optional Git initialization via `SystemGitProvider`.
12. Optional snapshot creation.
13. Swift wrapper using `ScriviCoreAdapter` and JSON-over-`std::string`.
14. Platform-neutral C++ tests via Catch2.
15. No production dependency on test framework.
16. No private identity material in `.scrivi` project packages.
17. Full scene Markdown crossing the C++/Swift boundary safely by value (in JSON).

---

## 31. Open Questions

~~1. **`JsonDoc` double support.**~~ **Resolved — T-0048 (EP-006).** `setDouble`/`getDouble` added to `JsonDoc`.

2. **`deviceID` in workspace state.** Currently hardcoded `"device-local"`. A real stable device identifier is needed before workspace state sync is designed. Deferred — not in EP-008 scope.

~~3. **`KeychainSecureStore` sprint timing.**~~ **Resolved — T-0049 (EP-006).** `KeychainSecureStore` implemented on Apple. Non-Apple platform strategy addressed in T-0058 (EP-008 / SP-018).

~~4. **Cursor and scroll wiring from Swift.**~~ **Resolved — T-0053 (EP-006).** Live cursor offset wired. Scroll at `0.0` — acceptable pending SwiftUI scroll API.

5. **Multi-scene `openProject` result.** Current result returns one `activeScene`. Addressed in T-0059/T-0060 (EP-008 / SP-019).

6. **Repair action operations.** No facade methods for applying repair actions yet. Deferred — not in EP-008 scope.

~~7. **`appSupportRoot` on non-Apple platforms.**~~ **Resolved — 2026-05-30.** `AppSupportLayout::platformDefault()` resolves the OS-appropriate root at the C++ layer. Linux: `$XDG_DATA_HOME/Scrivi` or `~/.local/share/Scrivi`. Windows: `%APPDATA%\Scrivi`. Apple: unchanged. Implemented in T-0057 (EP-008 / SP-018).
