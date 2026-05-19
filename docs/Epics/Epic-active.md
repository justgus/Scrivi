# Active Epics

---

## EP-001: ScriviCore Foundation

**Status:** 🟡 Active
**Goal:** A buildable, testable C++24 library skeleton with all value types, utilities, mock services, and schema I/O in place — no live services yet. This establishes the structural and type foundation that all service implementations depend on.
**Date Created:** 2026-05-19
**Target Close Date:** TBD
**Actual Close Date:** —

### Acceptance Criteria

- [ ] ScriviCore builds as a standalone static library (no UI dependencies)
- [ ] Catch2 tests build and run without UI
- [ ] nlohmann/json is hidden behind schema/json utilities and does not appear in public headers
- [ ] Public headers expose no UI framework types
- [ ] Git is behind `GitProvider` abstraction
- [ ] Secure storage is behind `SecureStore` abstraction
- [ ] App-local paths are supplied by request structs in MVP

### Sprints

| Sprint | Title | Status | Dates |
| ------ | ----- | ------ | ----- |
| —      | Not yet assigned | — | — |

### Tasks

| ID     | Title | Status |
| ------ | ----- | ------ |
| T-0001 | Repository Skeleton | 🔵 Backlog |
| T-0002 | Core Value Types | 🔵 Backlog |
| T-0003 | Utility Foundation | 🔵 Backlog |
| T-0004 | Mock Services | 🔵 Backlog |
| T-0005 | Schema Read/Write | 🔵 Backlog |

### Issues

| ID | Title | Status |
| -- | ----- | ------ |
| —  | None  | —      |

### Scope Notes

Scope drawn from Scrivi C++24 Core Repository Skeleton v0.1, Section 16, Milestones 1–5. Non-goals from Section 4 apply (no SQLite, no full object graph, no UI shells, no exports, no remote Git).

### Completion Summary

*To be filled in when EP-001 reaches 🟠 Complete.*

---

## EP-002: ScriviCore Services

**Status:** 🔵 Draft
**Goal:** All MVP service operations implemented and callable from Swift. Delivers the full MVP loop: create local identity/persona, create project, open project, resume last writing surface, save scene, update metadata, restore workspace state, detect external changes, optionally initialize and create Git-backed snapshots.
**Date Created:** 2026-05-19
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
| —      | Not yet assigned | — | — |

### Tasks

| ID     | Title | Status |
| ------ | ----- | ------ |
| T-0006 | Project Creation | 🔵 Backlog |
| T-0007 | Open/Resume | 🔵 Backlog |
| T-0008 | Save Scene | 🔵 Backlog |
| T-0009 | External Change Scan | 🔵 Backlog |
| T-0010 | Git Snapshots | 🔵 Backlog |
| T-0011 | Swift Interop Prototype | 🔵 Backlog |

### Issues

| ID | Title | Status |
| -- | ----- | ------ |
| —  | None  | —      |

### Scope Notes

EP-002 begins after EP-001 is closed and verified. Scope drawn from Section 16, Milestones 6–11.

### Completion Summary

*To be filled in when EP-002 reaches 🟠 Complete.*

---

*Last Updated: 2026-05-19*
