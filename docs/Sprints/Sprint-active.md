# Active Sprint

---

## SP-003: ScriviCore Swift Interop

**Status:** 🟡 Active
**Epic:** EP-002 / EP-003 (cross-epic sprint)
**Goal:** Prove the Swift/C++ interop boundary by implementing a minimal Apple-side wrapper that calls `createProject()`, `openProject()`, and `saveScene()` through the C++ core. Prerequisites: bootstrap `appSupportRoot` layout (T-0013) and implement `ensureLocalIdentity()` with a real `IdentityService` and `SystemUUIDProvider` (T-0012) so the interop prototype exercises a genuine end-to-end path.
**Start Date:** 2026-05-20
**End Date:** TBD
**Capacity:** TBD

### Assigned Tasks

| ID     | Title | Priority | Status |
| ------ | ----- | -------- | ------ |
| T-0013 | appSupportRoot Directory Bootstrap | High | 🟡 Implemented - Not Verified |
| T-0012 | Identity Service and UUID Provider | High | 🟡 Implemented - Not Verified |
| T-0011 | Swift Interop Prototype | High | 🟡 Active |

### Assigned Issues

| ID | Title | Severity | Status |
| -- | ----- | -------- | ------ |
| —  | None  | —        | —      |

### Sprint Notes

- Execution order: T-0013 first (bootstrap), then T-0012 (identity service), then T-0011 (Swift interop).
- T-0013 and T-0012 are EP-003 scope; T-0011 is EP-002 scope. All three are required to close EP-002's remaining acceptance criterion: "Swift can call `createProject()`, `openProject()`, `saveScene()` through C++ core."
- T-0011 scope is intentionally narrow: prove the interop bridge works end-to-end, not build the full app shell.
- Apple's Swift/C++ direct interop requires C++17 minimum; C++24 header compatibility with the bridge requires validation before T-0011 implementation begins.

### Retrospective

*To be filled in at Sprint close.*

---

*Last Updated: 2026-05-20*
