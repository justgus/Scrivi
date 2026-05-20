# Task-verified-0001

## T-0001: Repository Skeleton

**Status:** ✅ Implemented - Verified
**Component:** ScriviCore (C++ backend)
**Priority:** Critical
**Epic:** EP-001: ScriviCore Foundation
**Date Requested:** 2026-05-19
**Date Implemented:** 2026-05-19
**Date Verified:** 2026-05-19
**Sprint Assigned:** SP-001

**Rationale:**
The project cannot build, test, or receive any implementation without a correctly structured repository. This is the prerequisite for every subsequent task.

**Current Behavior:**
No repository exists. No build system, no source tree, no test harness.

**Desired Behavior:**
A CMake-configured repository that builds ScriviCore as a static library, runs an empty Catch2 test executable successfully, and has the correct directory skeleton per Section 5 of the Repository Skeleton doc.

**Requirements:**
1. Top-level `CMakeLists.txt` with `add_subdirectory(ScriviCore)`
2. `ScriviCore/CMakeLists.txt` defining a static library target with `cxx_std_23` feature
3. Public headers directory at `ScriviCore/include/scrivi/`
4. Empty `ScriviCore.cpp` in `src/public_api/`
5. Catch2 test target configured via CMake FetchContent
6. `README.md` with build/configure/test instructions
7. `.gitignore` per Section 14 of the doc

**Design Approach:**
Follow Section 5 (top-level layout), Section 9 (CMake targets), Section 10 (FetchContent for Catch2/nlohmann), and Section 14 (.gitignore). Use `cxx_std_23` in CMake until `cxx_std_26` is broadly available.

**Components Affected:**
- Repository root: CMakeLists.txt, CMakePresets.json, .gitignore, README.md
- ScriviCore/: CMakeLists.txt, include/scrivi/, src/public_api/ScriviCore.cpp
- ScriviCore/tests/: CMakeLists.txt

**Implementation Details:**
- Root `CMakeLists.txt`: cmake 3.24, project Scrivi, `enable_testing()` at root, `add_subdirectory(ScriviCore)`
- `ScriviCore/CMakeLists.txt`: static lib, `cxx_std_23`, public include dir, single source `ScriviCore.cpp`
- `ScriviCore/tests/CMakeLists.txt`: Catch2 v3.6.0 via FetchContent (SHA256 verified), `catch_discover_tests`
- 9 public headers written per doc Sections 8.1–8.9 (Error, Result, IDs, Types, RepairIssue, Services, Requests, Results, ScriviCore)
- `ScriviCore.cpp` stub: all 7 methods return `{ErrorCode::internalError, "not implemented"}`
- `ResultTests.cpp` + `IDTests.cpp`: 6 tests, 6 passing
- GitHub Actions CI: `scrivi-core-ci.yml`, matrix: macos-latest + ubuntu-latest, triggers on ScriviCore/ and CMakeLists.txt changes
- `.gitignore` updated with CMake/C++ build artefacts

**Test Steps:**
1. `cmake -S . -B build -DSCRIVI_BUILD_TESTS=ON` ✅ configures without errors
2. `cmake --build build --parallel` ✅ builds ScriviCore static lib + ScriviCoreTests executable
3. `ctest --test-dir build --output-on-failure` ✅ 6/6 tests pass
4. `grep -r "#import" ScriviCore/include/` ✅ clean — no UI framework imports

**Notes:**
CMake FetchContent is the approved dependency strategy for v0.1. Do not vendor or submodule dependencies at this stage. See Section 10.3 of the doc.

---

*Verified: 2026-05-19*
