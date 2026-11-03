# Verified Task — T-0646

**Sprint:** [SP-200] · **Epic:** [EP-052] · **Platform:** `[Cross]`
✅ **USER-VERIFIED 2026-11-02** (Mac live pass (user, 2026-11-01) and [T-0653] passed on the Linux VM (the live-pass checklist: Pass); the load measured 82.0 → 6.0 s on the VM)
**Archived:** 2026-11-02, on the user's authority (user, 2026-11-02: *"on my authority, closed all the objects that have now tested completed"*).

---

## T-0646 — `[Cross]` History load: measurements and Mac and rig live passes

**Epic:** [EP-052] (AC11) · **Status:** ✅ **Verified 2026-11-02** (Mac live pass (user, 2026-11-01) and [T-0653] passed on the Linux VM (the live-pass checklist: Pass); the load measured 82.0 → 6.0 s on the VM) · **Sprint:** [SP-200] (🟡 Active) →
[`../../Sprints/Closed/Sprint-SP-200.md`](../../Sprints/Closed/Sprint-SP-200.md) · **Date Requested:** 2026-11-01

**What:** the open's time and the log's bytes for dumas and the generated history (`scrivi_make_test_history`), before
and after each Task; Mac and rig live passes opening both, with undo and redo across a structural edit and a Replace All.

**Measured (2026-11-01, Debug, local disk):**
| History | Log | `scrivi_history_open` |
|---|---|---|
| dumas-prose-timelines | 34 MB → **18.7 MB** (compaction alone: 22.6 MB) | 0.20 s → **0.16 s** (2.05 s this morning) |
| dumas-prose-timelines-history (25,000 events) | 49 MB → **11.7 MB** (the evicted history gone) | 0.72 s → **0.33 s** (16.7 s this morning) |
Equivalence on the real logs (a scratch harness, not in the repo): the tree and the first 300 undo texts are the same read
from the original log and from the compacted, converted one (dumas differs only in the root anchor's timestamp, which every
open restamps). ✅ Mac live pass (user, 2026-11-01: *"All steps pass."*). ➡️ The rig live pass → [T-0653] ([EP-057], 2026-11-02).
