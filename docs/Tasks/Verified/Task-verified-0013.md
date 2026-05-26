# Task-verified-0013

## T-0013: appSupportRoot Directory Bootstrap

**Status:** ✅ Implemented - Verified
**Component:** ScriviCore (C++ backend) — app-local path layer
**Priority:** High
**Epic:** EP-003: Identity and First Launch
**Date Requested:** 2026-05-20
**Date Implemented:** 2026-05-20
**Date Verified:** 2026-05-26
**Sprint Assigned:** SP-003

**Rationale:**
ScriviCore assumed `appSupportRoot` subdirectories existed. This task made ScriviCore own the bootstrap of the required directory layout on first use.

**Implementation Details:**
- `src/platform/AppSupportLayout.hpp/.cpp` (originally in `src/util/`, relocated to `src/platform/` in T-0015) — `bootstrapAppSupport()` idempotently creates `identity/`, `state/projects/`, `cache/projects/`, `logs/`, `tmp/`
- Facade calls bootstrap at entry of `ensureLocalIdentity()`, `createProject()`, `openProject()`
- `tests/integration/AppSupportLayoutTests.cpp` — 4 tests (tests 56–59 in final suite)

---

*Last Updated: 2026-05-26*
