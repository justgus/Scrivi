# Active Epics

## EP-008: Multi-Scene Navigation and Cross-Platform Build

**Status:** 🔵 Draft
**Goal:** Extend ScriviCore so that `openProject` returns a full scene list and a new `openScene` method allows switching the active scene without reopening the project. Concurrently, prove that ScriviCore builds and all CTests pass on Ubuntu (GCC/Clang) and Windows (MSVC), identify all platform gaps, and produce a `SecureStore` implementation trade study for Linux and Windows.
**Date Created:** 2026-05-30
**Start Date:** —
**Target Close Date:** TBD
**Actual Close Date:** —

### Acceptance Criteria

- [x] ScriviCore builds cleanly on Ubuntu (GCC 13+ or Clang 17+) via CMake; all CTests pass
- [x] ScriviCore builds cleanly on Windows (MSVC 19.38+ / VS 2022) via CMake; all CTests pass
- [x] `AppSupportLayout` uses `$XDG_DATA_HOME` (Linux) and `%APPDATA%` (Windows) for platform-appropriate paths
- [x] All compiler/stdlib gaps between Apple Clang, GCC, and MSVC are documented
- [x] `SecureStore` trade study produced for Linux (libsecret vs. encrypted-file) and Windows (DPAPI) with a concrete recommendation
- [x] `OpenProjectResult` includes a full ordered scene list (`std::vector<SceneSummary>`)
- [x] `openScene` facade method added — switches active scene, updates workspace state, returns scene content
- [x] `ScriviCoreAdapter` exposes `openScene`; `ScriviEngine.swift` wraps it
- [x] Integration tests cover multi-scene `openProject` and `openScene` round-trips
- [x] All three test suites green at close: macOS ctest (165/165), swift test (19/19)
- [ ] Zero clang-tidy warnings across all ScriviCore source files (`verify-scrivi-core-tidy.sh` passes)

### Sprints

| Sprint | Title | Status | Dates |
| ------ | ----- | ------ | ----- |
| SP-017 | Cross-Platform Build — Ubuntu (GCC/Clang) | ✅ Closed | 2026-05-30 – 2026-05-31 |
| SP-018 | Cross-Platform Build — Windows (MSVC) + SecureStore Trade Study | ✅ Closed | 2026-05-31 – 2026-06-01 |
| SP-019 | Multi-Scene C++ Core — `openProject` Scene List + `openScene` | ✅ Closed | 2026-06-01 – 2026-06-01 |
| SP-020 | clang-tidy Housekeeping — Full Warning Sweep | 🟡 Active | 2026-06-01 – — |

### Tasks

| ID | Title | Status |
| -- | ----- | ------ |
| T-0055 | Ubuntu CMake Build — GCC/Clang Green + Gap Document | ✅ Verified |
| T-0056 | Windows CMake Build — MSVC Green + Gap Document | ✅ Verified |
| T-0057 | `AppSupportLayout` — Linux and Windows Platform Paths | ✅ Verified |
| T-0058 | SecureStore Trade Study — Linux and Windows | ✅ Verified |
| T-0059 | `OpenProjectResult` — Add Scene List | ✅ Verified |
| T-0060 | `openScene` Facade Method — Switch Active Scene | ✅ Verified |
| T-0061 | Adapter + Swift Engine — Expose `openScene` and Scene List | ✅ Verified |
| T-0062 | Integration Tests — Multi-Scene `openProject` and `openScene` | ✅ Verified |
| T-0063 | clang-tidy — `readability-braces-around-statements` (142 warnings) | 🟡 Active |
| T-0064 | clang-tidy — `modernize-use-designated-initializers` + `readability-qualified-auto` (51 warnings) | 🟡 Active |
| T-0065 | clang-tidy — `[[nodiscard]]`, `const`, and `static` member functions (21 warnings) | 🟡 Active |
| T-0066 | clang-tidy — Remaining checks (20 warnings) | 🟡 Active |

### Issues

| ID | Title | Status |
| -- | ----- | ------ |
| —  | None  | —      |

### Scope Notes

**Baseline entering EP-007:**
- ScriviCore builds and tests on macOS only (159/159 CTests, 17/17 swift tests).
- `openProject` returns a single `activeScene`; no scene list is present in the result.
- No `openScene` method exists — the only way to switch scenes is to reopen the project.
- `AppSupportLayout` is macOS/Apple-only; no Linux or Windows path logic exists.
- `SecureStore` on non-Apple platforms is unresolved.

**Deferred out of EP-007:**
- SwiftUI scene navigation sidebar — deferred to EP-008
- `SecureStore` implementation for Linux/Windows — trade study only in this Epic; implementation is EP-008 or later
- Android NDK build — future Epic
- Windows/Linux UI shell — future Epic

### Completion Summary

*(Filled in when EP-007 reaches 🟠 Complete)*

---

*Last Updated: 2026-06-01 (T-0059–T-0062 verified; SP-019 closed; SP-020 activated with T-0063–T-0066)*
