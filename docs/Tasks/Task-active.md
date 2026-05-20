# Active Tasks

Tasks listed here are assigned to an active Sprint and currently in progress.

---

## T-0006: Project Creation

**Status:** 🟡 Implemented - Not Verified
**Component:** ScriviCore (C++ backend)
**Priority:** High
**Epic:** EP-002: ScriviCore Services
**Date Requested:** 2026-05-19
**Date Implemented:** 2026-05-20
**Date Verified:** —
**Sprint Assigned:** SP-002

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
- `src/project_package/ProjectCreator.hpp/.cpp` — implements full creation sequence
- `ScriviCore::createProject()` delegates to `ProjectCreator::create()`
- Validates `AuthorshipRef` (non-empty identityID and personaID) before any I/O; returns `invalidArgument` if invalid
- Writes all 7 required files in sequence: `project.json`, `manuscript.meta.json`, `chapter.meta.json`, `scene.md` (empty), `scene.meta.json`, `project-members.json`, `project-personas.json`
- App-local workspace state written to `appSupportRoot/state/projects/<projectID>/workspace-state.json`
- Git path: `initRepository` → write `.gitignore` → write `snapshots/scrivi-snapshots.json` → `addAll` → `commit`
- Git author email mapped as `<identityID>@scrivi.author` per spec
- Any I/O failure propagates immediately; no partial cleanup attempted at this stage

**Test Steps:**
1. `ctest --test-dir build --output-on-failure` — 58/58 tests pass including:
2. Test 53: minimum package structure — all 7 files exist, no Git files
3. Test 54: project.json content contains title, slug, identity, schema tag
4. Test 55: scene.meta.json round-trips through `parseSceneMeta()`
5. Test 56: workspace-state.json written under `appSupportRoot`, references first scene
6. Test 57: Git path — init/addAll/commit called in order, .gitignore and snapshots/ written
7. Test 58: empty identityID returns `invalidArgument`

**Notes:**
Uses `LocalFileSystem`, `DeterministicUUIDProvider`, `FixedClock`, and `MockGitProvider` from T-0004.

---

## T-0007: Open/Resume

**Status:** 🟡 Implemented - Not Verified
**Component:** ScriviCore (C++ backend)
**Priority:** High
**Epic:** EP-002: ScriviCore Services
**Date Requested:** 2026-05-19
**Date Implemented:** 2026-05-20
**Date Verified:** —
**Sprint Assigned:** SP-002

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
- `src/project_package/ProjectValidator.hpp/.cpp` — walks package structure, detects missingMetadata, corruptMetadata, missingContent per scene
- `src/project_package/ProjectOpener.hpp/.cpp` — delegates to ProjectValidator, ManuscriptOrderResolver, WorkspaceStateService (read), SceneReader; selects active scene
- `src/manuscript/ManuscriptOrderResolver.hpp/.cpp` — reads manuscript.meta.json → chapter.meta.json → scene.meta.json chain; returns scenes in order
- `src/manuscript/SceneReader.hpp/.cpp` — reads content file at relative path from project root
- `src/workspace/WorkspaceStateService.hpp/.cpp` — load/save workspace state at `appSupportRoot/state/projects/<id>/workspace-state.json`
- `ScriviCore::openProject()` delegates to `ProjectOpener::open()`
- Blocking repair issues → `OpenMode::repairRequired`; no issues → `OpenMode::normalEdit`; non-blocking warnings → `OpenMode::editWithWarnings`
- Workspace state missing → falls back to first scene, zero selection/scroll
- Workspace state present → sceneID looked up in ordered scene list; selection and scroll restored

**Test Steps:**
1. `ctest --test-dir build --output-on-failure` — 70/70 tests pass including:
2. Test 59: normalEdit mode, correct projectID, zero repairIssues
3. Test 60: restores selection {5,10} and scroll 0.42 after saveScene + reopen
4. Test 61: fresh appSupportRoot → falls back to first scene, zero selection/scroll
5. Test 62: deleted scene.meta.json → repairRequired, missingMetadata issue
6. Test 63: deleted scene.md → repairRequired, missingContent issue
7. Test 64: corrupt scene.meta.json → repairRequired, corruptMetadata issue
8. Test 65: known content written directly → activeSceneMarkdown matches

**Notes:**
Error-path tests exercise the structural damage scenarios in-place (delete/corrupt on disk) rather than using checked-in fixtures. Fixtures approach deferred until T-0009 requires them.

---

## T-0008: Save Scene

**Status:** 🟡 Implemented - Not Verified
**Component:** ScriviCore (C++ backend)
**Priority:** High
**Epic:** EP-002: ScriviCore Services
**Date Requested:** 2026-05-19
**Date Implemented:** 2026-05-20
**Date Verified:** —
**Sprint Assigned:** SP-002

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
- `src/manuscript/SceneWriter.hpp/.cpp` — atomically writes content, reads+updates scene meta, updates workspace state
- `src/workspace/WorkspaceStateService.hpp/.cpp` — write path shared with T-0007
- `ScriviCore::saveScene()` delegates to `SceneWriter::save()`
- Idempotent: computes `sha256Hex(markdown)`, skips content write if hash matches `previouslyLoadedContentHash`; workspace state always updated
- `countText(markdown)` provides wordCount/characterCount for metadata and result
- `atomicWriteTextFile` (FileSystem service) guarantees no partial writes

**Test Steps:**
1. `ctest --test-dir build --output-on-failure` — 70/70 tests pass including:
2. Test 66: content written to disk reads back identically
3. Test 67: modifiedAt, modifiedByIdentityID, modifiedByDisplayName updated in metadata
4. Test 68: wordCount=10 for 10-word sentence, stored in metadata
5. Test 69: openProject after saveScene restores selection {3,7} and scroll 0.75
6. Test 70: same hash → saved=false, workspaceStateUpdated=true

**Notes:**
AtomicWrite (write to temp, then rename) ensures no partial writes on crash. See T-0003.

---

## T-0009: External Change Scan

**Status:** 🟡 Active
**Component:** ScriviCore (C++ backend)
**Priority:** High
**Epic:** EP-002: ScriviCore Services
**Date Requested:** 2026-05-19
**Date Implemented:** —
**Date Verified:** —
**Sprint Assigned:** SP-002

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

**Status:** 🟡 Active
**Component:** ScriviCore (C++ backend)
**Priority:** High
**Epic:** EP-002: ScriviCore Services
**Date Requested:** 2026-05-19
**Date Implemented:** —
**Date Verified:** —
**Sprint Assigned:** SP-002

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

*Last Updated: 2026-05-20*
