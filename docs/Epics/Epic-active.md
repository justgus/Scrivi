# Active Epics

---

## EP-002: ScriviCore Services

**Status:** 🟡 Active
**Goal:** All MVP service operations implemented and callable from Swift. Delivers the full MVP loop: create local identity/persona, create project, open project, resume last writing surface, save scene, update metadata, restore workspace state, detect external changes, optionally initialize and create Git-backed snapshots.
**Date Created:** 2026-05-19
**Start Date:** 2026-05-20
**Target Close Date:** TBD
**Actual Close Date:** —

### Acceptance Criteria

- [ ] Full MVP loop from Section 3 works end-to-end in integration tests
- [ ] Swift can call `createProject()`, `openProject()`, `saveScene()` through C++ core
- [ ] No backend behavior reimplemented in Swift
- [ ] Normal project works without Git (Git is opt-in)
- [ ] Cursor and scroll position restored after reopen

### Sprints

| Sprint | Title | Status | Dates |
| ------ | ----- | ------ | ----- |
| SP-002 | ScriviCore Services — Project Lifecycle and Repair | ✅ Closed | 2026-05-20 – 2026-05-20 |
| SP-003 | ScriviCore Swift Interop | 🟡 Active | 2026-05-20 – TBD |

### Tasks

| ID     | Title | Status |
| ------ | ----- | ------ |
| T-0006 | Project Creation | ✅ Verified |
| T-0007 | Open/Resume | ✅ Verified |
| T-0008 | Save Scene | ✅ Verified |
| T-0009 | External Change Scan | ✅ Verified |
| T-0010 | Git Snapshots | ✅ Verified |
| T-0011 | Swift Interop Prototype | 🟡 Active |

### Issues

| ID     | Title | Status |
| ------ | ----- | ------ |
| I-0001 | git commit fails on CI runners with no global git identity configured | ✅ Verified |

### Scope Notes

EP-002 begins after EP-001 is closed and verified. Scope drawn from Section 16, Milestones 6–11.

### Completion Summary

*To be filled in when EP-002 reaches 🟠 Complete.*

---

## EP-003: Identity and First Launch

**Status:** 🟡 Active
**Goal:** Implement `ensureLocalIdentity()` — the first facade operation in the ScriviCore public API. Delivers a real `IdentityService` backed by `SecureStore`, a real `SystemUUIDProvider` generating UUID v7-style IDs, the first-launch detection flow, and the `appSupportRoot` directory layout. When this Epic closes, ScriviCore can create a local identity and default persona without any account, email, or cloud login, and `createProject()` can reference a real identity rather than a stub.
**Date Created:** 2026-05-20
**Start Date:** 2026-05-20
**Target Close Date:** TBD
**Actual Close Date:** —

### Acceptance Criteria

- [ ] `ensureLocalIdentity()` returns an existing identity if one already exists in `SecureStore`
- [ ] `ensureLocalIdentity()` creates a new identity (device identity, Scrivi identity ID, default persona, certificate-ready key material) if none exists
- [ ] Private identity material is stored in `SecureStore` and never written inside a `.scrivi` project package
- [ ] `appSupportRoot` directory layout (`identity/`, `state/projects/`, `cache/projects/`, `logs/`, `tmp/`) is created and owned by ScriviCore
- [ ] A real `SystemUUIDProvider` generates UUID v7-style IDs with correct type prefixes (`identity_`, `persona_`, etc.)
- [ ] `MockSecureStore` (from EP-001) remains the test implementation; no production tests use it
- [ ] `createProject()` (from EP-002) accepts a real `AuthorshipRef` populated from `ensureLocalIdentity()` output
- [ ] All new behavior is covered by integration tests using `MockSecureStore` and `DeterministicUUIDProvider`
- [ ] `SecureStoreUnavailable` error is returned cleanly when `SecureStore` fails; no partial identity is left in an inconsistent state

### Sprints

| Sprint | Title | Status | Dates |
| ------ | ----- | ------ | ----- |
| SP-003 | ScriviCore Swift Interop | 🟡 Active | 2026-05-20 – TBD |

### Tasks

| ID     | Title | Status |
| ------ | ----- | ------ |
| T-0013 | appSupportRoot Directory Bootstrap | 🟡 Implemented - Not Verified |
| T-0012 | Identity Service and UUID Provider | 🟡 Implemented - Not Verified |

### Issues

| ID | Title | Status |
| -- | ----- | ------ |
| —  | None  | —      |

### Scope Notes

Drawn from:
- `Scrivi_Cpp24_Core_API_Sketch_v0_2.md` Section 20 (`ensureLocalIdentity` request/result), Section 16.4 (`SecureStore`), Section 18 (app-local path layout)
- `Scrivi_Backend_Behavior_Spec_v0_1.md` Section 5 (first app startup)
- `Scrivi_Project_Creation_and_Open_Flow_v0_1.md` Section 4 (first app startup flow)

T-0012 and T-0013 are assigned to SP-003 as prerequisites for T-0011 (Swift Interop Prototype). EP-003 closes after T-0012 and T-0013 are verified and the identity integration is exercised by T-0011.

**Explicit non-goals for this Epic:**
- No Apple Keychain implementation (`AppleKeychainSecureStore` belongs to EP-006 / Swift interop layer)
- No identity transfer protocol
- No group personas
- No encryption or cryptographic signing
- No UI

### Completion Summary

*To be filled in when EP-003 reaches 🟠 Complete.*

---

*Last Updated: 2026-05-20*

