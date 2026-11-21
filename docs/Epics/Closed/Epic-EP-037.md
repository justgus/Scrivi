# Epic EP-037 — ✅ CLOSED (superseded) 2026-11-20

**Closed:** ✅ **2026-11-20, SUPERSEDED by [EP-059]** (user, 2026-11-20: *"yes, file G1–G9 as an Epic (include everything here, so it is all tracked at once."*; asked how it relates to EP-035/036/037: *"One Epic for all"*).
Never activated. Every AC — verified and open — is carried into [EP-059] by reference; this file is the record of
what each AC meant and what it cost. ⚠️ The 2026-08-25 ruling that this work be THREE Epics (EP-035 §1) is superseded by
the same decision; EP-059 keeps the three as its parts A–C, in the same order.

---

## EP-037: `[Linux]` Relationships & Sources — ⚠️ **the third of three port Epics**

**Status:** 🔵 **Proposed** — created 2026-08-25 by the EP-035 three-way split.
**Codebase:** `[Linux]` — Qt/QML over `ScriviBridge`. ⚠️ **One ScriviCore-adjacent item: a C ABI test.**
**Goal:** The **relationship graph and citation apparatus** on Linux — related objects, relationship
creation, push-navigation, and sources with footnote text.
**Depends on:** ⚠️ **EP-036** — relationships and sources are sections OF the Detail Sheet.
**Date Created:** 2026-08-25 · **Target Close:** — (⚠️ **3–4 sprints, estimated before implementation**)

⚠️ **Apple equivalent: ~1,500 lines** (`ObjectSourcesSection` 604, `ObjectRelationsSection` 438,
`ObjectRelationPicker` 259, `ObjectDetailHistory` 100, `Citation` 97).

### Acceptance Criteria (draft — ⚠️ to be ruled at promotion)

- **AC1** — The related-objects section lists this object's edges ⚠️ **with labels reading correctly FROM
  THIS ENDPOINT.** ⚠️ **`list_edges_for` returns the label ALREADY RESOLVED — pass it through UNTOUCHED.**
  ⚠️ **Recomputing direction in Qt is the same defect class as restating kind scope**, which this project
  has paid for eight times.
- **AC2** — A relationship can be **created from the sheet**; ⚠️ **it appears from BOTH endpoints** and is
  **rejected as a duplicate from the second.**
- **AC3** — **Push-navigation to a related object, with back and forward.** ⚠️ **On Apple this needed FOUR
  Issues** (I-0151, I-0153, I-0157, I-0161) before it worked. ⚠️ **Verify both halves SEPARATELY** —
  back/forward and push-from-the-related-list are different mechanisms
  (`feedback_verify_each_half_separately`).
- **AC4** — **Sources can be created from the object they document**, with citation detail and footnote
  text. ⚠️ **Creation lives on the OBJECT's surface, never on the scene-scoped `sources` card** — user
  ruling R6: *"it would muddy the fact that the source must be associated with an object in the world."*
- **AC5** — ⚠️ **T-0472: a surface for CUSTOM RELATION TYPES**, calling `scrivi_upsert_relation_type`.
  ⚠️ **That endpoint is fully implemented and has NEVER had a caller on any platform** — projects are
  seeded with defaults and `list_relation_types` reads them, but ⚠️ **nothing can ADD one.**
  ✅ **User-ruled a REAL FEATURE owed a UI** (2026-08-25).
- **AC6** — ⚠️ **A C ABI test for `scrivi_upsert_relation_type`**, which ⚠️ **has NONE.**
  ⚠️ **Through the boundary, not the facade** — `feedback_boundary_tests_not_facade`; a facade test
  cannot see a boundary gap, and that is how I-0113 shipped green.
- **AC7** *(standing)* — ⚠️ **Porting Outline corrected from experience.**
- **AC8** *(standing)* — ⚠️ **Platform-independent rules re-honoured, not re-decided.**
- **AC9** *(standing)* — `ctest` **NON-ROOT, tests ON**, actually run.
- **AC10** *(standing)* — ⚠️ **A LIVE VNC click-through per surface-shipping sprint.**

### ⚠️ At this Epic's close, the port is DONE — and so is the debt

✅ **When EP-037 closes, SP-121's 47 bridged endpoints finally have readers** and the
`capability_without_surface` risk EP-034 named at its own close is discharged. ⚠️ **Until then it stands.**

---
