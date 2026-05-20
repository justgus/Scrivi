# Task T-0008: Save Scene

**Status:** ✅ Implemented - Verified
**Component:** ScriviCore (C++ backend)
**Priority:** High
**Epic:** EP-002: ScriviCore Services
**Date Requested:** 2026-05-19
**Date Implemented:** 2026-05-20
**Date Verified:** 2026-05-20
**Sprint Assigned:** SP-002

**Rationale:**
Saving is the most frequent operation a writer performs. It must be atomic, correct, and restore cursor state after reopen.

**Current Behavior:**
No save logic existed.

**Desired Behavior:**
`ScriviCore::saveScene()` atomically writes scene Markdown, updates scene metadata (modifiedAt, modifiedBy, text stats), and updates workspace state (cursor, scroll). After save + reopen, cursor position is restored.

**Requirements:**
1. `SceneWriter` atomically writes Markdown to scene content path
2. Scene metadata `modifiedAt` and `modifiedBy` updated
3. `TextStats` (word count, character count) recalculated and stored in metadata
4. `WorkspaceStateService` write path updates app-local workspace state
5. `SaveSceneResult` reports whether save occurred, metadata updated, and current stats
6. If `previouslyLoadedContentHash` is provided and content is unchanged, skip write (idempotent)

**Design Approach:**
Implemented `SceneWriter` in `ScriviCore/src/manuscript/`. WorkspaceStateService write path in `ScriviCore/src/workspace/`. AtomicWrite utility from T-0003 used for all file writes.

**Components Affected:**
- ScriviCore/src/manuscript/: SceneWriter.hpp/cpp
- ScriviCore/src/workspace/: WorkspaceStateService.hpp/cpp (write path)
- ScriviCore/tests/integration/SaveSceneTests.cpp

**Implementation Details:**
- `src/manuscript/SceneWriter.hpp/.cpp` — atomically writes content, reads+updates scene meta, updates workspace state
- `src/workspace/WorkspaceStateService.hpp/.cpp` — write path shared with T-0007
- `ScriviCore::saveScene()` delegates to `SceneWriter::save()`
- Idempotent: computes `sha256Hex(markdown)`, skips content write if hash matches `previouslyLoadedContentHash`; workspace state always updated
- `countText(markdown)` provides wordCount/characterCount for metadata and result
- `atomicWriteTextFile` (FileSystem service) guarantees no partial writes

**Test Steps:**
1. `ctest --test-dir build --output-on-failure` — 70/70 tests pass including:
2. Test 66: content written to disk reads back identically
3. Test 67: modifiedAt, modifiedByIdentityID, modifiedByDisplayName updated in metadata
4. Test 68: wordCount=10 for 10-word sentence, stored in metadata
5. Test 69: openProject after saveScene restores selection {3,7} and scroll 0.75
6. Test 70: same hash → saved=false, workspaceStateUpdated=true

**Notes:**
AtomicWrite (write to temp, then rename) ensures no partial writes on crash. See T-0003.
