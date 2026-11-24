# Verified Task — T-0703

**Sprint:** [SP-240] · **Epic:** [EP-059] · **Platform:** `[Linux]` (+ the Mac's trail, Q3)
✅ **USER-VERIFIED 2026-11-23** (user, 2026-11-23: *"All steps pass on the Linux VM and the Mac."*); closed (user, 2026-11-23: *"Step 2 passes now, close SP-240"*)
**Archived:** 2026-11-23.

---

## ✅ T-0703 — `[Linux]` EP-059 A9: Delete and Change Kind for objects on Linux, as the Mac's

**Epic:** [EP-059] (A9) · **Status:** ✅ Verified (user, 2026-11-23: *"All steps pass on the Linux VM and the Mac."*) · **Sprint:** [SP-240] (✅ Closed) →
[`../../Sprints/Closed/Sprint-SP-240.md`](../../Sprints/Closed/Sprint-SP-240.md) · **Date Requested:** 2026-11-21 (user, 2026-11-21: *"Build them on Linux and Mac and iOS"*)

**What:** the Mac's ⋯ menu ([SP-239]) in Linux's detail window — Change to Artifact / Item, Delete <Kind>… — and, on both platforms,
a Back trail that never reaches a deleted object or one under its old kind (Q3).

**Built:** `ObjectDetailDialog.cpp/.hpp` (⋯, the delete box, `deleteCurrent`/`changeKind` off the main thread, the trail);
`ObjectDetailSheet.swift` + `ObjectDetailHistory.swift` (the Mac's trail, Q3); `object_detail_smoke.cpp` §13; an interop test.
Details and the live pass: the Sprint.
