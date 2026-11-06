# Verified Task — T-0611

**Sprint:** [SP-184] · **Epic:** [EP-048] · **Platform:** `[Cross]`
✅ **USER-VERIFIED 2026-11-06** (user, the live pass on scrivi-vm and the Mac: *"All tests pass."*; Q7: *"60% on both platforms is correct."*)
**Archived:** 2026-11-06.

---

## ✅ T-0611 — `[Cross]` EP-048 S5c: Show chapter titles (on by default), and the chapter-ending divider

**Epic:** [EP-048] (S5c) · **Status:** ✅ **Verified 2026-11-06** (user) · **Sprint:** [SP-184] (✅ Closed 2026-11-06) →
[`../../Sprints/Closed/Sprint-SP-184.md`](../../Sprints/Closed/Sprint-SP-184.md) · **Date Requested:** 2026-10-15 (user: *"plan EP-048 fully."*)

**What:** the dialog's Writing Surface ▸ Show chapter titles in manuscript ([EP-026] Q4), read from the shared settings file
(absent per [SP-184] Q1). `SceneDocument` builds with or without chapter headings, and every chapter boundary gets Apple's
full-strength closing divider ([T-0575]). Every structural operation and its undo is tested with titles off and on.
✅ **2026-11-05 ([SP-184] rulings):** ⛔ Q1 re-ruled — titles **ON by default on the Mac, iOS and Linux** (`[Apple]`:
`ProjectPreferences.swift:123` `?? false` → on, per Q5); the divider in `QPalette::Highlight` at 60% (Q3); Orca hears *"End of
chapter"* at a chapter end (Q4, Q6: the Mac's string). Now `[Cross]`.

**Done (2026-11-05):** [SP-184] as built: one boundary builder with the titles flag (`platforms/linux/src/SceneDocument.cpp:37`), the
toggle (`:768`), `dividers()` (`:757`), the chapter-end colour (`ThemeColours.hpp:91`), *"End of chapter"* on the presented page
(`ManuscriptFind.cpp:89`), Writing Surface ▸ Show chapter titles in manuscript, `EditorShell::applyChapterTitles`; `[Apple]`
`ProjectPreferences.swift` absent = ON and the key written only once chosen. Tested by `tests/chapter_titles_smoke.*` (in CI, 7/7
mutations caught) and `structural_baseline_smoke`'s second run with titles off.
