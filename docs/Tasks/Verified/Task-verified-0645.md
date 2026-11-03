# Verified Task — T-0645

**Sprint:** [SP-200] · **Epic:** [EP-052] · **Platform:** `[ScriviCore]`
✅ **USER-VERIFIED 2026-11-02** (Mac live pass (user, 2026-11-01) and [T-0653] passed on the Linux VM (the live-pass checklist: Pass))
**Archived:** 2026-11-02, on the user's authority (user, 2026-11-02: *"on my authority, closed all the objects that have now tested completed"*).

---

## T-0645 — `[ScriviCore]` History load b3: the structural payload stored as JSON

**Epic:** [EP-052] (AC11) · **Status:** ✅ **Verified 2026-11-02** (Mac live pass (user, 2026-11-01) and [T-0653] passed on the Linux VM (the live-pass checklist: Pass)) · **Sprint:** [SP-200] (🟡 Active) →
[`../../Sprints/Closed/Sprint-SP-200.md`](../../Sprints/Closed/Sprint-SP-200.md) · **Date Requested:** 2026-11-01

**What:** design §3 b3. Written as an object, read as an object or a string. Core only. 38% of dumas's log.
**Done (2026-11-01):** written as an object, read as an object or a string (`HistoryStore.cpp:460`). Test `[T-0645]`:
the log's shape, and a log rewritten into the old string shape still returns the payload. Linux's
`structured_paste_smoke` read the log's payload as a string; it reads both now. ctest 799/799, Mac interop 328/328, iOS 17/17, Linux smokes 38/38 (Docker, Qt 6.4, offscreen, non-root).
