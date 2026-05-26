# SP-007: EP-002 Completion — Full MVP Loop Test

**Status:** 🔵 Planning
**Epic:** EP-002: ScriviCore Services
**Goal:** Add a single C++ integration test that chains all 13 Section 3 MVP loop steps end-to-end, making EP-002 acceptance criterion 1 unambiguous.
**Start Date:** TBD
**End Date:** TBD

### Assigned Tasks

| ID     | Title | Priority | Status |
| ------ | ----- | -------- | ------ |
| T-0027 | Full MVP Loop Integration Test | High | 🔵 Backlog |

### Assigned Issues

| ID | Title | Severity | Status |
| -- | ----- | -------- | ------ |
| —  | None  | —        | —      |

### Sprint Notes

- All 13 Section 3 steps must be exercised in a single `TEST_CASE` in a new file `integration/MvpLoopTests.cpp`.
- Use `MockGitProvider` for git steps (consistent with existing integration test pattern).
- File must be added to `CMakeLists.txt` and `ScriviCore.xcodeproj/project.pbxproj` in the same step.
- Sprint ends when `ctest --test-dir build --output-on-failure` passes the new test.
- On close, EP-002 is eligible to be marked Complete.

---

*Last Updated: 2026-05-26*
