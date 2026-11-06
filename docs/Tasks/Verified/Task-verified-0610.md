# Verified Task — T-0610

**Sprint:** [SP-183] · **Epic:** [EP-048] · **Platform:** `[Linux]`
✅ **USER-VERIFIED 2026-11-05** (user, the live pass on scrivi-vm: *"all three re-test steps passed."* · *"C1, C2, C3, and C6 all passed."*)
**Archived:** 2026-11-05.

---

## ✅ T-0610 — `[Linux]` EP-048 S5b: line spacing and the first-line indent

**Epic:** [EP-048] (S5b, L10) · **Status:** ✅ **Verified 2026-11-05** (user) · **Sprint:** [SP-183] (🟡 Active 2026-11-05) →
[`../../Sprints/Closed/Sprint-SP-183.md`](../../Sprints/Closed/Sprint-SP-183.md) · **Date Requested:** 2026-10-15 (user: *"plan EP-048 fully."*)

**What:** line spacing fixed at 1.45 × size, and EP-047's first-line indent on Linux (None · Every · Book convention, the
amount in ems, the small gap), with zero characters in the file and no dirty scene or history step. Both are Qt block
formats, so the mechanism is measured first and ruled from the numbers ([SP-183] Q1–Q2). Adds the dialog's indent rows.

**Done (2026-11-05):** [SP-183] Q1 = (B): `ManuscriptLayout` (`platforms/linux/src/ManuscriptLayout.cpp:7`), a
`QPlainTextDocumentLayout` that re-lays each block Qt has laid out, with the first line indented and every line on a fixed
pitch; what each line gets is decided by the presenter while it highlights (`ManuscriptPresenter.cpp:343` `lineStyle`, Apple's
`isBody` and book rule at `:328`) and left on the block as user data, never a document edit; the next paragraph is redrawn
after an edit to the one before (`:511`). The setting is Apple's `paragraphIndent` / `indentEm` in `project-settings.json`
(`ProjectPreferences`); the dialog's Paragraph indent and Indent rows (`ProjectSettingsDialog.cpp:75`). Tested by
`tests/indent_smoke.*` (in CI), 5 of 5 mutations caught. With it, [I-0351] fixed.
