# Epic Backlog

Epics listed here are proposed and queued for future planning. They are not yet being actively defined or worked. All items are 🔵 Proposed.

---

## EP-006: Swift Interop and Apple Shell

**Status:** 🔵 Proposed
**Goal:** Build the minimal Apple platform layer: Swift/C++ interop bridge, thin wrapper over all ScriviCore facade operations, and enough SwiftUI shell to prove the full vertical slice end-to-end on macOS.
**Date Proposed:** 2026-05-20
**Depends On:** EP-005 closed and verified; C++ core sufficiently complete

### Scope Notes

T-0011 (SP-003) proves the interop boundary for three operations. This Epic extends that to the full API surface and adds the macOS app shell. UI work intentionally deferred until the C++ core is complete. Drawn from `Scrivi_Cpp24_Core_API_Sketch_v0_2.md` (interop strategy) and `Scrivi_Backend_Architecture_v0_2.md`.

---

*Last Updated: 2026-05-27 (removed EP-004 and EP-005 — both closed/active; EP-006 is now the sole backlog item)*
