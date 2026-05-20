# Task-verified-0002

## T-0002: Core Value Types

**Status:** ✅ Implemented - Verified
**Component:** ScriviCore (C++ backend)
**Priority:** Critical
**Epic:** EP-001: ScriviCore Foundation
**Date Requested:** 2026-05-19
**Date Implemented:** 2026-05-19
**Date Verified:** 2026-05-19
**Sprint Assigned:** SP-001

**Rationale:**
All service implementations depend on the shared value types, ID types, Result<T>, and error types. These must be correct and compile-clean before any further work.

**Current Behavior:**
Headers exist as sketches in the Repository Skeleton doc but are not implemented.

**Desired Behavior:**
All public headers compile cleanly. `Result<T>` and `Result<void>` work correctly. Basic tests for Result pass.

**Requirements:**
1. `IDs.hpp` — ProjectID, ManuscriptID, ChapterID, SceneID, IdentityID, PersonaID, SnapshotID, CommitID
2. `Types.hpp` — all scalar aliases, enums (ManuscriptStatus, PersonaKind, ProjectRole, MemberStatus, OpenMode), and structs (TextSelection, ScrollPosition, AuthorshipRef, LastWritingSurface, WorkspaceState, ProjectSummary, SceneSummary)
3. `Error.hpp` — ErrorCode enum and Error struct
4. `Result.hpp` — Result<T> and Result<void> per Section 8.2
5. `RepairIssue.hpp` — RepairSeverity, RepairCategory, RepairActionKind, RepairAction, RepairIssue
6. `Requests.hpp` — all request structs per Section 8.8
7. `Results.hpp` — all result structs per Section 8.9
8. `Services.hpp` — all service interfaces and CoreServices struct per Section 8.7

**Design Approach:**
Implement exactly as sketched in Sections 8.1–8.9 of the Repository Skeleton doc. Result<T> may begin with the simple optional-based implementation; add defensive assertions. No third-party types in public headers.

**Components Affected:**
- ScriviCore/include/scrivi/ — all eight headers

**Implementation Details:**
- All 8 public headers were written as part of T-0001 and confirmed complete
- `Result<T>`: optional-based, static `success()`/`failure()` factories, assert guards on `value()` and `error()`
- `Result<void>`: specialisation with private constructor, same assert guard pattern
- `IDs.hpp`: 8 distinct single-field structs (not aliases) — compiler enforces non-interchangeability
- `Types.hpp`: 5 type aliases, 4 enums, 7 structs — no third-party types
- All other headers (`Error`, `RepairIssue`, `Services`, `Requests`, `Results`) compile clean

**Test Steps:**
1. All headers compile with `cmake --build build` ✅
2. `unit/ResultTests.cpp` passes: success/failure construction, ok(), value(), error() ✅ (4/4)
3. `unit/IDTests.cpp` passes: ID types are distinct types, not interchangeable ✅ (2/2)
4. No UI dependency introduced (check with `grep -r "#import" include/`) ✅ clean

**Notes:**
Result<T> may internally use `std::expected` if available on the target compiler. Begin with the simpler optional-based version for portability. See Section 8.2.

---

*Verified: 2026-05-19*
