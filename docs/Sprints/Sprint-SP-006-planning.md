# SP-006: Skeleton v0.2 Migration — Adapter and Swift Completion

**Status:** 🟡 Active
**Epic:** EP-007: Skeleton v0.2 Migration
**Goal:** Complete the Apple platform wrapper to cover all seven ScriviCore facade operations. Separate `ScriviError.swift` per the approved skeleton layout. Add adapter methods and Swift engine methods for `scanForExternalChanges`, `enableGitSnapshots`, and `createSnapshot`. All Swift interop tests pass green.
**Start Date:** 2026-05-26
**End Date:** TBD
**Capacity:** TBD

### Assigned Tasks

| ID     | Title | Priority | Status |
| ------ | ----- | -------- | ------ |
| T-0023 | Extract `ScriviError.swift` as a Separate File | Medium | 🟠 Implemented - Not Verified |
| T-0024 | Add Adapter Methods for `scanForExternalChanges`, `enableGitSnapshots`, `createSnapshot` | High | 🟠 Implemented - Not Verified |
| T-0025 | Add Swift Engine Methods for `scanForExternalChanges`, `enableGitSnapshots`, `createSnapshot` | High | 🟠 Implemented - Not Verified |
| T-0026 | Add Swift Interop Tests for New Adapter Methods | Medium | 🟠 Implemented - Not Verified |

### Assigned Issues

| ID | Title | Severity | Status |
| -- | ----- | -------- | ------ |
| —  | None  | —        | —      |

### Sprint Notes

- Depends on SP-005 (all C++ integration tests green).
- Execution order: T-0023 (extract ScriviError) → T-0024 (adapter methods) → T-0025 (Swift methods) → T-0026 (interop tests).
- T-0023 is low-risk and should be done first to clear the structural debt before adding new surface.
- T-0026 note: the `ScriviCoreAdapter::Impl` now wires `SystemGitProvider` (replacing `nullptr`). The `enableGitSnapshots` and `createSnapshot` interop tests are skip-guarded via `withKnownIssue` if git is not available in PATH.
- Sprint ends when `swift test` from `platforms/apple/` is fully green with all 8 interop tests. ✅ Achieved 2026-05-26.
- After this sprint, EP-007 can be marked Complete (pending user verification).

### Retrospective

All 4 tasks implemented in one session. Key notes:
- T-0024 also fixed a pre-existing bug: `openProject` on a missing/broken project path returned `ok:true` with an incomplete `activeScene`, causing Test 5 to throw `DecodingError` instead of `ScriviError`. Fixed by checking `v.mode == repairRequired || cannotOpen` and returning `errorEnvelope` in those cases.
- `SystemGitProvider` wired into `Impl` — git tests now use real `git` binary (skip-guarded when unavailable).
- All 8 `swift test` assertions pass on first run after fix.

---

*Last Updated: 2026-05-26 (all tasks implemented, awaiting user verification)*
