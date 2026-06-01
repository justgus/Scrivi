# SP-019: Multi-Scene C++ Core — `openProject` Scene List + `openScene`

**Status:** ✅ Closed
**Epic:** EP-008: Multi-Scene Navigation and Cross-Platform Build
**Goal:** Extend `openProject` to return a full scene list alongside the active scene. Add `openScene` to the C++ facade and adapter, allowing the UI to switch the active scene without reopening the project. All three test suites (macOS ctest, swift test, Ubuntu ctest) must remain green.
**Start Date:** 2026-06-01
**End Date:** 2026-06-01
**Capacity:** —

### Assigned Tasks

| ID     | Title | Priority | Status |
| ------ | ----- | -------- | ------ |
| T-0059 | `OpenProjectResult` — Add Scene List | Critical | ✅ Verified |
| T-0060 | `openScene` Facade Method — Switch Active Scene | Critical | ✅ Verified |
| T-0061 | Adapter + Swift Engine — Expose `openScene` and Scene List | Critical | ✅ Verified |
| T-0062 | Integration Tests — Multi-Scene `openProject` and `openScene` | High | ✅ Verified |

### Assigned Issues

| ID | Title | Severity | Status |
| -- | ----- | -------- | ------ |
| —  | None  | —        | —      |

### Retrospective

**Completed:**
- T-0059: `OpenProjectResult` extended with `std::vector<SceneSummary> scenes`
- T-0060: `openScene` facade method added — resolves order, reads content, updates workspace state
- T-0061: Adapter and Swift engine expose `openScene` and updated `openProject` scene list
- T-0062: 5 C++ integration tests + 2 Swift interop tests; ctest 165/165, swift test 19/19

**Returned to Backlog:** None

**What went well:**
- Clean implementation — no rework needed on core logic
- `ManuscriptOrderResolver` already provided the data; population was straightforward
- Test fixture helper (`addSecondScene`) pattern consistent with existing integration tests

**What to improve:**
- clang-tidy integration via cmake (`CMAKE_CXX_CLANG_TIDY`) was brittle — it applied tidy to third-party deps (Catch2) causing failures; switched to a direct file-by-file script approach
- Partial `clang-tidy --fix` run (before user stopped it) left some source files with linter-applied changes that needed manual cleanup
- Tracking docs (Task-backlog.md) were not cleared of implemented tasks promptly — caught and fixed at SP-020 planning

**Carry-forward notes:**
- SP-020 (clang-tidy sweep, T-0063–T-0066) is the immediate follow-on; 234 warnings remain across 42 source files
- `ExternalChangeScanner.cpp` erase-remove idiom with `std::ranges::remove_if` needed `.begin()` fix — was a latent bug surfaced by linter changes

---

*Closed: 2026-06-01 (user approved)*
