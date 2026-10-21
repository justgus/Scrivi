# Verified Task — T-0614

**Sprint:** [SP-186] · **Epic:** [EP-053] (S1) · **Platform:** `[ScriviCore]`
**Filed:** 2026-10-17 · **Implemented:** 2026-10-20 · ✅ **USER-VERIFIED 2026-10-20** (user: *"mark T-0614 as verified"*) — by review; no app calls the endpoints until [SP-187], so there was no live pass.
**Archived:** 2026-10-20, with the Sprint close (*"close SP-186"*).

---

## T-0614 — `[ScriviCore]` EP-053 S1: the Range and the aggregate range operations

**Epic:** [EP-053] (S1) · **Status:** ✅ **Verified 2026-10-20** (user: *"mark T-0614 as verified"*) — archived with the Sprint close · **Sprint:** ✅ [SP-186] →
[`../../Sprints/Closed/Sprint-SP-186.md`](../../Sprints/Closed/Sprint-SP-186.md) · **Date Requested:** 2026-10-17 (user: *"start the first sprint"*)

**What:** in ScriviCore, the Range (`scrivi.range.v1`, design v0.2 §4.2) and three aggregate operations — `scrivi_range_read`,
`scrivi_range_paste`, `scrivi_range_cut` (structural) — each making one pass over exactly its files, returning the delta and
patching the project index in place. ✅ A two-scene paste: 3 reads + 5 writes, no listing (re-ruled 2026-10-20); a two-scene
cut: 4 reads + 5 writes. ✅ A cut Range pasted back restores scene and chapter IDs (ruled 2026-10-20).
**Where:** `ScriviCore/src/manuscript/RangeOperations.hpp/.cpp`, `ProjectIndex.hpp/.cpp` (in-place patches),
`scrivi_c_api.cpp` + `scrivi.h` (the three endpoints); tests `RangeOperationBudgetTests.cpp`, `RangeOperationCApiTests.cpp`.
