# Verified Task — T-0685

**Sprint:** [SP-222] · **Epic:** [EP-055] · **Platform:** `[Apple]`
✅ **USER-VERIFIED 2026-11-18** (user, 2026-11-18: *"iPad, and Mac live passes all passed.  On the iPhone the software keyboard does not have the shortcuts, but I was able to verify all the functionality using the context menu for cut copy paste, new Scene, and new Chapter and three fingered swipe for undo/redo/branch selection.  close SP-222"*)
**Archived:** 2026-11-18.

---

## ✅ T-0685 — `[Apple]` EP-055 S3a: undo/redo, structural edits and cross-scene cut/copy/paste on iOS

**Epic:** [EP-055] (AC5, first half) · **Status:** ✅ **Verified 2026-11-18** (user) · **Sprint:** [SP-222] (✅ Closed) →
[`../../Sprints/Closed/Sprint-SP-222.md`](../../Sprints/Closed/Sprint-SP-222.md) · **Date Requested:** 2026-11-18 (user: *"…proceed with EP-055."*)

**What:** on iOS, undo and redo through Scrivi's history; New Scene / Chapter, Split and Merge as one range call and one undo step;
cut, copy and paste across scenes — by moving the Mac `Coordinator`'s undo apply path, `runStructuralEdit` and the cross-scene
clipboard into shared code that both hosts call. Rulings and ACs: the Sprint.
