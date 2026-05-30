# SP-013: Inbox and Adapter Completion

**Status:** ✅ Closed
**Epic:** EP-005: Full Project Package — Objects, Assets, and Comments
**Goal:** Implement the inbox facade (`listInbox` / `importFromInbox`) and expose all EP-005 facade methods through `ScriviCoreAdapter` and `ScriviEngine.swift` with interop tests. Closes EP-005.
**Start Date:** 2026-05-28
**End Date:** 2026-05-28
**Capacity:** Single session

### Assigned Tasks

| ID     | Title | Priority | Status |
| ------ | ----- | -------- | ------ |
| T-0046 | Inbox — `listInbox` / `importFromInbox` Facade Methods | Medium | ✅ Implemented - Verified |
| T-0047 | `ScriviCoreAdapter` — Expose All EP-005 Facade Methods | High | ✅ Implemented - Verified |

### Assigned Issues

| ID | Title | Severity | Status |
| -- | ----- | -------- | ------ |
| —  | None  | —        | —      |

### Sprint Notes

- T-0046 landed before T-0047 (adapter requires `InboxStore` to exist).
- `ProjectCreator` amended to create `inbox/dropped-files/` at project creation time.
- All new `.cpp`/`.hpp` files added to `ScriviCore.xcodeproj/project.pbxproj` in the same step (E022–E024, GRP_INBOX).
- T-0047 was the EP-005 completion gate: `swift test` passed before EP-005 was marked Complete.

### Retrospective

Both tasks implemented and verified in a single session. `InboxStore` landed cleanly with three action branches (importAsAsset delegates to `AssetStore`, ignore, deleteFile). Adapter completion added 13 new methods across object CRUD, assets, comments, and inbox — all wired through `ScriviEngine.swift` with 12 new result types and 8 new interop tests. 158/158 CTest + 17/17 `swift test` passing at close. No issues or blockers. EP-005 is complete.

---

*Closed: 2026-05-28*
