# Active Tasks

---

## 🟡 T-0614 — `[ScriviCore]` EP-053 S1: the Range and the aggregate range operations

**Epic:** [EP-053] (S1) · **Status:** 🟡 **Active** (2026-10-17) · **Sprint:** [SP-186] (🟡 Active) →
[`../Sprints/Sprint-SP-186.md`](../Sprints/Sprint-SP-186.md) · **Date Requested:** 2026-10-17 (user: *"start the first sprint"*)

**What:** in ScriviCore, the Range (`scrivi.range.v1`, a doubly linked list of scene entries — design v0.2 §4.2) and three
aggregate operations: `scrivi_range_read`, `scrivi_range_paste`, `scrivi_range_cut` (structural). Each makes one pass over
exactly the files in its range, returns the delta, and patches the project index in place. A two-scene paste into a
1,186-scene manuscript: ≤ 4 reads and ≤ 4 writes, guarded by count tests that fail if a walk returns.

---

*Last Updated: 2026-10-17 (**T-0614 filed → active** with SP-186 (EP-053 S1).)*

*Last Updated: 2026-10-17 (**T-0613 → 🟠 Implemented - Not Verified** → `Task-unverified.md`.)*

*Last Updated: 2026-10-17 (**T-0613 filed** in [SP-185] (planned, awaiting go-ahead).)*

*Last Updated: 2026-10-17 (**T-0612 → 🟠 Implemented - Not Verified** → `Task-unverified.md`.)*

*Last Updated: 2026-10-17 (**T-0612 filed → active** with SP-185's activation.)*

*Last Updated: 2026-10-15 (**T-0609 → 🟠 Implemented - Not Verified** → `Task-unverified.md`.)*

*Last Updated: 2026-10-15 (**T-0609 → active** with SP-182's activation.)*

*Last Updated: 2026-10-15 (**T-0608 → 🟠 Implemented - Not Verified** → `Task-unverified.md`.)*

*Last Updated: 2026-10-15 (**T-0608 → active** with SP-181's activation.)*

*Last Updated: 2026-10-13 (**T-0607 → 🟠 Implemented - Not Verified** → `Task-unverified.md`.)*

*Last Updated: 2026-10-13 (**T-0607 → active** with SP-180's activation.)*

*Last Updated: 2026-10-12 (**T-0606 → 🟠 Implemented - Not Verified** → `Task-unverified.md`.)*

*Last Updated: 2026-10-12 (**T-0606 → active** with SP-179's activation.)*
