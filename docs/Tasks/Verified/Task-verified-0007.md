# Task T-0007: Open/Resume

**Status:** ✅ Implemented - Verified
**Component:** ScriviCore (C++ backend)
**Priority:** High
**Epic:** EP-002: ScriviCore Services
**Date Requested:** 2026-05-19
**Date Implemented:** 2026-05-20
**Date Verified:** 2026-05-20
**Sprint Assigned:** SP-002

**Rationale:**
Writers must be able to return to their project and resume exactly where they left off. This is the core daily-use workflow.

**Current Behavior:**
No open/resume logic existed.

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
Implemented in `ScriviCore/src/project_package/` and `ScriviCore/src/manuscript/`. `ProjectOpener` calls `ProjectValidator` to detect repair issues before opening. `OpenMode` reflects the project health state.

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
Error-path tests exercise structural damage scenarios in-place (delete/corrupt on disk) rather than using checked-in fixtures.
