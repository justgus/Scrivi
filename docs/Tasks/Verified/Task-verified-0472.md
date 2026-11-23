# Verified Task — T-0472

**Sprint:** [SP-238] · **Epic:** [EP-059] · **Platform:** `[Linux]`
✅ **USER-VERIFIED 2026-11-23** (user, 2026-11-23: *"All steps pass on the Linux VM."*); closed (user, 2026-11-23: *"close SP-238"*)
**Archived:** 2026-11-23.

---

## ✅ T-0472 — `[Linux]` EP-059 C5: custom relationship types on Linux, as the Mac's

**Epic:** [EP-059] (C5) · **Status:** ✅ Verified (user, 2026-11-23: *"All steps pass on the Linux VM."*) · **Sprint:** [SP-238] (✅ Closed) →
[`../../Sprints/Closed/Sprint-SP-238.md`](../../Sprints/Closed/Sprint-SP-238.md) · **Date Requested:** 2026-08-25 (EP-034 close / SP-121's ABI gap audit)

**What:** the Mac's surface ([SP-237]) on Linux: Project Settings ▸ Relationship Types (built-in types marked Built in, Edit… on
the writer's own, New Relationship Type…), the form, and New Type… in Relate….

**Built:** `RelationTypes.cpp/.hpp` (new): `RelationTypeDialog`, `RelationTypesBox`; `ProjectSettingsDialog.cpp` (the box after
Authors); `ObjectDetailDialog::chooseRelation` (New Type…, the picker reopened with the new type chosen). Details and the live
pass: the Sprint.

<details><summary>Original backlog entry</summary>

**Epic:** **EP-059** C5 (EP-037 superseded 2026-11-20; ⚠️ **was EP-035 — moved by the 2026-08-25 three-way split**) · **Ruled:** 2026-08-25 (user)
**Origin:** EP-034 close / SP-121's ABI gap audit

⚠️ **`scrivi_upsert_relation_type` is fully implemented and has zero callers on ANY platform.** The core
writes custom relationship types into the project; `ProjectCreator` seeds the defaults
(`RelationTypeStore::seedTypes()`); `scrivi_list_relation_types` READS them and the app uses it.
⚠️ **Nothing can ADD one** — the write half has never been surfaced.

✅ **User ruling: this is a REAL FEATURE owed a UI, not speculative code.** Custom relationship vocabulary
is a legitimate worldbuilding capability — a DM wanting `sworn-enemy-of` or `liege-of` beyond the seeded
set.

⚠️ **Assigned to EP-037**, the Epic that builds Linux relationships — ⚠️ **it was EP-035 until the
2026-08-25 split moved relationship work into EP-037.** ⚠️ **It is BACKLOG, not scheduled.**

⚠️ **Also owed: a C ABI test.** `scrivi_upsert_relation_type` has **no ScriviCore test at the boundary**
(`feedback_boundary_tests_not_facade` — a facade test cannot see a boundary gap; that is how I-0113
shipped green).

</details>
