# Sprint SP-003 — Planning

## SP-003: ScriviCore Swift Interop

**Status:** 🔵 Planning
**Epic:** EP-002: ScriviCore Services
**Goal:** Prove the Swift/C++ interop boundary by implementing a minimal Apple-side wrapper that calls `createProject()`, `openProject()`, and `saveScene()` through the C++ core. Establishes the pattern for the full Apple app shell.
**Start Date:** TBD (begins after SP-002 closes)
**End Date:** TBD
**Capacity:** TBD

### Assigned Tasks

| ID     | Title | Priority | Status |
| ------ | ----- | -------- | ------ |
| T-0011 | Swift Interop Prototype | High | 🔵 Backlog |

### Assigned Issues

| ID | Title | Severity | Status |
| -- | ----- | -------- | ------ |
| —  | None  | —        | —      |

### Sprint Notes

- SP-003 begins after SP-002 is closed and T-0006–T-0010 are verified.
- T-0011 scope is intentionally narrow: prove the interop bridge works, not build the full app shell.
- Apple's Swift/C++ direct interop is well-documented but C++24 header compatibility with the bridge requires validation before implementation begins.
- Capacity estimate should be revisited once SP-002 closes and interop toolchain can be assessed.

### Retrospective

*To be filled in at Sprint close.*

---

*Last Updated: 2026-05-20*
