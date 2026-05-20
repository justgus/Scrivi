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
| SP-002 | ScriviCore Services — Project Lifecycle and Repair | 🟡 Active | 2026-05-20 – TBD |
| SP-003 | ScriviCore Swift Interop | 🔵 Planning | TBD |

### Tasks

| ID     | Title | Status |
| ------ | ----- | ------ |
| T-0006 | Project Creation | 🟡 Active |
| T-0007 | Open/Resume | 🟡 Active |
| T-0008 | Save Scene | 🟡 Active |
| T-0009 | External Change Scan | 🟡 Active |
| T-0010 | Git Snapshots | 🟡 Active |
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

*Last Updated: 2026-05-20*

