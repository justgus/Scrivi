# Verified Tasks — T-0485 – T-0490 (SP-126, EP-035)

**Sprint:** SP-126 — `[Linux]` The three-tab Scene Inspector shell (✅ **CLOSED 2026-08-30**)
**Epic:** [EP-035](../../Epics/Epic-active.md) — `[Linux]` Object Foundations · **sprint 2**
**Verified:** 2026-08-30 — **user-approved after a live pass on the REAL RIG (build 8)**
**Closes:** **AC10**

---

## What these six Tasks delivered

✅ **The Scene Inspector's real three-tab shell** — **Writing · Worldbuilding · Properties**.
⚠️ **"Scene Entities" — Apple's DELETED SP-090 placeholder, copied by EP-024 and wired by SP-125 — is
finally retired.**

| ID | Title | Status |
| -- | ----- | ------ |
| **T-0485** | The three-tab shell, Apple's display order, defaulting to Writing | ✅ **Verified** |
| **T-0486** | ⚠️ **`inspector-layout.json` PATCHED, never reconstructed** | ✅ **Verified** |
| **T-0487** | Writing tab — tags / outline / todo | ✅ **Verified** |
| **T-0488** | Properties tab — ⚠️ **read-only IN FACT** | ✅ **Verified** |
| **T-0489** | Object list moved into Worldbuilding, unchanged | ✅ **Verified** |
| **T-0490** | ⚠️ **LIVE pass on the REAL RIG** | ✅ **Verified** |

---

## Evidence

- ✅ **571/571 `ctest` non-root + 19/19 Linux smokes**, natively on the rig at **Qt 6.10.2**
- ✅ **T-0486's round trip: 17/17 checks** — an Apple-written layout survives a Linux write with
  `stackSort`, `defaultStacks`, per-scene `scenes` ⚠️ **and a key this build has never heard of** all
  intact. ✅ **A CORRUPT file is left untouched** rather than overwritten.
- ✅ **No `ObjectKind` name hardcoded** — grep-proven.
- ✅ **The live pass ran on real projects** (`the-lone-golem.scrivi`, Apple-written layout).

---

## ⚠️ Three defects found by LOOKING — none by any suite

1. ⚠️ **Properties was UNREACHABLE** at the 200px default — Qt fell back to ~8px scroll arrows,
   ⚠️ **a gesture-only affordance in all but name.** ✅ Fixed by elision, then ✅ **the panel was WIDENED
   to 240/400 by user ruling**, window grown to 1220 so the manuscript did not pay for it.
2. ⚠️ **Outline and To-do were NEVER SAVED.** `QTextEdit` has no `editingFinished`, so only Tags
   committed. ⚠️ **My own header comment claimed a focus-out hook I had never written** — ⚠️ **a comment
   asserting behaviour that does not exist is worse than no comment.**
3. **I-0179** — the relationship label. ⚠️ **Three rounds; the first two fixed symptoms.**

## ⚠️ I-0179 in full, because the pattern matters

- **Round 1:** the message recovered a name by **string-surgery on the row text**.
  ⚠️ **A row's visible text is a PRESENTATION; parsing it back apart to recover data is the bug.**
- **Round 2:** the row itself read wrong — *"Myton at 23 — features"* implies Myton features something.
  ⚠️ **The label describes what the SCENE does**, projected by the core for the queried endpoint.
- ⚠️ **Round 3 — the user found the REAL defect: REDUNDANCY.** One word repeated down the whole list.
  ✅ **Hoisted to the group header:** `characters (2) (features)`.

⚠️ **My supporting analysis was also wrong and the user corrected it** — I counted `cites` edges, but
⚠️ **`cites` runs source→object and never touches a scene.**

## ✅ The dividend

**I-0180** — the same wrong label has shipped on **macOS since EP-031**, unnoticed.
✅ **Building the surface a SECOND TIME is what exposed it.**

---

*Verified 2026-08-30 by user approval. Archived in the same step SP-126 closed
(`feedback_archive_on_close`).*
