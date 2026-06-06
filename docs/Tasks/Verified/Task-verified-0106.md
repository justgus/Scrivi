## T-0106: Wire `reorderScene` and `reorderChapter` through `scrivi.h` C API and `ScriviEngine.swift`

**Status:** ✅ Implemented - Verified
**Component:** ScriviCore C API / ScriviEngine.swift
**Priority:** High
**Date Requested:** 2026-06-04
**Date Implemented:** 2026-06-04
**Date Verified:** 2026-06-05
**Sprint Assigned:** SP-031

**Rationale:**
The C++ `reorderScene` and `reorderChapter` facade methods were implemented in SP-029 but were not yet exposed through the `scrivi.h` plain-C API boundary or callable from Swift. This task wires them end-to-end so the SwiftUI drag-and-drop implementation can invoke them.

**Components Affected:**
- `ScriviCore/include/scrivi/scrivi.h` — new C API declarations
- `ScriviCore/src/public_api/scrivi_c_api.cpp` — C API implementations
- `Scrivi/Engine/ScriviEngine.swift` — Swift wrappers calling C API

---

*Closed: 2026-06-05*
