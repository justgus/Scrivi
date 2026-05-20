# Epic Backlog

Epics listed here are proposed and queued for future planning. They are not yet being actively defined or worked. All items are 🔵 Proposed.

---


## EP-004: Repair and Recovery

**Status:** 🔵 Proposed
**Goal:** Implement the full external change repair system: staging repair actions, applying them, and confirming resolution. Covers all 20 failure conditions defined in the repair matrix.
**Date Proposed:** 2026-05-20
**Depends On:** EP-002 closed and verified

### Scope Notes

Drawn from `Scrivi_External_Change_Repair_Matrix_v0_1.md` (20 failure conditions) and `Scrivi_Backend_Behavior_Spec_v0_1.md` (repair staging and resolution behavior). T-0009 (EP-002) implements detection only. This Epic implements the full repair lifecycle: stage → present to caller → apply → confirm.

---

## EP-005: Full Project Package — Objects, Assets, and Comments

**Status:** 🔵 Proposed
**Goal:** Implement the full `.scrivi` project package beyond manuscripts: objects (characters, locations, items, rules, timelines), assets (with metadata sidecars), comments, and inbox. Delivers the worldbuilding layer that makes Scrivi more than a scene editor.
**Date Proposed:** 2026-05-20
**Depends On:** EP-003 closed and verified

### Scope Notes

Drawn from `Scrivi_Project_Package_Structure_v0_1.md` (objects/, assets/, comments/, inbox/ layout) and `Scrivi_Backend_Architecture_v0_2.md`. None of this package structure is implemented in EP-001 or EP-002. Tasks will be defined when this Epic is activated.

---

## EP-006: Swift Interop and Apple Shell

**Status:** 🔵 Proposed
**Goal:** Build the minimal Apple platform layer: Swift/C++ interop bridge, thin wrapper over all ScriviCore facade operations, and enough SwiftUI shell to prove the full vertical slice end-to-end on macOS.
**Date Proposed:** 2026-05-20
**Depends On:** EP-005 closed and verified; C++ core sufficiently complete

### Scope Notes

T-0011 (SP-003) proves the interop boundary for three operations. This Epic extends that to the full API surface and adds the macOS app shell. UI work intentionally deferred until the C++ core is complete. Drawn from `Scrivi_Cpp24_Core_API_Sketch_v0_2.md` (interop strategy) and `Scrivi_Backend_Architecture_v0_2.md`.

---

*Last Updated: 2026-05-20*
