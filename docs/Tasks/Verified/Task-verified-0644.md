# Verified Task — T-0644

**Sprint:** [SP-200] · **Epic:** [EP-052] · **Platform:** `[ScriviCore]`
✅ **USER-VERIFIED 2026-11-02** (Mac live pass (user, 2026-11-01) and [T-0653] passed on the Linux VM (the live-pass checklist: Pass))
**Archived:** 2026-11-02, on the user's authority (user, 2026-11-02: *"on my authority, closed all the objects that have now tested completed"*).

---

## T-0644 — `[ScriviCore]` History load b2: barrier texts stored as changes

**Epic:** [EP-052] (AC11) · **Status:** ✅ **Verified 2026-11-02** (Mac live pass (user, 2026-11-01) and [T-0653] passed on the Linux VM (the live-pass checklist: Pass)) · **Sprint:** [SP-200] (🟡 Active) →
[`../../Sprints/Closed/Sprint-SP-200.md`](../../Sprints/Closed/Sprint-SP-200.md) · **Date Requested:** 2026-11-01

**What:** design §3 b2. `sceneTexts` as per-scene diffs against the history's text at the parent; `sceneTextsBefore` only
when it differs (Q2); expanded to full texts during the load. 27% of dumas's log.
**Done (2026-11-01):** a barrier's texts are stored as a change against the history's text at its parent
(`sceneTextDiffs`), a before-text equal to it as its name only (`sceneTextsBeforeSame`), a different one as a change;
a scene with no history text then keeps its full text (`persistableBarrierTexts` / `expandBarrierTexts`,
`HistoryService.cpp:280`, `:295`; `HistoryStore.cpp:79`). The load expands them in one walk (`:1332`), which also gives
an older log's barriers their changes, so a compaction writes the new shape. ⚠️ That walk and the consistency check now
share `walkWithTexts` (`:1243`): one text map changed on the way down and restored on the way up — a copy per fork was
the whole manuscript per branch (generated history's open 626 → 331 ms). Test `[T-0644]`: the scene's text is in the log
once (its floor), and after a reopen the next edit undoes to what the op left; a mutation (no expansion) is caught. The
I-0288 legacy test's fabricated old log strips either shape. ctest 799/799, Mac interop 328/328, iOS 17/17, Linux smokes 38/38 (Docker, Qt 6.4, offscreen, non-root).
