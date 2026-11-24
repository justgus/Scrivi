# Verified Task — T-0702

**Sprint:** [SP-239] · **Epic:** [EP-059] · **Platform:** `[Apple]`
✅ **USER-VERIFIED 2026-11-23** (user, 2026-11-23: *"Steps 3 through 11 also pass.  On all three platforms (mac, iPad, and iPhone)"*); closed (user, 2026-11-23: *"you may close SP-239"*)
**Archived:** 2026-11-23.

---

## ✅ T-0702 — `[Apple]` EP-059 A8: Delete and Change Kind for objects on the Mac and iOS

**Epic:** [EP-059] (A8) · **Status:** ✅ Verified (user, 2026-11-23: *"Steps 3 through 11 also pass.  On all three platforms (mac, iPad, and iPhone)"*) · **Sprint:** [SP-239] (✅ Closed) →
[`../../Sprints/Closed/Sprint-SP-239.md`](../../Sprints/Closed/Sprint-SP-239.md) · **Date Requested:** 2026-11-21 (user, 2026-11-21: *"Build them on Linux and Mac and iOS"*)

**What:** Delete <Kind>… and Change to Artifact / Item in the detail sheet's More menu (both endpoints existed, uncalled); the core
refuses promotes other than item ↔ artifact.

**Built:** `ObjectStore.cpp` (the guard); `ObjectDetailSheet.swift` (More, the delete alert, Back or close, the kind change);
`ObjectDetailHistory.swift` (`visit` compares whole entries); `project.pbxproj` (the test host's bundle-ID suffix restored).
Details and the live pass: the Sprint.
