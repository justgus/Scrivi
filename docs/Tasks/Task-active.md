# Active Tasks

---

## T-0013: appSupportRoot Directory Bootstrap

**Status:** 🟡 Implemented - Not Verified
**Component:** ScriviCore (C++ backend) — app-local path layer
**Priority:** High
**Epic:** EP-003: Identity and First Launch
**Date Requested:** 2026-05-20
**Date Implemented:** 2026-05-20
**Date Verified:** —
**Sprint Assigned:** SP-003

**Rationale:**
`appSupportRoot` is passed into every request struct but ScriviCore never creates or validates the directory layout beneath it. The approved layout (`identity/`, `state/projects/<projectID>/`, `cache/projects/<projectID>/`, `logs/`, `tmp/`) is defined in the API sketch (Section 18) but nothing creates it. In SP-002 integration tests this was handled by the test fixture creating temp directories manually. When a real Swift wrapper calls ScriviCore on a fresh device, `appSupportRoot` may be a path that exists but whose subdirectories do not. This task makes ScriviCore own that bootstrap.

**Current Behavior:**
ScriviCore assumes `appSupportRoot` subdirectories exist. No code creates or validates them. Tests manually create temp directories. First real-device call with a fresh `appSupportRoot` will fail with `ioError` inside workspace state write.

**Desired Behavior:**
ScriviCore bootstraps the full `appSupportRoot` directory layout on first use (or on demand). A dedicated `AppSupportLayout` utility (or equivalent) calls `FileSystem::createDirectories()` for each required subfolder. All operations that write to `appSupportRoot` can rely on this bootstrap having run.

**Requirements:**
1. `AppSupportLayout::bootstrap(appSupportRoot, FileSystem&)` creates all required subdirectories: `identity/`, `state/projects/`, `cache/projects/`, `logs/`, `tmp/` — idempotent (safe to call if directories already exist)
2. Bootstrap is called at the start of `ensureLocalIdentity()` (T-0012) and at the start of `createProject()` / `openProject()` for the `state/` and `cache/` subtrees
3. No subdirectory creation is leaked into existing service classes (ProjectCreator, ProjectOpener, etc.) — bootstrap is a shared entry point
4. Integration tests verify bootstrap runs on a completely empty temp directory and all subdirectories are present afterward
5. Existing tests (Tests 1–82) continue to pass unmodified — bootstrap is additive, not a breaking change

**Design Approach:**
A small `AppSupportLayout` struct or free function in `src/util/` or `src/platform/`. It takes `appSupportRoot` and a `FileSystem*`. Each `createDirectories()` call is independent; a failure on one path returns an error without partially completing. The facade calls it at the top of `ensureLocalIdentity()`. `createProject()` and `openProject()` already receive `appSupportRoot` — they call the relevant subset (state subtree) at their entry points, or the bootstrap can be called in full from the facade for simplicity.

**Components Affected:**
- New: `ScriviCore/src/util/AppSupportLayout.hpp` / `.cpp` (or `src/platform/`)
- Modified: `ScriviCore/src/public_api/ScriviCore.cpp` — call bootstrap at facade entry points
- New: `ScriviCore/tests/integration/test_app_support_layout.cpp`
- `ScriviCore.xcodeproj/project.pbxproj` — must be updated in the same step

**Implementation Details:**
- `AppSupportLayout.hpp/.cpp` added to `src/util/`
- `bootstrapAppSupport()` iterates five required subdirs (`identity/`, `state/projects/`, `cache/projects/`, `logs/`, `tmp/`) calling `FileSystem::createDirectories()` on each; first failure returns immediately
- Facade (`src/public_api/ScriviCore.cpp`) calls `bootstrapAppSupport()` at the top of `ensureLocalIdentity()`, `createProject()`, and `openProject()`; any bootstrap failure propagates as the facade's error
- `AppSupportLayoutTests.cpp` added to `tests/integration/` — 4 tests (Tests 53–56)
- `CMakeLists.txt` updated in both `ScriviCore/` and `tests/`
- `project.pbxproj` updated with new file references and group entries

**Test Steps:**
1. Call `ensureLocalIdentity()` (via T-0012) with a completely empty temp directory as `appSupportRoot` — all subdirectories (`identity/`, `state/projects/`, `cache/projects/`, `logs/`, `tmp/`) exist afterward
2. Call bootstrap a second time on the same directory — no error, no duplicate directories
3. Call `createProject()` with the same `appSupportRoot` on a fresh temp directory — workspace state is written successfully to `state/projects/<projectID>/workspace-state.json`
4. Call `openProject()` on that project with the same `appSupportRoot` — workspace state is read successfully
5. All existing tests (Tests 1–82) continue to pass

**Notes:**
The `identity/` subdirectory is created by bootstrap but ScriviCore never writes plaintext identity material there — that is `SecureStore`'s domain (T-0012). The directory exists as a forward-compatible reservation per the approved layout in API sketch Section 18.

---

## T-0012: Identity Service and UUID Provider

**Status:** 🟡 Implemented - Not Verified
**Component:** ScriviCore (C++ backend) — identity layer
**Priority:** High
**Epic:** EP-003: Identity and First Launch
**Date Requested:** 2026-05-20
**Date Implemented:** 2026-05-20
**Date Verified:** —
**Sprint Assigned:** SP-003

**Rationale:**
`ensureLocalIdentity()` is currently a stub returning `internalError: "not implemented"`. T-0011 (Swift Interop Prototype) needs a real `AuthorshipRef` — with a genuine `IdentityID` and `PersonaID` — to exercise a meaningful end-to-end path. Without this, the Swift prototype either hardcodes fake identity strings or exercises a broken code path. This task implements the full `IdentityService` backed by `SecureStore` and a real `SystemUUIDProvider` generating UUID v7-style IDs.

**Current Behavior:**
`ScriviCore::ensureLocalIdentity()` returns `Result::failure({ErrorCode::internalError, "not implemented"})`. `DeterministicUUIDProvider` exists for tests but no production `UUIDProvider` implementation exists. All identity values in SP-002 tests were hardcoded strings.

**Desired Behavior:**
`ensureLocalIdentity()` fully implemented: returns existing identity if one is present in `SecureStore`; creates a new identity (device ID, Scrivi identity ID, default persona, a random secret blob as placeholder key material) if none exists. `SystemUUIDProvider` generates UUID v7-style IDs with correct type prefixes (`identity_`, `persona_`, `project_`, etc.). `createProject()` can be called with the `AuthorshipRef` produced by `ensureLocalIdentity()`.

**Requirements:**
1. `IdentityService` class implements `ensureLocalIdentity()` logic: check `SecureStore` for existing identity, return it if present
2. On first call, `IdentityService` generates: a `deviceID` string, an `IdentityID` (via `UUIDProvider`), a `PersonaID` (via `UUIDProvider`), and a random 32-byte secret blob stored under a stable key in `SecureStore`
3. Identity material is stored in `SecureStore` only — never written inside a `.scrivi` project package and never written to `appSupportRoot/identity/` as plaintext
4. `SystemUUIDProvider` implements `UUIDProvider` interface; all `new*ID()` methods generate UUID v7-style strings with correct type prefix (e.g., `identity_`, `persona_`, `project_`, `scene_`, etc.)
5. `SecureStoreUnavailable` / `secureStoreError` is returned cleanly if `SecureStore` fails; no partial identity is left in an inconsistent state
6. `MockSecureStore` (already exists in `tests/mocks/`) remains the test implementation; no production tests use it
7. Integration tests cover: new identity creation, idempotent re-call returning same identity, `SecureStore` failure path

**Design Approach:**
`IdentityService` lives in `src/identity/`. It takes `SecureStore*`, `UUIDProvider*`, and `Clock*` from `CoreServices`. The storage key scheme uses stable string constants (e.g., `"scrivi.identity.v1"`). `SystemUUIDProvider` lives in `src/platform/` alongside `LocalFileSystem`. UUID v7-style generation uses the current timestamp (milliseconds since Unix epoch) packed into the high bits of a 128-bit value, with random low bits; formatted as `<prefix>_<base32-or-hex-string>`. The exact format should match the examples in `Scrivi_Cpp24_Core_API_Sketch_v0_2.md` Section 8.1.

Key material scoping: the Behavior Spec (Section 5.3) and API sketch (Section 20.3) reference "certificate-ready identity material" but no cryptographic design is approved yet. This task stores a 32-byte random secret blob as a forward-compatible placeholder. Actual key format is deferred to a future epic.

**Components Affected:**
- New: `ScriviCore/src/identity/IdentityService.hpp` / `.cpp`
- New: `ScriviCore/src/platform/SystemUUIDProvider.hpp` / `.cpp`
- Modified: `ScriviCore/src/public_api/ScriviCore.cpp` — wire `IdentityService` into `ensureLocalIdentity()`
- New: `ScriviCore/tests/integration/test_identity.cpp`
- `ScriviCore.xcodeproj/project.pbxproj` — must be updated in the same step

**Implementation Details:**
- `src/identity/IdentityService.hpp/.cpp` — `ensureLocalIdentity()` checks `SecureStore` for key `"scrivi.identity.v1"`; returns existing identity if present, otherwise generates new `IdentityID`/`PersonaID` via `UUIDProvider`, builds a JSON bundle (identity ID, persona ID, display name, device ID, 32-byte hex secret material), stores as `SecretBytes`, returns `createdNewIdentity=true`; any `SecureStore` failure propagates as `secureStoreError`
- `src/platform/SystemUUIDProvider.hpp/.cpp` — UUID v7-style generation using 48-bit Unix millisecond timestamp in high bits, random low bits, RFC 9562 version/variant nibbles; formatted as `<prefix>_<xxxxxxxx-xxxx-7xxx-yxxx-xxxxxxxxxxxx>`
- `src/public_api/ScriviCore.cpp` — `ensureLocalIdentity()` now delegates to `IdentityService` after bootstrap
- `tests/integration/IdentityTests.cpp` — 6 tests (Tests 57–62): new identity creation, prefixed IDs, idempotency, `SecureStore` failure path, no plaintext in `identity/` dir, `createProject()` with real `AuthorshipRef`
- Test 54 (AppSupportLayoutTests) updated: assertion corrected now that `ensureLocalIdentity()` succeeds instead of returning a stub error

**Test Steps:**
1. Call `ensureLocalIdentity()` with a fresh `MockSecureStore` and a valid `appSupportRoot` — result is success, `createdNewIdentity == true`, `identityID` and `defaultPersonaID` are non-empty with correct prefixes
2. Call `ensureLocalIdentity()` again with the same `MockSecureStore` — result is success, `createdNewIdentity == false`, same `identityID` and `defaultPersonaID` returned
3. Call `ensureLocalIdentity()` with a `MockSecureStore` configured to fail on `putSecret` — result is failure with `secureStoreError` or `secureStoreUnavailable`, no partial identity stored
4. Call `createProject()` using the `AuthorshipRef` constructed from step 1's result — project created successfully, `project-members.json` and `project-personas.json` contain the real identity/persona IDs
5. All existing tests (Tests 1–82) continue to pass

**Notes:**
`DeterministicUUIDProvider` in `tests/mocks/` must not be changed — it is the test double for all existing and future integration tests. `SystemUUIDProvider` is the production implementation only.

---

## T-0011: Swift Interop Prototype

**Status:** 🟡 Active
**Component:** ScriviCore (C++ backend) / Apple platform wrapper
**Priority:** High
**Epic:** EP-002: ScriviCore Services
**Date Requested:** 2026-05-19
**Date Implemented:** —
**Date Verified:** —
**Sprint Assigned:** SP-003

**Rationale:**
The C++ core has no value if the Apple platform cannot call it. A minimal Swift wrapper that exercises `createProject()`, `openProject()`, and `saveScene()` proves the interop boundary works and establishes the pattern for the full Apple app shell. Depends on T-0013 (bootstrap) and T-0012 (real identity) to exercise a genuine end-to-end path.

**Current Behavior:**
No Swift/C++ interop wrapper exists.

**Desired Behavior:**
A minimal Apple-side wrapper can call `ensureLocalIdentity()`, `createProject()`, `openProject()`, and `saveScene()` through the C++ core using real identity values. No backend behavior is reimplemented in Swift.

**Requirements:**
1. Swift/C++ direct interop (not Objective-C bridging) per approved approach in Repository Skeleton doc Section 2
2. Swift wrapper exposes `ensureLocalIdentity()`, `createProject()`, `openProject()`, `saveScene()` to Swift callers
3. Swift wrapper handles `Result<T>` conversion to Swift error/value pattern
4. No ScriviCore backend logic duplicated in Swift
5. Wrapper compiles and links against ScriviCore static library
6. End-to-end test: call `ensureLocalIdentity()` → use returned `AuthorshipRef` in `createProject()` → call `openProject()` → call `saveScene()`

**Design Approach:**
Use direct Swift/C++ interop. The wrapper lives outside `ScriviCore/` (it is a platform adapter, not part of the core). A minimal test target or playground can exercise the entry points. The full Apple app shell is out of scope for this task.

**Components Affected:**
- New directory: `apple/` or `platforms/apple/` (wrapper target)
- ScriviCore/include/scrivi/: public headers consumed by Swift
- CMakeLists.txt: may need XCFramework or xcbuild integration for Xcode targets

**Implementation Details:**
*To be filled in during implementation.*

**Test Steps:**
1. Swift code calls `ensureLocalIdentity()` — returns a real `IdentityID` and `PersonaID`, no crash
2. Swift code calls `createProject()` with the returned `AuthorshipRef` — project created on disk, no crash
3. Swift code calls `openProject()` on that project — returns active scene Markdown
4. Swift code calls `saveScene()` — Markdown persists and is readable
5. No backend logic (JSON parsing, path resolution, etc.) exists in Swift layer
6. Build succeeds on macOS 26+ with Xcode 26+

**Notes:**
Swift/C++ interop requires C++17 minimum; C++24 features in headers exposed to Swift must be compatible with the interop bridge. This may require careful header hygiene. Confirm interop toolchain behavior before implementation.

---

*Last Updated: 2026-05-20*
