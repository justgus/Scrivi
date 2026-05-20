# Task T-0010: Git Snapshots

**Status:** ✅ Implemented - Verified
**Component:** ScriviCore (C++ backend)
**Priority:** High
**Epic:** EP-002: ScriviCore Services
**Date Requested:** 2026-05-19
**Date Implemented:** 2026-05-20
**Date Verified:** 2026-05-20
**Sprint Assigned:** SP-002

**Rationale:**
Git-backed snapshots give writers a safety net and version history without requiring them to understand Git. This is an opt-in feature that must not affect projects where it is disabled.

**Current Behavior:**
No Git snapshot logic existed.

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
- `src/util/Process.hpp/.cpp` — runs shell commands via `popen`, returns `ProcessResult{exitCode, stdout_}`; `executableInPath()` checks availability
- `src/git/SystemGitProvider.hpp/.cpp` — wraps git CLI via Process; all methods return `gitUnavailable` if git not in PATH
- `src/git/SnapshotService.hpp/.cpp` — implements `enable()` and `createSnapshot()`; reads/appends `snapshots/scrivi-snapshots.json` via `parseJson()`/`appendToArray()`
- `ScriviCore::enableGitSnapshots()` and `createSnapshot()` delegate to `SnapshotService`
- `SystemGitProvider::available()` is a static method tests use to skip gracefully

**Test Steps:**
1. `ctest --test-dir build --output-on-failure` — 82/82 tests pass including:
2. Test 77: mock — init/addAll/commit called in order, .gitignore written, snapshot metadata has one entry
3. Test 78: mock with repoExists=true — alreadyRepository=true, initRepository not called
4. Test 79: mock — createSnapshot appends second metadata entry with correct label/note
5. Test 80: normal project — createProject+openProject with MockGitProvider, zero git calls
6. Test 81: real git — .git directory created, initial commit hash is 40 chars
7. Test 82: real git — createSnapshot produces a second commit, metadata has two entries

**Notes:**
`Process` utility uses `popen` for cross-platform subprocess execution. Tests 81–82 skip with `SKIP()` if `SystemGitProvider::available()` returns false.
