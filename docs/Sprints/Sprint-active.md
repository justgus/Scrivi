# Active Sprint

---

## SP-002: ScriviCore Services — Project Lifecycle and Repair

**Status:** 🟡 Active
**Epic:** EP-002: ScriviCore Services
**Goal:** Implement the full C++ service layer for the MVP project lifecycle: create, open, save, scan for external changes, and Git snapshots. At the end of this Sprint, all five service operations are implemented and pass integration tests against real temporary directories.
**Start Date:** 2026-05-20
**End Date:** TBD
**Capacity:** TBD

### Assigned Tasks

| ID     | Title | Priority | Status |
| ------ | ----- | -------- | ------ |
| T-0006 | Project Creation | High | 🟡 Implemented - Not Verified |
| T-0007 | Open/Resume | High | 🟡 Implemented - Not Verified |
| T-0008 | Save Scene | High | 🟡 Implemented - Not Verified |
| T-0009 | External Change Scan | High | 🟡 Active |
| T-0010 | Git Snapshots | High | 🟡 Active |

### Assigned Issues

| ID | Title | Severity | Status |
| -- | ----- | -------- | ------ |
| —  | None  | —        | —      |

### Sprint Notes

- T-0006 (Project Creation) is a prerequisite for T-0007, T-0008, and T-0009 — complete and verify it first.
- T-0007, T-0008, and T-0009 can be worked in parallel once T-0006 is in place.
- T-0010 (Git Snapshots) depends on T-0006 for the project creation path; can begin in parallel with T-0007–T-0009.
- Success is measured by integration tests passing against real temp directories for all five operations.
- T-0011 (Swift Interop) is intentionally deferred to SP-003 to de-risk the toolchain boundary work.

### Retrospective

*To be filled in at Sprint close.*

---

*Last Updated: 2026-05-20*
