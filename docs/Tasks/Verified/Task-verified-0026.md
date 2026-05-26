# Task-verified-0026

## T-0026: Add Swift Interop Tests for New Adapter Methods

**Status:** ✅ Implemented - Verified
**Component:** Apple platform wrapper (Swift) — test suite
**Priority:** Medium
**Epic:** EP-007: Skeleton v0.2 Migration
**Date Requested:** 2026-05-22
**Date Implemented:** 2026-05-26
**Date Verified:** 2026-05-26
**Sprint Assigned:** SP-006

**Rationale:**
The three new adapter methods added in T-0024/T-0025 needed end-to-end coverage to confirm the JSON envelope contract works at the Swift boundary.

**Current Behavior (before):**
`ScriviInteropTests.swift` had 5 tests for `ensureLocalIdentity`, `createProject`, `openProject`, `saveScene`, and error handling. No coverage for the three new methods.

**Desired Behavior (after):**
`ScriviInteropTests.swift` has 8 tests covering all 7 adapter methods.

**Requirements:**
1. Test `scanForExternalChanges` — fresh project, zero issues ✅
2. Test `enableGitSnapshots` — `gitInitialized == true`, non-empty snapshot/commit IDs ✅
3. Test `createSnapshot` — `created == true` ✅
4. All 8 total interop tests pass via `swift test` ✅

**Implementation Details:**
- Added `gitAvailable()` static helper (runs `git --version` via `Process`).
- Added `makeProjectFixture()` shared setup helper (creates identity, creates project, returns all handles).
- Test 6: `scanForExternalChanges` — fresh project, `includeGitStatus: false`, asserts `issueCount == 0`.
- Test 7: `enableGitSnapshots` — skip-guarded via `withKnownIssue` if no git; asserts `gitInitialized == true`, non-empty IDs.
- Test 8: `createSnapshot` — skip-guarded; enables git, saves scene with markdown, creates snapshot, asserts `created == true`.
- All 8 tests pass in 0.211 seconds on first run.

**Components Affected:**
- Modified: `platforms/apple/Tests/ScriviInteropTests/ScriviInteropTests.swift`

**Test Steps:**
1. `swift test` from `platforms/apple/` — all 8 tests pass ✅
