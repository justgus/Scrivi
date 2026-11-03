# Verified Task — T-0626

**Sprint:** [SP-195] · **Epic:** [EP-052] · **Platform:** `[Cross]`
✅ **USER-VERIFIED 2026-11-02** (Mac live pass (user, 2026-10-26) and [T-0648] passed on the Linux VM (the live-pass checklist: Pass))
**Archived:** 2026-11-02, on the user's authority (user, 2026-11-02: *"on my authority, closed all the objects that have now tested completed"*).

---

## T-0626 — `[Cross]` EP-052 S2a: every scene save through the queue, Mac and Linux

**Epic:** [EP-052] (AC4, AC5, AC6, AC7) · **Status:** ✅ **Verified 2026-11-02** (Mac live pass (user, 2026-10-26) and [T-0648] passed on the Linux VM (the live-pass checklist: Pass)) · **Sprint:** [SP-195] (🟡 Active) →
[`../../Sprints/Closed/Sprint-SP-195.md`](../../Sprints/Closed/Sprint-SP-195.md) · **Date Requested:** 2026-10-26 (user: *"approve the close.  plan S2."*)

**What:** on both platforms the autosave, app switch, undo and redo writes, Replace All, the drive-returned flush, quit, and the
resume stamp go through `scrivi_enqueue_save_scenes`; a background waiter applies each outcome on the main thread (written →
clean; failed → dirty). The platforms' `noteScenePersisted` save-path calls and W6's ordering code are removed. Replace All
and its undo stop locking the manuscript; Linux Replace All shows a real count ([SP-185] Q3). Close and quit wait for the
queue and ask when it is held ([T-0570] on the Mac; on Linux, [I-0322] folded in per [SP-195] Q3: the Mac's banner and question).

**Done (2026-10-26):** Mac (`ViewportSceneLoader.swift` `enqueueSaves` / `startWaiting` / `apply`) and Linux (`EditorShell.cpp`
`enqueueSaves` / `startWaiting` / `applyWriteOutcomes`): every scene save queued, outcomes on the main thread, W6 removed,
Replace All and undo never lock the manuscript, a close and quit wait for the queue and ask when it is held (Linux [I-0322]:
the banner and the question), Linux Replace All counts *"Replacing N of M"*. **Evidence:** interop 319/319; smokes 38/38;
main-thread file operations for the saves 0 on both (Mac autosave / ⌘Z / ⇧⌘Z 5 → 0; Linux 6 / 8 / 7 → 0); ctest Mac 781,
Linux 784. Details: [SP-195]'s log.
