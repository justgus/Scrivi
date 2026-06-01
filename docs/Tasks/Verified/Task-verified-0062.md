# T-0062: Integration Tests — Multi-Scene `openProject` and `openScene`

**Status:** ✅ Implemented - Verified
**Component:** ScriviCore tests / ScriviInteropTests
**Priority:** High
**Date Requested:** 2026-06-01
**Date Implemented:** 2026-06-01
**Date Verified:** 2026-06-01
**Sprint Assigned:** SP-019

**Rationale:**
Multi-scene behaviour needs regression coverage at both the C++ and Swift interop layers.

**Implementation:**

New file `ScriviCore/tests/integration/MultiSceneTests.cpp` with 5 tests:
- `openProject - scenes list contains all scenes in manuscript order`
- `openProject - scenes list has one entry for single-scene project`
- `openScene - loads correct content for requested scene`
- `openScene - updates workspace state to newly opened scene`
- `openScene - returns error for unknown sceneID`

New Swift interop tests in `ScriviInteropTests.swift`:
- `openProject returns scenes array with one entry for a freshly created project`
- `openScene returns correct scene content and openProject restores it as active scene`

`MultiSceneTests.cpp` registered in `ScriviCore/tests/CMakeLists.txt` and `ScriviCore.xcodeproj/project.pbxproj`.

**Test result:** 165/165 ctest (up from 160), 19/19 swift test (up from 17)

---

*Verified: 2026-06-01 (user approved)*
