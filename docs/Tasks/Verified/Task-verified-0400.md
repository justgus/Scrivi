# Verified Task — T-0400

**Sprint:** [SP-200] · **Epic:** — · **Platform:** `[ScriviCore]`
✅ **USER-VERIFIED 2026-11-02** (Mac live pass (user, 2026-11-01) and [T-0653] passed on the Linux VM (the live-pass checklist: Pass))
**Archived:** 2026-11-02, on the user's authority (user, 2026-11-02: *"on my authority, closed all the objects that have now tested completed"*).

---

## T-0400 — `[ScriviCore]` History log-segment rotation → (c) compaction

**Epic:** EP-052 (AC11) — was EP-019 (deferred) · **Status:** ✅ **Verified 2026-11-02** (Mac live pass (user, 2026-11-01) and [T-0653] passed on the Linux VM (the live-pass checklist: Pass)) · **Sprint:** [SP-200] (🟡 Active) →
[`../../Sprints/Closed/Sprint-SP-200.md`](../../Sprints/Closed/Sprint-SP-200.md). Was 🟢 Nice to have (user ruling 2026-08-11); scheduled
2026-11-01 (user: *"I'd like to do all three (a), (b), and (c)"*).

**Now (2026-11-01):** design [`../Scrivi_History_Load_Design_v0_1.md`](../../Scrivi_History_Load_Design_v0_1.md) §2 — the
live tree written to `log-(N+1).jsonl` through the write queue at open and close when worth it (Q1), `state.json` moved to
it, old segments removed; the open replays only that segment. ⚠️ **Correction:** the text below says the reader honours
`activeLogSegment`; it does not — `state.json` is read only after `log-000001.jsonl` has been replayed
(`HistoryStore.cpp`), so reading it first is part of this Task. dumas's log is now 34 MB.

**As filed:**

`activeSegment_` is hard-fixed to `log-000001.jsonl` (`HistoryStore.hpp:114`); nothing rotates it, so
a project accumulates one unbounded segment (a real project is already ~3.4 MB). Capacity/eviction
bounds the **tree**, not the **log**. The reader already honours `activeLogSegment` from `state.json`,
so rotation is **additive, with no format change**. Documented as not-implemented in package-structure
§16a. Raised by T-0217; deliberately **not** folded into a verification sprint.
**Done (2026-11-01):** `HistoryStore::resolveActiveSegment` (`HistoryStore.cpp:640`) opens the segment `state.json`
names (else the newest) and removes the others; `compactIfWorthIt` / `compact` (`:664`, `:678`) write the live tree as log
records — the last session closed, floors, a promoted root named by a new `ctl:root`, every live node parents-first, fork
primaries, the current node, the persisted hashes, `ctl:compacted` — through the write queue, then `state.json`. Called at
open (after the load) and at close (`scrivi_c_api.cpp:4238`); the queue is connected before the open so the open's
compaction is queued. ⚠️ Q1's "10,000 in all" is counted since the LAST compaction: a 20,000-event live tree always holds
more, and would be rewritten at every open. Tests `[T-0400]` (`HistoryCApiTests.cpp`): a 2,500-event history at capacity
50 with a re-selected fork, a barrier and an edit group compacts at close and reopens as the same tree and undo texts; the
three crash points of design §2.2; a short history is not compacted. A mutation (no fork-primary records) is caught — the
first version of the test missed it ([I-0332] notes why). Real logs (a scratch harness): dumas and the generated history
open as the same tree and the same 300 undo texts before and after. ctest 799/799, Mac interop 328/328, iOS 17/17, Linux smokes 38/38 (Docker, Qt 6.4, offscreen, non-root).
