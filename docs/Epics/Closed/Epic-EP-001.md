# Epic EP-001 — Closed

## EP-001: ScriviCore Foundation

**Status:** ✅ Closed
**Goal:** A buildable, testable C++24 library skeleton with all value types, utilities, mock services, and schema I/O in place — no live services yet. This establishes the structural and type foundation that all service implementations depend on.
**Date Created:** 2026-05-19
**Target Close Date:** TBD
**Actual Close Date:** 2026-05-20

### Acceptance Criteria

- [x] ScriviCore builds as a standalone static library (no UI dependencies)
- [x] Catch2 tests build and run without UI
- [x] nlohmann/json is hidden behind schema/json utilities and does not appear in public headers
- [x] Public headers expose no UI framework types
- [x] Git is behind `GitProvider` abstraction
- [x] Secure storage is behind `SecureStore` abstraction
- [x] App-local paths are supplied by request structs in MVP

### Sprints

| Sprint | Title | Status | Dates |
| ------ | ----- | ------ | ----- |
| SP-001 | ScriviCore Foundation — Build, Types, and Utilities | ✅ Closed | 2026-05-19 – 2026-05-20 |

### Tasks

| ID     | Title | Status |
| ------ | ----- | ------ |
| T-0001 | Repository Skeleton | ✅ Implemented - Verified |
| T-0002 | Core Value Types | ✅ Implemented - Verified |
| T-0003 | Utility Foundation | ✅ Implemented - Verified |
| T-0004 | Mock Services | ✅ Implemented - Verified |
| T-0005 | Schema Read/Write | ✅ Implemented - Verified |

### Issues

| ID | Title | Status |
| -- | ----- | ------ |
| —  | None  | —      |

### Scope Notes

Scope drawn from Scrivi C++24 Core Repository Skeleton v0.1, Section 16, Milestones 1–5. Non-goals from Section 4 apply (no SQLite, no full object graph, no UI shells, no exports, no remote Git). All 5 tasks completed and verified within SP-001. No scope changes from original plan.

### Completion Summary

EP-001 delivered the complete structural foundation for ScriviCore: a CMake-configured C++24 static library with CI, all public value types, six utility modules, six mock service implementations, and seven JSON schema read/write modules. The test suite reached 52 passing tests at close. All acceptance criteria were met in a single sprint. EP-002 can begin immediately with the full foundation in place.

---

*Closed: 2026-05-20*
