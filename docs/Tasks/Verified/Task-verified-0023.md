# Task-verified-0023

## T-0023: Extract `ScriviError.swift` as a Separate File

**Status:** ✅ Implemented - Verified
**Component:** Apple platform wrapper
**Priority:** Medium
**Epic:** EP-007: Skeleton v0.2 Migration
**Date Requested:** 2026-05-22
**Date Implemented:** 2026-05-26
**Date Verified:** 2026-05-26
**Sprint Assigned:** SP-006

**Rationale:**
The skeleton v0.2 (Section 7) specifies two separate Swift source files: `ScriviEngine.swift` and `ScriviError.swift`. Currently the error type, envelope types, and `decode()` helper all live inline in `ScriviEngine.swift`. Separating them matches the approved layout and keeps each file focused.

**Current Behavior:**
`Sources/Scrivi/ScriviEngine.swift` contained: the `ScriviEngine` class, all Swift result types, `ScriviError`, `Envelope<T>`, `ErrorPayload`, and the `decode()` free function.

**Desired Behavior:**
`Sources/Scrivi/ScriviError.swift` contains: `ScriviError`, `Envelope<T>`, `ErrorPayload`, `decode()`. `ScriviEngine.swift` contains: `ScriviEngine` class and all Swift result types only.

**Requirements:**
1. `Sources/Scrivi/ScriviError.swift` created with `ScriviError`, `Envelope`, `ErrorPayload`, `decode()` ✅
2. Those types removed from `ScriviEngine.swift` ✅
3. `swift build` succeeds ✅
4. `swift test` passes all existing interop tests ✅

**Implementation Details:**
- Created `platforms/apple/Sources/Scrivi/ScriviError.swift` with `ScriviError`, `Envelope<T>`, `ErrorPayload`, `decode()`.
- Removed those types from `ScriviEngine.swift`, replaced with a comment pointing to the new file.
- `swift build` clean (0 errors). `swift test` all 8 tests pass.

**Components Affected:**
- New: `platforms/apple/Sources/Scrivi/ScriviError.swift`
- Modified: `platforms/apple/Sources/Scrivi/ScriviEngine.swift`

**Test Steps:**
1. `swift build` from `platforms/apple/` — succeeds ✅
2. `swift test` from `platforms/apple/` — all 8 tests pass ✅
