# Epic EP-003 — Closed

## EP-003: Identity and First Launch

**Status:** ✅ Closed
**Goal:** Implement `ensureLocalIdentity()` — the first facade operation in the ScriviCore public API. Delivers a real `IdentityService` backed by `SecureStore`, a real `SystemUUIDProvider` generating UUID v7-style IDs, the first-launch detection flow, and the `appSupportRoot` directory layout. When this Epic closes, ScriviCore can create a local identity and default persona without any account, email, or cloud login, and `createProject()` can reference a real identity rather than a stub.
**Date Created:** 2026-05-20
**Start Date:** 2026-05-20
**Target Close Date:** TBD
**Actual Close Date:** 2026-05-26

### Acceptance Criteria

- [x] `ensureLocalIdentity()` returns an existing identity if one already exists in `SecureStore`
- [x] `ensureLocalIdentity()` creates a new identity (device identity, Scrivi identity ID, default persona, certificate-ready key material) if none exists
- [x] Private identity material is stored in `SecureStore` and never written inside a `.scrivi` project package
- [x] `appSupportRoot` directory layout (`identity/`, `state/projects/`, `cache/projects/`, `logs/`, `tmp/`) is created and owned by ScriviCore
- [x] A real `SystemUUIDProvider` generates UUID v7-style IDs with correct type prefixes (`identity_`, `persona_`, etc.)
- [x] `MockSecureStore` (from EP-001) remains the test implementation; no production tests use it
- [x] `createProject()` (from EP-002) accepts a real `AuthorshipRef` populated from `ensureLocalIdentity()` output
- [x] All new behavior is covered by integration tests using `MockSecureStore` and `DeterministicUUIDProvider`
- [x] `SecureStoreUnavailable` error is returned cleanly when `SecureStore` fails; no partial identity is left in an inconsistent state

### Sprints

| Sprint | Title | Status | Dates |
| ------ | ----- | ------ | ----- |
| SP-003 | ScriviCore Swift Interop | ✅ Closed | 2026-05-20 – 2026-05-21 |

### Tasks

| ID     | Title | Status |
| ------ | ----- | ------ |
| T-0013 | appSupportRoot Directory Bootstrap | ✅ Verified |
| T-0012 | Identity Service and UUID Provider | ✅ Verified |

### Issues

| ID | Title | Status |
| -- | ----- | ------ |
| —  | None  | —      |

### Scope Notes

Drawn from:
- `Scrivi_Cpp24_Core_API_Sketch_v0_2.md` Section 20 (`ensureLocalIdentity` request/result), Section 16.4 (`SecureStore`), Section 18 (app-local path layout)
- `Scrivi_Backend_Behavior_Spec_v0_1.md` Section 5 (first app startup)
- `Scrivi_Project_Creation_and_Open_Flow_v0_1.md` Section 4 (first app startup flow)

**Explicit non-goals:**
- No Apple Keychain implementation (`AppleKeychainSecureStore` belongs to EP-006 / Swift interop layer)
- No identity transfer protocol
- No group personas
- No encryption or cryptographic signing
- No UI

### Completion Summary

EP-003 delivered the identity and first-launch foundation across SP-003:

- **T-0013** (`appSupportRoot` Directory Bootstrap) created the full `appSupportRoot` directory layout (`identity/`, `state/projects/`, `cache/projects/`, `logs/`, `tmp/`) via `bootstrapAppSupport()`, covered by `AppSupportLayoutTests.cpp`.
- **T-0012** (Identity Service and UUID Provider) implemented `IdentityService` backed by `SecureStore`, `SystemUUIDProvider` generating UUID v7-style IDs with type prefixes, idempotency on second call, `FailingSecureStore` error path, and confirmed identity material is never written as plaintext to disk. Covered by 6 `IdentityTests.cpp` tests.
- The identity integration was exercised end-to-end by T-0011 (Swift interop), T-0026 (Swift interop tests for all 7 adapter methods), and T-0027 (MVP loop: step 1 chains identity directly into `createProject`).

Final state: all 9 acceptance criteria met. Both tasks verified. All tests pass.

---

*Closed: 2026-05-26*
