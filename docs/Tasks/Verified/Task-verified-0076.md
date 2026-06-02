---
name: T-0076
title: "Swift interop tests — round-trip createScene and createChapter"
sprint: SP-022
epic: EP-009
status: ✅ Verified
verified_date: 2026-06-01
---

Added Test 16 (`createSceneInsertsNewScene`) and Test 17 (`createChapterAppendsNewChapter`) to `ScriviInteropTests.swift`. Both verify the full round-trip through the adapter and confirm `openProject` reflects the updated scene list. Swift test count grew from 19 to 21, all green. 171/171 C++ ctests also green.

**Modified files:**
- `platforms/apple/Sources/ScriviCoreAdapter/ScriviCoreAdapter.hpp` — two new method declarations
- `platforms/apple/Sources/ScriviCoreAdapter/ScriviCoreAdapter.cpp` — two new method implementations
- `platforms/apple/Sources/Scrivi/ScriviEngine.swift` — two new engine methods + two new result types
- `platforms/apple/Tests/ScriviInteropTests/ScriviInteropTests.swift` — two new tests
