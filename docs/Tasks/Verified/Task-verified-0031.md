## T-0031: Automatic Rename Detection in `RepairClassifier`

**Status:** ✅ Implemented - Verified
**Component:** ScriviCore (C++ backend) — repair
**Priority:** Medium
**Epic:** EP-004: Repair and Recovery
**Date Requested:** 2026-05-26
**Date Implemented:** 2026-05-27
**Date Verified:** 2026-05-27
**Sprint Assigned:** SP-009

**Rationale:**
Per repair matrix §6.6–6.8: metadata rename and paired rename can be auto-applied when the `sceneID` inside the renamed file matches exactly and only one candidate exists. The `RepairClassifier` needs `possibleRename` and `possiblePairedRename` issue builders, and the scanner needs to attempt auto-repair for the high-confidence cases.

**Current Behavior:**
`RepairClassifier` has 4 classifiers: `missingContent`, `missingMetadata`, `corruptMetadata`, `unregisteredFile`. No rename detection.

**Desired Behavior:**
`RepairClassifier` adds rename issue builders. `ExternalChangeScanner` attempts auto-apply for exact-ID, single-candidate renames; stages an issue for ambiguous cases.

**Requirements:**
1. `RepairClassifier` gains 4 new static methods for rename conditions
2. `ExternalChangeScanner` cross-references orphan metadata / unmatched content files by `sceneID`
3. Auto-apply path: metadata updated when ID match is unambiguous + single candidate
4. Stage-only path: issue added when multiple candidates or confidence is low
5. Integration tests cover both paths

**Components Affected:**
- Modified: `ScriviCore/src/repair/RepairClassifier.hpp/.cpp`
- Modified: `ScriviCore/src/repair/ExternalChangeScanner.cpp`

**Implementation Details:**
- Added 4 new static classifiers to `RepairClassifier`: `possibleRename`, `possibleMetadataRename`, `possiblePairedRename`, `possibleChapterFolderRename`.
- `issueID` added to all classifiers via `makeIssueID()` (SHA-256 hash of category + sceneID + chapterID + path, truncated to 16 hex chars with "issue-" prefix). Previously empty, which broke dispatcher validation.
- `ExternalChangeScanner` rewritten with rename detection pass: collects `MissingMetaInfo` and `orphanMetasBySceneID`, cross-references; single candidate → auto-apply; multiple candidates → stage issue.
- `handleRelinkToFile` fixed: `issue.path` for `missingContent` is the content path, not the meta path; handler now resolves meta path via `findChapterForScene`.
- Build clean, 112/112 tests pass.

**Test Steps:**
1. Auto-rename test (#114): rename `.meta.json` → scanner auto-relinks, no issue staged ✅
2. Ambiguous rename test (#115): two candidates → `possibleRename` issue staged ✅
3. `ctest --test-dir build --output-on-failure` — 112/112 pass ✅
