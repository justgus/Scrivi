# Task-verified-0004

## T-0004: Mock Services

**Status:** ✅ Implemented - Verified
**Component:** ScriviCore (C++ backend)
**Priority:** Critical
**Epic:** EP-001: ScriviCore Foundation
**Date Requested:** 2026-05-19
**Date Implemented:** 2026-05-19
**Date Verified:** 2026-05-19
**Sprint Assigned:** SP-001

**Rationale:**
Integration tests for project creation, open, and save require controllable, deterministic implementations of all service interfaces. Without mocks, integration tests cannot run portably or produce stable output.

**Current Behavior:**
No mock implementations exist.

**Desired Behavior:**
Tests can run deterministically. Project creation integration tests can use temporary directories. All service interfaces are satisfied by test-only implementations.

**Requirements:**
1. `FixedClock` — returns a configurable deterministic timestamp
2. `DeterministicUUIDProvider` — returns known IDs in sequence for predictable fixture output
3. `MockSecureStore` — in-memory key-value store; must not be compiled into production targets
4. `MockGitProvider` — records calls, returns configurable results; no real Git invoked
5. `NullLogger` — discards all log output; used in tests where logging is irrelevant
6. `LocalFileSystem` — wraps `std::filesystem` for real file I/O; used in integration tests with temp directories

**Design Approach:**
Implement in `ScriviCore/src/platform/`. Mark test-only classes (MockSecureStore, MockGitProvider, FixedClock, DeterministicUUIDProvider) with a `SCRIVI_TEST_ONLY` guard or keep in the test target's CMakeLists. LocalFileSystem and NullLogger may be compiled into the main library.

**Components Affected:**
- ScriviCore/src/platform/: all six implementations
- ScriviCore/tests/CMakeLists.txt: link test-only mocks to test target only

**Implementation Details:**
- `NullLogger`: header-only, discards all output — compiled into main library
- `LocalFileSystem`: wraps `std::filesystem` + `util::atomicWriteTextFile` — compiled into main library
- `FixedClock`, `DeterministicUUIDProvider`, `MockSecureStore`, `MockGitProvider`: header-only, live in `tests/mocks/` — linked only to the test target via `target_include_directories`
- `DeterministicUUIDProvider`: independent counter per ID type, formatted as `<prefix>-001`, `<prefix>-002`
- `MockGitProvider`: `shouldFail` flag + call recording vectors for assertion in tests

**Test Steps:**
1. `FixedClock` returns configured timestamp, stable across calls ✅
2. `DeterministicUUIDProvider` produces sequential IDs; counters are independent per type ✅
3. `MockSecureStore` round-trips arbitrary byte sequences; missing key returns error ✅
4. `MockGitProvider` records calls on success; returns failure when `shouldFail = true` ✅
5. Production CMake target does not link MockSecureStore or MockGitProvider ✅ (mocks are test-include-only)

**Notes:**
See Section 13 of the doc for the design intent behind each mock. DeterministicUUIDProvider is especially important for golden-output tests.

---

*Verified: 2026-05-19*
