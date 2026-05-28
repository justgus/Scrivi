# Active Sprint

## SP-013: Inbox and Adapter Completion

**Status:** 🟡 Active
**Epic:** EP-005: Full Project Package — Objects, Assets, and Comments
**Goal:** Implement the inbox facade (`listInbox` / `importFromInbox`) and expose all EP-005 facade methods through `ScriviCoreAdapter` and `ScriviEngine.swift` with interop tests. Closes EP-005.
**Start Date:** 2026-05-28
**End Date:** TBD
**Capacity:** Single session

### Assigned Tasks

| ID     | Title | Priority | Status |
| ------ | ----- | -------- | ------ |
| T-0046 | Inbox — `listInbox` / `importFromInbox` Facade Methods | Medium | 🟠 Implemented - Not Verified |
| T-0047 | `ScriviCoreAdapter` — Expose All EP-005 Facade Methods | High | 🟠 Implemented - Not Verified |

### Assigned Issues

| ID | Title | Severity | Status |
| -- | ----- | -------- | ------ |
| —  | None  | —        | —      |

### Sprint Notes

- T-0046 must land before T-0047 (adapter can't expose `listInbox`/`importFromInbox` until `InboxStore` exists).
- `ProjectCreator` must be amended to create `inbox/dropped-files/` at project creation time.
- All new `.cpp`/`.hpp` files must be added to `ScriviCore.xcodeproj/project.pbxproj` in the same step they are created.
- T-0047 is the EP-005 completion gate: `swift test` must pass before EP-005 can be closed.

### Retrospective

*To be filled in at Sprint close.*

---

*Last Updated: 2026-05-28 (SP-013 activated)*
