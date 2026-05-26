# Sprint SP-006 — Closed

## SP-006: Skeleton v0.2 Migration — Adapter and Swift Completion

**Status:** ✅ Closed
**Epic:** EP-007: Skeleton v0.2 Migration
**Goal:** Complete the Apple platform wrapper to cover all seven ScriviCore facade operations. Separate `ScriviError.swift` per the approved skeleton layout. Add adapter methods and Swift engine methods for `scanForExternalChanges`, `enableGitSnapshots`, and `createSnapshot`. All Swift interop tests pass green.
**Start Date:** 2026-05-26
**End Date:** 2026-05-26
**Actual Close Date:** 2026-05-26

### Assigned Tasks

| ID     | Title | Priority | Status |
| ------ | ----- | -------- | ------ |
| T-0023 | Extract `ScriviError.swift` as a Separate File | Medium | ✅ Verified |
| T-0024 | Add Adapter Methods for `scanForExternalChanges`, `enableGitSnapshots`, `createSnapshot` | High | ✅ Verified |
| T-0025 | Add Swift Engine Methods for `scanForExternalChanges`, `enableGitSnapshots`, `createSnapshot` | High | ✅ Verified |
| T-0026 | Add Swift Interop Tests for New Adapter Methods | Medium | ✅ Verified |

### Assigned Issues

| ID | Title | Severity | Status |
| -- | ----- | -------- | ------ |
| —  | None  | —        | —      |

### Sprint Notes

- T-0024 also fixed a pre-existing bug: `openProject` on a missing/broken project path returned `ok:true` with an incomplete `activeScene`, causing Test 5 to throw `DecodingError` instead of `ScriviError`. Fixed by checking `v.mode == repairRequired || cannotOpen` and returning `errorEnvelope`.
- `SystemGitProvider` wired into `Impl` — git operations now use the real `git` binary; interop tests skip-guarded when unavailable.
- All 8 `swift test` assertions pass in 0.211 seconds.

### Retrospective

All 4 tasks implemented and verified in one session. The sprint delivered the full 7-method adapter surface, aligned the Swift file layout to skeleton v0.2, and surfaced a pre-existing error-handling deficiency in `openProject` which was fixed as part of the adapter work. EP-007 acceptance criteria are now fully met.

---

*Last Updated: 2026-05-26*
