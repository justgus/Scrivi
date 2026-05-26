# SP-009: Repair and Recovery — Handlers, Tests, and Adapter

**Status:** 🔵 Planning
**Epic:** EP-004: Repair and Recovery
**Goal:** Implement all 8 manuscript repair handlers (T-0030), automatic rename detection in `RepairClassifier` (T-0031), a full integration test suite for `applyRepair` (T-0032), and the adapter method + full `RepairIssue` serialization in scan/open results (T-0033). After this sprint, EP-004 acceptance criteria are fully met and the repair lifecycle is callable end-to-end from Swift.
**Start Date:** TBD
**End Date:** TBD
**Capacity:** TBD

### Assigned Tasks

| ID     | Title | Priority | Status |
| ------ | ----- | -------- | ------ |
| T-0030 | Repair Handlers — Manuscript File Operations | High | 🔵 Backlog |
| T-0031 | Automatic Rename Detection in `RepairClassifier` | Medium | 🔵 Backlog |
| T-0032 | Integration Tests for `applyRepair` | High | 🔵 Backlog |
| T-0033 | Adapter — `applyRepair` Method + Full `RepairIssue` Serialization in Scan/Open Results | High | 🔵 Backlog |

### Assigned Issues

| ID | Title | Severity | Status |
| -- | ----- | -------- | ------ |
| —  | None  | —        | —      |

### Sprint Notes

- Depends on SP-008 being complete (schema module and facade stub must exist).
- Execution order: T-0030 (fill handler stubs) → T-0031 (rename detection) → T-0032 (integration tests) → T-0033 (adapter surface).
- T-0031 may be parallelized with T-0030 since they touch different files (`RepairClassifier` vs `RepairHandlers`).
- T-0032 is the acceptance gate — each action kind must have ≥1 integration test before EP-004 can be marked complete.
- T-0033 includes two sub-concerns:
  1. Replacing `issueCount` with a full `repairIssues` array in `scanForExternalChanges` and `openProject` JSON results.
  2. Adding `applyRepair` adapter method + `applyRepair` in `ScriviEngine.swift`.
- Sprint ends when `ctest --test-dir build --output-on-failure` passes all tests and all 10 EP-004 acceptance criteria are met.
- After this sprint, EP-004 can be marked Complete (pending user verification).

### Retrospective

*To be filled in when SP-009 closes.*

---

*Last Updated: 2026-05-26 (created during EP-004 planning)*
