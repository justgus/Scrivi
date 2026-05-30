# T-0048: `JsonDoc` Double Support — `setDouble` / `getDouble`

**Status:** ✅ Verified
**Component:** ScriviCore / util / JsonDoc
**Priority:** High
**Date Requested:** 2026-05-28
**Date Implemented:** 2026-05-28
**Date Verified:** 2026-05-29
**Sprint Assigned:** SP-014

**Rationale:**
`ScrollPosition.value` is a `double`. The previous `JsonDoc` wrapper had no `setDouble`/`getDouble`, so `WorkspaceStateJson` stored scroll as `int(value * 1000)` — a lossy workaround noted in Architecture v0.3 Open Question 2. This task added the missing methods and removed the workaround.

**Implementation Details:**
- `Json.hpp`: Added `getDouble(key, defaultValue = 0.0)` and `setDouble(key, value)` declarations
- `Json.cpp`: Implemented both — `getDouble` uses `is_number()` (accepts both integer and float JSON numbers); `setDouble` assigns directly to `nlohmann::json` (one-liner, identical shape to `setInt`)
- `WorkspaceStateJson.cpp`: Replaced `lws.setInt("scrollPosition", static_cast<int>(d.scrollPosition * 1000))` / `lws.getInt("scrollPosition") / 1000.0` with `lws.setDouble("scrollPosition", d.scrollPosition)` / `lws.getDouble("scrollPosition")`
- No new files; no pbxproj change required

**Components Affected:**
- `ScriviCore/src/util/Json.hpp` — two declarations added
- `ScriviCore/src/util/Json.cpp` — two implementations added
- `ScriviCore/src/schemas/WorkspaceStateJson.cpp` — workaround removed, `setDouble`/`getDouble` used directly

---

*Verified: 2026-05-29 (user approved)*
