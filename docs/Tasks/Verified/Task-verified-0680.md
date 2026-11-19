# Verified Task — T-0680

**Sprint:** [SP-222] · **Epic:** [EP-054] · **Platform:** `[Apple]`
✅ **USER-VERIFIED 2026-11-18** (user, 2026-11-18: *"iPad, and Mac live passes all passed.  On the iPhone the software keyboard does not have the shortcuts, but I was able to verify all the functionality using the context menu for cut copy paste, new Scene, and new Chapter and three fingered swipe for undo/redo/branch selection.  close SP-222"*)
**Archived:** 2026-11-18.

---

## ✅ T-0680 — `[Apple]` EP-054: Import Manuscript into the open project on the iPad

**Epic:** [EP-054] (S6a, [SP-218] Q2) · **Status:** ✅ **Verified 2026-11-18** (user) · **Sprint:** [SP-222] (✅ Closed, Q5) → [`../../Sprints/Closed/Sprint-SP-222.md`](../../Sprints/Closed/Sprint-SP-222.md) · **Date Requested:** 2026-11-15 (from
[SP-218]'s iPad live pass; user: *"option 1"*)

**What:** SP-218 Q2 (c) ruled New Project / Into This Project on every platform; the iPad got New Project only, because its editor
has no structured paste or structural undo (`ManuscriptUITextView.swift:12`, EP-055 S2/S3). When it has them: with a project open,
the iPad asks New Project / Into This Project as the Mac does (`ProjectArchiveFlow.swift`, `importManuscript(_:into:)`), and Into
This Project pastes the Range at the caret as one undo step. Until then the iPad says why the project closes
(`presentImportManuscript`, SP-218).
