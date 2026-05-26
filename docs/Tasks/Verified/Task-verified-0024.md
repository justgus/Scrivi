# Task-verified-0024

## T-0024: Add Adapter Methods for `scanForExternalChanges`, `enableGitSnapshots`, `createSnapshot`

**Status:** ✅ Implemented - Verified
**Component:** ScriviCoreAdapter (C++) / Apple platform wrapper
**Priority:** High
**Epic:** EP-007: Skeleton v0.2 Migration
**Date Requested:** 2026-05-22
**Date Implemented:** 2026-05-26
**Date Verified:** 2026-05-26
**Sprint Assigned:** SP-006

**Rationale:**
`ScriviCoreAdapter` previously exposed four of the seven ScriviCore facade methods. The remaining three — `scanForExternalChanges`, `enableGitSnapshots`, `createSnapshot` — had no adapter surface. This task completed the adapter coverage and wired in `SystemGitProvider` for production use.

**Current Behavior (before):**
`ScriviCoreAdapter` had 4 methods. `scanForExternalChanges`, `enableGitSnapshots`, `createSnapshot` unreachable from Swift. `Impl::gitProvider` was `nullptr`.

**Desired Behavior (after):**
`ScriviCoreAdapter` has 7 methods. All three new methods accept `const char*` inputs and return `std::string` JSON envelopes. `SystemGitProvider` wired in `Impl`.

**Requirements:**
1. `ScriviCoreAdapter.hpp` declares 3 new methods ✅
2. `ScriviCoreAdapter.cpp` implements all three, delegating to `impl_->core` ✅
3. JSON result schemas match spec ✅
4. No ScriviCore types appear in `ScriviCoreAdapter.hpp` ✅
5. `swift build` succeeds ✅

**Implementation Details:**
- Added `SystemGitProvider` include and wired `scrivi::git::SystemGitProvider gitProvider` into `Impl` (replaces `nullptr`).
- Added `openProject` mode guard: returns `errorEnvelope` when `mode == repairRequired` or `cannotOpen`, fixing a pre-existing bug where Test 5 threw `DecodingError` instead of `ScriviError`.
- Added 3 methods to `.hpp` and `.cpp`: `scanForExternalChanges(projectRootPath, appSupportRoot, includeGitStatus)`, `enableGitSnapshots(projectRootPath, identityID, personaID, displayName, initialSnapshotLabel)`, `createSnapshot(projectRootPath, identityID, personaID, displayName, label, note)`.
- JSON schemas: scan → `{projectID, indexesDirty, gitStatusChecked, hasUnsnapshottedChanges, issueCount}`, enableGit → `{gitInitialized, alreadyRepository, initialSnapshotID, initialCommitID}`, createSnapshot → `{snapshotID, commitID, createdAt, created}`.

**Components Affected:**
- Modified: `platforms/apple/Sources/ScriviCoreAdapter/ScriviCoreAdapter.hpp`
- Modified: `platforms/apple/Sources/ScriviCoreAdapter/ScriviCoreAdapter.cpp`

**Test Steps:**
1. `swift build` from `platforms/apple/` — zero errors ✅
2. `swift test` — all 8 tests pass ✅
