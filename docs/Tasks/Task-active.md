# Active Tasks

*Last Updated: 2026-11-01 (**T-0400, T-0643–T-0645 → 🟠** → `Task-unverified.md`; T-0646 stays active for the live passes.)*

*Last Updated: 2026-11-01 (**T-0400, T-0643–T-0646 → active** with SP-200's activation (EP-052 AC11).)*

*Last Updated: 2026-10-31 (**T-0639–T-0642** filed from SP-199's live pass and implemented on Apple → 🟠 `Task-unverified.md`.)*

*Last Updated: 2026-10-31 (**T-0636, T-0637, T-0638 → 🟠 Implemented - Not Verified** → `Task-unverified.md`.)*

*Last Updated: 2026-10-31 (**T-0636, T-0637, T-0638 filed → active** with SP-199's activation (EP-055 S2).)*

---

## 🟢 T-0646 — `[Cross]` History load: measurements and Mac and rig live passes

**Epic:** [EP-052] (AC11) · **Status:** 🟢 Active · **Sprint:** [SP-200] (🟡 Active) →
[`../Sprints/Sprint-SP-200.md`](../Sprints/Sprint-SP-200.md) · **Date Requested:** 2026-11-01

**What:** the open's time and the log's bytes for dumas and the generated history (`scrivi_make_test_history`), before
and after each Task; Mac and rig live passes opening both, with undo and redo across a structural edit and a Replace All.

**Measured (2026-11-01, Debug, local disk):**
| History | Log | `scrivi_history_open` |
|---|---|---|
| dumas-prose-timelines | 34 MB → **18.7 MB** (compaction alone: 22.6 MB) | 0.20 s → **0.16 s** (2.05 s this morning) |
| dumas-prose-timelines-history (25,000 events) | 49 MB → **11.7 MB** (the evicted history gone) | 0.72 s → **0.33 s** (16.7 s this morning) |
Equivalence on the real logs (a scratch harness, not in the repo): the tree and the first 300 undo texts are the same read
from the original log and from the compacted, converted one (dumas differs only in the root anchor's timestamp, which every
open restamps). ✅ Mac live pass (user, 2026-11-01: *"All steps pass."*). ⏳ The rig live pass.
