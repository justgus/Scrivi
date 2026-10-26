# Active Tasks

---

## 🟡 T-0623 — `[ScriviCore]` EP-052 S1: two locks per project (memory and disk)

**Epic:** [EP-052] (AC0) · **Status:** 🟡 **Active** (2026-10-26) · **Sprint:** [SP-194] (🟡 Active) →
[`../Sprints/Sprint-SP-194.md`](../Sprints/Sprint-SP-194.md) · **Date Requested:** 2026-10-26 (user: *"Q1–Q7 agreed as recommended"*)

**What:** replace SP-173's single exclusive project lock (`ScriviCore/src/public_api/ProjectLock.hpp`) with a memory lock (M)
and a disk lock (D), per v0.2 §3: D before M, never M before D; the revision moves under M. Each of the 126 guarded
endpoints names its kind, and `scripts/check-abi-project-guards.sh` enforces it. A memory-only endpoint that makes a file
operation fails a test. SP-173's concurrency tests move to the new form.

---

## 🟡 T-0624 — `[ScriviCore]` EP-052 S1: the per-project write queue and its endpoints

**Epic:** [EP-052] (AC1–AC3) · **Status:** 🟡 **Active** (2026-10-26) · **Sprint:** [SP-194] (🟡 Active) →
[`../Sprints/Sprint-SP-194.md`](../Sprints/Sprint-SP-194.md) · **Date Requested:** 2026-10-26 (user: *"Q1–Q7 agreed as recommended"*)

**What:** v0.1 W1–W4 and W7 under D: one writer thread per open project, FIFO, coalescing; enqueue endpoints for scene saves
and structural operations with a ticket; history appends in the same queue; `scrivi_wait_for_writes` reports every
outcome; synchronous file endpoints wait for earlier jobs (W3); `scrivi_close_project` drains. Boundary tests through
`scrivi_*` and a ThreadSanitizer stress run on macOS and Linux. A structural job's delta comes back inside its outcome in `scrivi_wait_for_writes` ([SP-194] Q1 (a), ruled 2026-10-26).

---

*Last Updated: 2026-10-26 (**T-0623, T-0624 → active** with SP-194's activation (EP-052 S1).)*

*Last Updated: 2026-10-24 (**T-0622 → 🟠 Implemented - Not Verified** → `Task-unverified.md`.)*

*Last Updated: 2026-10-24 (**T-0622 → active** with SP-193's activation (EP-054 S0).)*

*Last Updated: 2026-10-24 (**T-0620 → 🟠 Implemented - Not Verified** → `Task-unverified.md`.)*

*Last Updated: 2026-10-24 (**T-0620 → active** with SP-192's activation.)*


*Last Updated: 2026-10-22 (**T-0616 → 🟠 Implemented - Not Verified** → `Task-unverified.md`.)*

*Last Updated: 2026-10-22 (**T-0616 → active** with SP-188's activation (EP-053 S3).)*

*Last Updated: 2026-10-21 (**T-0619 → 🟠 Implemented - Not Verified** → `Task-unverified.md`.)*

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
