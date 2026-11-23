# Verified Task — T-0700

**Sprint:** [SP-237] · **Epic:** [EP-059] · **Platform:** `[Apple]`
✅ **USER-VERIFIED 2026-11-23** (user, 2026-11-23: *"all the tests pass on iOS as well (both iPad and iPhone)"*; closed (user, 2026-11-23: *"Close SP-237."*))
**Archived:** 2026-11-23.

---

## ✅ T-0700 — `[Apple]` EP-059 C5/C6: custom relationship types on the Mac and iOS; the upsert endpoint's C ABI test and fixes

**Epic:** [EP-059] (C5, C6) · **Status:** ✅ Verified (user, 2026-11-23: live pass on the Mac, iPad and iPhone) · **Sprint:** [SP-237] (✅ Closed) →
[`../../Sprints/Closed/Sprint-SP-237.md`](../../Sprints/Closed/Sprint-SP-237.md) · **Date Requested:** 2026-11-22 (user, 2026-11-22: *"plan C5-C6"*)

**What:** a surface for `scrivi_upsert_relation_type`, which no platform had: Project Settings ▸ Relationship Types and New Type… in
Relate…, on the Mac and iOS; the endpoint's C ABI test; the endpoint made to refuse what it saved silently. Findings, questions and ACs: the Sprint.

**Built:** core — `scrivi_upsert_relation_type` refuses unknown kinds and `world`, an empty code is a new unique type;
`RelationTypeStore::upsert` refuses seeded codes and shape changes to used types; the list reports `builtIn`, `inUse`;
`objectKindFromName` derived. Swift — `ScriviEngine.upsertRelationType`, `RelationTypeForm.swift` (the form and the settings section),
`ProjectSettingsSheet.swift`, `ObjectRelationPicker.swift`. Details and the live pass: the Sprint.

