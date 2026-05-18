# Scrivi C++24 Backend Core Plan v0.1

**Project:** Scrivi  
**Document:** C++24 Backend Core Plan  
**Version:** 0.1  
**Status:** Draft / Approved Direction Pending Review  
**Date:** 2026-05-18

---

## 1. Purpose

This document defines the initial implementation plan for the Scrivi C++24 shared backend core.

It translates the approved runtime decision into a concrete backend structure, API boundary, module plan, and MVP implementation sequence.

The core mission is:

> Build the smallest C++24 backend that proves Scrivi can create a project, open it, resume writing, save writing, detect simple external changes, and optionally create Git-backed snapshots.

---

## 2. Approved Runtime Direction

Scrivi will use:

```text
C++24 shared backend core from the start
```

with thin platform wrappers and native/platform-appropriate UI shells.

The C++ core must remain UI-independent.

---

## 3. MVP Backend Responsibilities

The C++24 backend core owns:

```text
project package creation
schema read/write
Markdown file persistence
metadata synchronization
workspace state logic
external change scanning
repair classification
Git snapshot abstraction
identity/persona project metadata
app-local cache/index policy
```

The C++24 backend core does not own:

```text
SwiftUI/AppKit/UIKit/visionOS UI
Windows UI
Linux UI
Android UI
rich text editor presentation
visual design
platform-specific navigation
platform-specific secure storage implementation
remote Git hosting
real-time collaboration
```

---

## 4. Guardrails

### 4.1 Coarse API boundary

The backend should expose coarse operations:

```text
createProject()
openProject()
saveScene()
scanForExternalChanges()
enableGitSnapshots()
createSnapshot()
```

Avoid chatty UI-driven APIs across language boundaries.

---

### 4.2 No UI dependencies

The core must not include UI framework headers or types.

Forbidden in core:

```text
SwiftUI
AppKit
UIKit
RealityKit
WinUI
Qt widgets
GTK widgets
Android Views
Jetpack Compose
```

---

### 4.3 Platform services are injected

The core interacts with platform-specific facilities through interfaces.

Examples:

```text
SecureStore
FileSystem
Clock
GitProvider
UUIDProvider
Logger
```

---

### 4.4 Canonical formats remain platform-neutral

The core writes:

```text
Markdown
JSON
native asset references
```

It must not use platform-specific canonical formats.

---

### 4.5 Minimum implementation only

The MVP must not implement speculative systems:

```text
complete object graph
timeline engine
query language
plugin system
custom object database
real-time collaboration
project encryption
full signing validation
remote Git
```

---

## 5. Proposed Repository / Source Layout

Initial source layout:

```text
ScriviCore/
  CMakeLists.txt

  include/
    scrivi/
      ScriviCore.hpp
      Result.hpp
      Types.hpp

  src/
    public_api/
      ScriviCore.cpp

    domain/
      IDs.hpp
      ProjectTypes.hpp
      IdentityTypes.hpp
      ManuscriptTypes.hpp
      WorkspaceTypes.hpp

    schemas/
      ProjectJson.hpp
      ManuscriptMetaJson.hpp
      ChapterMetaJson.hpp
      SceneMetaJson.hpp
      ProjectMembersJson.hpp
      ProjectPersonasJson.hpp
      WorkspaceStateJson.hpp

    project_package/
      ProjectCreator.hpp
      ProjectOpener.hpp
      ProjectValidator.hpp
      PackagePaths.hpp

    manuscript/
      SceneReader.hpp
      SceneWriter.hpp
      ManuscriptOrderResolver.hpp

    identity/
      IdentityService.hpp
      PersonaService.hpp

    workspace/
      WorkspaceStateService.hpp

    repair/
      ExternalChangeScanner.hpp
      RepairIssue.hpp
      RepairClassifier.hpp

    git/
      GitProvider.hpp
      SystemGitProvider.hpp
      MockGitProvider.hpp
      SnapshotService.hpp

    platform/
      FileSystem.hpp
      LocalFileSystem.hpp
      SecureStore.hpp
      Clock.hpp
      SystemClock.hpp
      UUIDProvider.hpp
      Logger.hpp

    util/
      Json.hpp
      AtomicWrite.hpp
      PathUtils.hpp
      TextStats.hpp

  tests/
    fixtures/
    unit/
    integration/
```

This layout is a starting point, not a final architecture.

---

## 6. Public API Shape

The C++ API should be coarse and stable.

Illustrative facade:

```cpp
namespace scrivi {

class ScriviCore {
public:
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

The exact type names are not final. The important decision is that calls are workflow-level, not field-level.

---

## 7. Wrapper Boundary Strategy

### 7.1 Native C++ API

Used by:

```text
C++ tests
Windows/Linux native shells if appropriate
command-line test harness
```

### 7.2 C ABI layer

A C ABI layer may be needed for stable cross-language bindings.

Possible approach:

```text
scrivi_create_project(json_request) -> json_result
scrivi_open_project(json_request) -> json_result
scrivi_save_scene(json_request) -> json_result
```

This is not as type-rich as a native API, but it greatly simplifies early wrappers.

### 7.3 Swift wrapper

Swift wrapper responsibilities:

```text
convert Swift request models to C++/C ABI requests
call ScriviCore
convert results to Swift models
map repair issues to UI state
provide platform SecureStore implementation
provide platform app-local paths
```

The Swift wrapper must not reimplement backend behavior.

### 7.4 Android wrapper

Android wrapper likely uses JNI/NDK.

MVP Android is not first implementation, but the C++ core should avoid choices that make JNI impossible.

### 7.5 Windows/Linux wrapper

Windows/Linux shells may call the C++ core directly or through a C ABI, depending on chosen UI technology later.

---

## 8. Core Data Model

### 8.1 IDs

Use UUID v7-style opaque IDs with type prefixes.

Examples:

```text
project_01J8Z7V6B5W4Q3R2N1M0K9J8H7
manuscript_01J8Z8D4C3B2A1N0M9K8J7H6G5
chapter_01J8Z8H9K7M6N5B4V3C2X1Z0A9
scene_01J8Z8P4N3M2K1J0H9G8F7E6D5
identity_01J8Z7Y9R6T5E4W3Q2A1S0D9F8
persona_01J8Z801V6B5N4M3K2J1H0G9F8
```

The ID provider should be abstracted:

```cpp
struct UUIDProvider {
    virtual ProjectID newProjectID() = 0;
    virtual SceneID newSceneID() = 0;
    // ...
};
```

MVP may implement a simple UUID v7-style generator internally or use a small dependency.

### 8.2 Paths

Paths inside project metadata should be relative where approved.

The C++ core should normalize paths internally but preserve project-friendly path strings in JSON.

### 8.3 Authorship references

Use identity/persona references.

Required fields include:

```text
identityID
personaID
displayNameAtCreation
displayNameAtModification
```

### 8.4 Status values

Initial scene/chapter status values:

```text
draft
revised
final
archived
```

Do not add more statuses in MVP.

---

## 9. JSON Strategy

The MVP requires JSON read/write for:

```text
project.json
manuscript.meta.json
chapter.meta.json
scene.meta.json
project-members.json
project-personas.json
workspace-state.json
scrivi-snapshots.json
```

### 9.1 Requirements

JSON support must provide:

```text
UTF-8 support
stable serialization
good error reporting
object/array/string/number/bool/null support
round-trip enough for known schemas
```

### 9.2 Library choice

The exact JSON library is not locked in by this document.

Candidate approaches:

```text
nlohmann/json
Boost.JSON
simdjson for parsing plus custom serialization
custom minimal JSON layer
```

### 9.3 Recommendation

For MVP, choose the simplest maintainable option that supports clear schema read/write and good errors.

Likely recommendation:

```text
nlohmann/json for MVP
```

Reason:

```text
fast to implement
widely used
simple API
adequate for project metadata scale
```

Possible later migration is acceptable if wrapped behind `util/Json.hpp`.

### 9.4 Guardrail

Do not expose the third-party JSON library throughout the entire core.

Wrap it behind schema-specific loaders/writers or a narrow JSON utility layer.

---

## 10. Filesystem and Atomic Writes

### 10.1 Filesystem responsibilities

The core needs to:

```text
create directories
write files
read files
test existence
list folders
detect missing files
resolve relative paths
perform safe writes
```

### 10.2 Standard library

Use C++ standard filesystem facilities where sufficient:

```cpp
std::filesystem
```

### 10.3 Atomic write utility

Implement:

```text
write temporary file
flush/sync where practical
atomic rename/replace
preserve UTF-8
avoid partial target writes
```

API concept:

```cpp
Result<void> atomicWriteTextFile(
    const Path& path,
    std::string_view utf8Text);
```

### 10.4 Platform differences

Atomic replace behavior differs by platform. Hide differences behind `FileSystem`.

---

## 11. Workspace State

Workspace state is app-local, not canonical project data.

The core should support reading/writing workspace state given app-local paths supplied by the platform wrapper.

Example:

```text
App Support/
  state/
    projects/
      <projectID>/
        workspace-state.json
```

The core should not hardcode platform app support directories.

The wrapper/platform layer supplies those locations.

---

## 12. Identity and Secure Storage

### 12.1 MVP identity scope

MVP requires:

```text
local identity ID
default persona ID
project membership metadata
project persona metadata
public key placeholder or certificate-ready metadata
private material stored outside project package
```

### 12.2 SecureStore abstraction

The C++ core should use an interface:

```cpp
class SecureStore {
public:
    virtual Result<void> putSecret(std::string_view key, SecretBytes value) = 0;
    virtual Result<SecretBytes> getSecret(std::string_view key) = 0;
    virtual Result<bool> containsSecret(std::string_view key) = 0;
};
```

### 12.3 Platform implementations

Later wrappers provide:

```text
Apple Keychain implementation
Windows Credential Manager / DPAPI implementation
Linux Secret Service / fallback implementation
Android Keystore implementation
MockSecureStore for tests
```

### 12.4 MVP simplification

If full secure store integration is not implemented immediately, the MVP may use a test/mock implementation for command-line tests.

However, real app builds must not store private keys inside `.scrivi` project packages.

---

## 13. Git Abstraction

### 13.1 GitProvider interface

Git support should be abstracted.

Concept:

```cpp
class GitProvider {
public:
    virtual Result<bool> isRepository(const Path& root) = 0;
    virtual Result<void> initRepository(const Path& root) = 0;
    virtual Result<void> addAll(const Path& root) = 0;
    virtual Result<CommitID> commit(
        const Path& root,
        const CommitRequest& request) = 0;
    virtual Result<GitStatus> status(const Path& root) = 0;
};
```

### 13.2 Initial implementation

MVP can use:

```text
SystemGitProvider
```

which invokes the system `git` executable.

### 13.3 Why system Git first

Pros:

```text
minimum code
matches writer/power-user expectations
works with real Git repositories
avoids embedding libgit2 early
```

Cons:

```text
requires Git installed
command-line process handling
platform path/quoting issues
```

### 13.4 Guardrail

The rest of ScriviCore should not know whether Git is system Git or a library.

### 13.5 Mock provider

Tests should use:

```text
MockGitProvider
```

where possible.

Integration tests may use real system Git when available.

---

## 14. Error and Result Model

### 14.1 Result type

Use explicit results.

Concept:

```cpp
template <typename T>
class Result {
public:
    bool ok() const;
    const T& value() const;
    const Error& error() const;
};
```

Or use an equivalent expected-style type.

C++23 includes `std::expected`; if available in the chosen toolchain, consider using it. If not consistently available, provide a small project-local equivalent.

### 14.2 Error categories

Minimum categories:

```text
ok
validationWarning
repairRequired
unsupportedVersion
ioError
parseError
permissionDenied
gitError
secureStoreError
invalidArgument
internalError
```

### 14.3 Repair issues

Repair issues are not generic errors.

They should be structured:

```text
issueID
severity
category
path
relatedObjectID
message
suggestedActions
```

Severity:

```text
info
warning
blocking
```

### 14.4 Wrapper mapping

The Swift/Android/UI wrapper maps structured repair issues to UI.

The C++ core does not present dialogs.

---

## 15. External Change Scanner

### 15.1 MVP scope

Must detect:

```text
scene.md changed externally
scene.md missing
scene.meta.json missing
scene.meta.json corrupt/unreadable
workspace last scene invalid
new unregistered .md in manuscript folder
```

May detect:

```text
possible rename
Git checkout changed open scene
unknown files
```

### 15.2 Output

The scanner returns:

```text
ExternalChangeScanResult
  repairIssues[]
  dirtyIndexes[]
  safeReloads[]
```

### 15.3 No destructive actions

The scanner does not repair by itself.

It classifies and reports.

Repair actions are separate operations.

---

## 16. Project Creation Service

### 16.1 Responsibilities

`ProjectCreator` creates the minimum package:

```text
project.json
manuscript/manuscript.meta.json
chapter.meta.json
scene.md
scene.meta.json
project-members.json
project-personas.json
workspace-state.json
```

### 16.2 Creation behavior

Steps:

```text
generate IDs
derive slugs/filenames
create folders
write JSON metadata
write empty Markdown scene
write app-local workspace state
optionally initialize Git
optionally create initial snapshot
```

### 16.3 Failure behavior

If creation fails midway:

```text
clean up files created by Scrivi where safe
do not delete pre-existing user files
return structured error
```

---

## 17. Project Opening Service

### 17.1 Responsibilities

`ProjectOpener` handles:

```text
read project.json
validate basic version
read identities/personas
read workspace state
resolve last scene
fallback to first scene
return editor-ready scene
start/report background validation state
```

### 17.2 Output

`OpenProjectResult` should include:

```text
project summary
active scene ID
active scene title
Markdown content
cursor/selection/scroll state
active identity/persona
repair issues
open mode
```

Open modes:

```text
normalEdit
editWithWarnings
repairRequired
readOnly
cannotOpen
```

---

## 18. Scene Saving Service

### 18.1 Responsibilities

`SceneWriter` handles:

```text
safe write Markdown
update modifiedAt
update modifiedBy
update stats if enabled
write metadata
update workspace state
mark cache dirty
```

### 18.2 External conflict behavior

If file changed externally since loaded:

```text
do not overwrite silently
return repair/conflict issue
preserve both versions where possible
```

MVP may detect this using timestamp/hash captured at load time.

---

## 19. App-Local Cache MVP

The MVP should not implement SQLite.

Minimum cache behavior:

```text
workspace state
dirty flags
optional simple validation cache
```

Cache must be:

```text
app-local
deletable
rebuildable
not canonical
```

No full-text search in MVP.

---

## 20. Dependencies

### 20.1 Keep dependency set small

Initial dependencies should be minimal.

Possible MVP dependencies:

```text
JSON library
UUID v7/ULID helper or small internal generator
test framework
optional process execution helper for system Git
```

Avoid large frameworks until needed.

### 20.2 Candidate dependencies

Possible choices:

```text
nlohmann/json
Catch2 or GoogleTest
fmt or std::format
```

The exact list should be approved before implementation.

### 20.3 Dependency guardrail

Do not let dependency APIs leak through the public ScriviCore API.

---

## 21. Build System

### 21.1 Recommendation

Use CMake for the C++ core.

Reasons:

```text
cross-platform
works with Apple, Windows, Linux, Android/NDK workflows
supports tests
familiar integration path for many IDEs/build systems
```

### 21.2 Alternative

If the project ecosystem strongly prefers another build system, this can be revisited.

But for the shared core, CMake is the default recommendation.

### 21.3 Build targets

Initial targets:

```text
ScriviCore static library
ScriviCore unit tests
ScriviCore integration tests
optional command-line test harness
```

Later targets:

```text
Swift wrapper
Android JNI wrapper
Windows/Linux app wrapper
```

---

## 22. Testing Strategy

### 22.1 Unit tests

Cover:

```text
ID generation
slug generation
JSON load/write
path resolution
atomic write
workspace state
error mapping
repair classifier
```

### 22.2 Integration tests

Use fixture projects.

Test:

```text
create project
open project
save scene
reopen project
external edit detection
missing Markdown detection
missing metadata detection
corrupt metadata detection
Git init/snapshot if system Git available
```

### 22.3 Golden fixtures

Maintain minimal `.scrivi` project fixtures:

```text
fixtures/minimal-valid/
fixtures/missing-scene-md/
fixtures/missing-scene-meta/
fixtures/corrupt-scene-meta/
fixtures/external-edit/
```

### 22.4 Cross-platform goal

The same fixtures should run on:

```text
macOS
Windows
Linux
```

Android fixture testing can come later.

---

## 23. C++ Public Type Style

### 23.1 Prefer simple value types

Use:

```text
std::string
std::vector
std::optional
std::filesystem::path internally
strong ID wrapper types where useful
```

### 23.2 Avoid over-modeling

Do not create a grand class hierarchy for project objects.

Use simple structs and services for MVP.

### 23.3 Example request type

```cpp
struct CreateProjectRequest {
    Path projectRoot;
    std::string title;
    IdentityID identityID;
    PersonaID personaID;
    std::string personaDisplayName;
    bool enableGitSnapshots = false;
};
```

### 23.4 Example result type

```cpp
struct CreateProjectResult {
    ProjectID projectID;
    SceneID firstSceneID;
    Path firstScenePath;
    std::vector<RepairIssue> warnings;
};
```

---

## 24. Suggested Implementation Milestones

### Milestone 1 — Core skeleton

```text
CMake project
ScriviCore library
test target
Result/Error types
basic path utilities
JSON utility wrapper
```

### Milestone 2 — IDs and schemas

```text
UUID v7-style ID provider
schema structs
project.json writer/reader
manuscript.meta writer/reader
chapter.meta writer/reader
scene.meta writer/reader
identity/persona metadata writer/reader
workspace-state writer/reader
```

### Milestone 3 — Project creation

```text
create minimum package
write empty scene
write metadata
write workspace state
test createProject()
```

### Milestone 4 — Open/resume

```text
open project.json
resolve workspace state
open last scene
fallback to first scene
return editor-ready result
```

### Milestone 5 — Save scene

```text
safe write Markdown
update modifiedAt/modifiedBy
update stats
update workspace state
detect changed-on-disk conflict
```

### Milestone 6 — External change scanner

```text
detect missing .md
detect missing .meta.json
detect corrupt metadata
detect unregistered .md
return repair issues
```

### Milestone 7 — Git provider

```text
GitProvider interface
SystemGitProvider
.gitignore writer
enable Git at creation
enable Git later
create snapshot
snapshot metadata
```

### Milestone 8 — Swift wrapper prototype

```text
Swift calls createProject
Swift calls openProject
Swift calls saveScene
Swift receives repair issues
```

---

## 25. MVP Acceptance Tests

The C++ core should pass the acceptance tests from the MVP slice.

Minimum test set:

```text
first launch identity test
create project test
save and reopen test
workspace fallback test
external edit test
missing Markdown test
missing metadata test
corrupt metadata test
Git at creation test
Git enable later test
snapshot test
```

---

## 26. Risks

### 26.1 Wrapper complexity

C++ core plus Swift/Android wrappers adds upfront complexity.

Mitigation:

```text
coarse API
C ABI or JSON request/result boundary for early wrappers
avoid chatty calls
```

### 26.2 Dependency sprawl

C++ projects can accumulate heavy dependencies.

Mitigation:

```text
approve dependencies explicitly
wrap dependencies
keep MVP dependency set small
```

### 26.3 Memory/ownership bugs

C++ requires discipline.

Mitigation:

```text
value types
RAII
smart pointers only where needed
no raw owning pointers
sanitizer builds
tests
explicit Result errors
```

### 26.4 Platform filesystem differences

Atomic writes and path behavior differ by OS.

Mitigation:

```text
FileSystem abstraction
platform tests
conservative path rules
relative project paths
```

### 26.5 Git availability

System Git may not be installed everywhere.

Mitigation:

```text
GitProvider abstraction
NoGitProvider fallback
clear user-facing message
optional Git feature
```

### 26.6 Secure storage integration

Secure storage differs by platform.

Mitigation:

```text
SecureStore abstraction
mock implementation for core tests
platform implementation outside core
```

---

## 27. Non-Goals for C++ Core v0.1

Do not implement:

```text
SQLite index
full-text search
asset metadata
comments
worldbuilding object schemas
relationship graph
timelines
exports
identity transfer
QR transfer
project encryption
signature verification
group personas
remote Git
branch UI
merge UI
real-time collaboration
```

---

## 28. Success Criteria

The C++24 Backend Core Plan is successful if:

1. The backend core can be implemented without UI dependencies.
2. The MVP writing loop is clearly achievable.
3. Platform-specific services are abstracted.
4. Project package files remain platform-neutral.
5. The public API is coarse and wrapper-friendly.
6. The dependency set is small and controlled.
7. The test plan validates real `.scrivi` fixtures.
8. Git support is optional and abstracted.
9. Secure storage remains outside project packages.
10. Future platform wrappers are not blocked.

---

## 29. Recommended Next Document

The next document should be:

```text
Scrivi C++24 Core API Sketch v0.1
```

It should define:

```text
request/result structs
error/result type
repair issue type
service interfaces
public facade
C ABI boundary option
initial test fixture expectations
```

After that, the project can proceed to an initial repository skeleton.
