# Task T-0006: Project Creation

**Status:** ✅ Implemented - Verified
**Component:** ScriviCore (C++ backend)
**Priority:** High
**Epic:** EP-002: ScriviCore Services
**Date Requested:** 2026-05-19
**Date Implemented:** 2026-05-20
**Date Verified:** 2026-05-20
**Sprint Assigned:** SP-002

**Rationale:**
Project creation is the entry point for the entire writer workflow. Without it, no project exists to open, save, or scan.

**Current Behavior:**
No project creation logic existed.

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
Implemented `ProjectCreator` in `ScriviCore/src/project_package/`. `ScriviCore::createProject()` in `src/public_api/ScriviCore.cpp` delegates to `ProjectCreator` with injected services. All file I/O through `FileSystem` service. All ID generation through `UUIDProvider`. All timestamps through `Clock`.

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
