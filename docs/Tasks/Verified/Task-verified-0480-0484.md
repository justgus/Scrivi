# Verified Tasks — T-0480 – T-0484 (SP-125, EP-035)

**Sprint:** SP-125 — `[Linux]` The Scene Inspector, wired to real objects
**Epic:** [EP-035](../../Epics/Epic-active.md) — `[Linux]` Object Foundations (sprint 1)
**Verified:** 2026-08-28 — **user-approved after a live click-through**
**Codebase:** `[Linux]` — Qt **Widgets**

---

## What these five Tasks delivered

✅ **The FIRST object surface the Linux app has ever had.** `SceneInspector` had been a **67-line stub**
since EP-024/SP-078 — *"wired to NO project data… a muted 'No entities yet.' empty state, and a DISABLED
'Add Entity' button."* It now lists the scene's real objects, grouped by kind.

| ID | Title | Status |
| -- | ----- | ------ |
| **T-0480** | Kind scope **DERIVED** from `listObjectKinds` — never restated in Qt | ✅ **Verified** |
| **T-0481** | Scene Entities tab lists the scene's objects, grouped by kind | ✅ **Verified** |
| **T-0482** | **Double-click AND context-menu** both open an object | ✅ **Verified** |
| **T-0483** | Empty / unavailable / pending are **DISTINCT and EXPLAINED** | ✅ **Verified** |
| **T-0484** | **LIVE pass** (EP-035 AC9) + `ctest` non-root + Epic wording corrected | ✅ **Verified** |

---

## Evidence

- ✅ **`ctest` 571/571, NON-ROOT (`builder`), tests ON** — the expected figure exactly.
  ⚠️ **Required fixing I-0171 first: it recurred and blocked the run outright.**
- ✅ **All 18 Linux smokes pass**, including the new 23-check `scene_inspector_smoke`.
- ✅ **T-0480's grep proof is clean** — no `ObjectKind` name appears anywhere in `platforms/linux/`.
  ⚠️ **A restatement here would have been OCCURRENCE NINE** of the project's most-repeated defect;
  occurrence five was in Swift, so a new platform layer is exactly where it recurs.
- ✅ **T-0482's two halves verified SEPARATELY** (`feedback_verify_each_half_separately`): double-click
  opened a **world-scoped** `character`, the context menu a **project-scoped** `source`.
  ✅ **That pairing also proves `worldID` threads in BOTH directions** — the SP-104 failure mode.
- ✅ **The user's live click-through succeeded**, confirming the *"opened successfully"* dialog against
  **real writing work** (`the-lone-golem.scrivi` + `the-stairs-of-tintagael.scrivi`, both bound to the
  shared **Eskandar** world).
- ✅ **T-0483's hardest state was proven on REAL data, not the fixture**: with the world mounted at a
  non-matching path the panel reported *"World 'Eskandar' is unavailable. These 5 links are held
  pending."* — named, explained, and never implying the links were lost.

---

## Issues raised against these Tasks

| ID | Outcome |
| -- | ------- |
| **I-0173** | ⚠️ **Found by the LIVE PASS** — rows elided the relationship label. ✅ Fixed in the same sprint (tooltips). ⚠️ **All 571 ctests and 23 smoke checks were green with it present**, because it is a defect in what a writer can READ. |
| **I-0174** | ⚠️ **My diagnosis was WRONG and the user corrected it.** ✅ Settled — see below. |
| **I-0175** | ⚠️ **My own process defect**, not the app's: a synthetic-input driver typed a path into the user's manuscript. ✅ Repaired byte-for-byte. |

---

## ⚠️ The correction worth carrying forward

⚠️ **I reported that opening a project performs an unexplained write.** ✅ **It does not, and it is not a
risk.** The two files that change are a `generation` counter and a world `cachedIndex` reconcile —
⚠️ **and the "new" characters (*Pet*, *Myton at 23*, *Administrator Rusk*) were authored in
`the-lone-golem.scrivi`, a SECOND project bound to the SAME Eskandar world.**

✅ **That is shared-world propagation working exactly as designed.** No prose is touched.
⚠️ **The lesson: a surprising write into a SHARED resource should send you looking for the other sharer
before calling it a defect.**

---

*Verified 2026-08-28 by user approval following a live click-through. Archived in the same step
(`feedback_archive_on_close`).*
