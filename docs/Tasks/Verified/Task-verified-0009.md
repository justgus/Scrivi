# Task T-0009: External Change Scan

**Status:** ✅ Implemented - Verified
**Component:** ScriviCore (C++ backend)
**Priority:** High
**Epic:** EP-002: ScriviCore Services
**Date Requested:** 2026-05-19
**Date Implemented:** 2026-05-20
**Date Verified:** 2026-05-20
**Sprint Assigned:** SP-002

**Rationale:**
Writers may edit files in the `.scrivi` package outside of Scrivi (e.g., in a text editor or via Git merge). Scrivi must detect these changes and surface repair options rather than silently overwriting or corrupting data.

**Current Behavior:**
No external change detection existed.

**Desired Behavior:**
`ScriviCore::scanForExternalChanges()` detects missing content, missing metadata, corrupt metadata, and unregistered Markdown files. Returns a list of `RepairIssue` items with suggested actions.

**Requirements:**
1. `ExternalChangeScanner` walks the project package and compares against registered metadata
2. `RepairClassifier` categorizes each detected issue into a `RepairCategory`
3. Detect: missing `.md` (missingContent), missing `.meta.json` (missingMetadata), corrupt `.meta.json` (corruptMetadata), `.md` file with no matching metadata entry (unregisteredManuscriptFile)
4. Optionally check Git status if `includeGitStatus = true`
5. Returns `ExternalChangeScanResult` with all issues

**Design Approach:**
Implemented in `ScriviCore/src/repair/`. Scanner uses `FileSystem` service to walk directory tree. Classifier uses `RepairIssue.hpp` categories from T-0002.

**Components Affected:**
- ScriviCore/src/repair/: ExternalChangeScanner.hpp/cpp, RepairClassifier.hpp/cpp
- ScriviCore/tests/integration/ExternalChangeTests.cpp

**Implementation Details:**
- `src/repair/RepairClassifier.hpp/.cpp` — builds `RepairIssue` structs for each condition with suggested actions
- `src/repair/ExternalChangeScanner.hpp/.cpp` — reads manuscript index chain, checks all registered files, walks filesystem for unregistered `.md` files, optionally checks Git status
- When scene metadata is missing/corrupt, expected content path derived from metadata path (`.meta.json` → `.md`) and registered to suppress false unregistered-file reports
- `ScriviCore::scanForExternalChanges()` delegates to `ExternalChangeScanner::scan()`

**Test Steps:**
1. `ctest --test-dir build --output-on-failure` — 82/82 tests pass including:
2. Test 71: clean project — zero issues, indexesDirty=false
3. Test 72: deleted scene .md → one missingContent issue, severity blocking
4. Test 73: deleted scene .meta.json → one missingMetadata issue, severity blocking
5. Test 74: corrupt scene .meta.json → one corruptMetadata issue, severity blocking
6. Test 75: unregistered interloper.md → one unregisteredManuscriptFile warning, indexesDirty=true
7. Test 76: MockGitProvider with repoExists=true → gitStatusChecked=true, statusCalls recorded

**Notes:**
Tests create projects on disk and surgically damage them — no pre-built fixtures needed.
