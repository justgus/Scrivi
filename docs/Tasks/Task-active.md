# Active Tasks

---

## 🟡 T-0619 — `[Cross]` EP-053 S2b: copy, paste and its undo, split and merge — as deltas, off the main thread, no reload

**Epic:** [EP-053] (S2b) · **Status:** 🟡 **Active** (2026-10-21) · **Sprint:** [SP-191] (🟡 Active) →
[`../Sprints/Sprint-SP-191.md`](../Sprints/Sprint-SP-191.md) · **Date Requested:** 2026-10-20 (from [SP-187]'s Q1 ruling: split S2)

**What:** on the Mac and on Linux, copy builds the Range from the in-memory list; paste, paste's undo (a range cut) and redo,
split, chapter split, merge and chapter merge each make one range call (`rangePaste` / `rangeCut`) off the main thread, and
the editor applies the returned delta with no reload. Each undoes in one step, with no barriers; the Range carries scene and
chapter sidecars so an undo restores them; a scene merge is a range cut of the break and a chapter merge is
a range relabel (`scrivi_range_relabel`, which moves the scene files), and `scrivi_merge_scene` / `scrivi_merge_chapter` retire. The clipboard and copy buffers store Ranges (an old fragment reads as empty). Old
`structuredPaste` / `structuredCut` history nodes convert to the same path. Absorbs [SP-185] AC4.

---

*Last Updated: 2026-10-21 (**T-0619 → active** with SP-191's activation (EP-053 S2b).)*

*Last Updated: 2026-10-20 (**T-0615 → 🟠 Implemented - Not Verified** → `Task-unverified.md`.)*

*Last Updated: 2026-10-20 (**T-0615 → active** with SP-187's activation (EP-053 S2a).)*

*Last Updated: 2026-10-20 (**T-0614 → 🟠 Implemented - Not Verified** → `Task-unverified.md`.)*

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
