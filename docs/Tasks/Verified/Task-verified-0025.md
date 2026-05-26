# Task-verified-0025

## T-0025: Add Swift Engine Methods for `scanForExternalChanges`, `enableGitSnapshots`, `createSnapshot`

**Status:** ✅ Implemented - Verified
**Component:** Apple platform wrapper (Swift)
**Priority:** High
**Epic:** EP-007: Skeleton v0.2 Migration
**Date Requested:** 2026-05-22
**Date Implemented:** 2026-05-26
**Date Verified:** 2026-05-26
**Sprint Assigned:** SP-006

**Rationale:**
After T-0024 added the three adapter methods, `ScriviEngine.swift` needed to expose them to Swift callers with proper type conversion, envelope decoding, and Swift result types.

**Current Behavior (before):**
`ScriviEngine.swift` had 4 methods. `scanForExternalChanges`, `enableGitSnapshots`, `createSnapshot` not accessible from Swift.

**Desired Behavior (after):**
`ScriviEngine.swift` has 7 public methods. Swift callers can call all three new methods and receive typed Swift result structs.

**Requirements:**
1. Three new methods added to `ScriviEngine` with `withCString` input conversion and `decode()` ✅
2. Three new `Decodable` Swift result structs: `ScanResult`, `EnableGitResult`, `CreateSnapshotResult` ✅
3. `swift build` succeeds ✅
4. `swift test` passes all tests ✅

**Implementation Details:**
- Added `scanForExternalChanges(projectRootPath:appSupportRoot:includeGitStatus:)` → `ScanResult`
- Added `enableGitSnapshots(projectRootPath:authorshipRef:initialSnapshotLabel:)` → `EnableGitResult`
- Added `createSnapshot(projectRootPath:authorshipRef:label:note:)` → `CreateSnapshotResult`
- `ScanResult`: `projectID`, `indexesDirty`, `gitStatusChecked`, `hasUnsnapshottedChanges`, `issueCount`
- `EnableGitResult`: `gitInitialized`, `alreadyRepository`, `initialSnapshotID`, `initialCommitID`
- `CreateSnapshotResult`: `snapshotID`, `commitID`, `createdAt`, `created`

**Components Affected:**
- Modified: `platforms/apple/Sources/Scrivi/ScriviEngine.swift`

**Test Steps:**
1. `swift build` — zero errors ✅
2. All 8 interop tests pass via `swift test` ✅
