# Verified Task — T-0632

**Sprint:** [SP-198] · **Epic:** [EP-055] · **Platform:** `[Apple]`
✅ **USER-VERIFIED 2026-11-03** (iPad A1–A4, A13 and iPhone B1–B7 (user, 2026-11-03), iPhone Duo simulator C1–C4 (user, 2026-11-03: *"all pass!"*))
**Archived:** 2026-11-03, on the user's reported pass.

---

## T-0632 — `[Apple]` EP-055 S1: the manuscript host gap audit — what is AppKit, what is shared, the iOS shape

**Epic:** [EP-055] (AC1) · **Status:** ✅ **Verified 2026-11-03** (iPad A1–A4, A13 and iPhone B1–B7 (user, 2026-11-03), iPhone Duo simulator C1–C4 (user, 2026-11-03: *"all pass!"*)) (2026-10-30) — ⏳ the live passes ([SP-198] AC5) · **Sprint:** [SP-198] (🟡 Active) → [`Sprint-SP-198.md`](../../Sprints/Closed/Sprint-SP-198.md) · **Date Requested:** 2026-10-29 (user: *"we should focus on the Manuscript View first"*)

**What:** every behaviour in `ManuscriptTextView.swift` (the representable `:37`, the `Coordinator` `:287`, `ManuscriptNSTextView`
`:2614`, `SceneBoundaryTable` `:3537`) and in `ManuscriptCommands` / `ManuscriptFind` / `ManuscriptAccessibility`, each given one
disposition: **shared as is**, **shared after a font/colour swap**, **moved out of the host into shared code**, or
**re-implemented on UIKit** (and why). Ordered by what S1, S2 and S3 need. Answers [SP-198] Q1 for the user to rule.
**Output:** `docs/Scrivi_iOS_Manuscript_Design_v0_1.md`.
**Done (2026-10-30):** `docs/Scrivi_iOS_Manuscript_Design_v0_1.md`. Finding: the bytes a keystroke stores were decided in
`ManuscriptNSTextView`'s AppKit overrides. ✅ Q1 = (c) and undo off in S1, both ruled by the user 2026-10-30.
