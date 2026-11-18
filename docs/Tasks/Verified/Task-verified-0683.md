# Verified Task — T-0683

**Sprint:** [SP-220] · **Epic:** [EP-054] · **Platform:** `[Cross]`
✅ **USER-VERIFIED 2026-11-17** (user, 2026-11-17: *"Live pass for mac passed all steps.  iPad passed all steps."*; *"Linux steps 2 and 3 passed, close SP-220"*)
**Archived:** 2026-11-17.

---

## ✅ T-0683 — `[Cross]` EP-054 S7: import a Scrivener project

**Epic:** [EP-054] (AC6, AC7, AC8) · **Status:** ✅ **Verified 2026-11-17** (user) · **Sprint:** [SP-220] (✅ Closed) →
[`../../Sprints/Closed/Sprint-SP-220.md`](../../Sprints/Closed/Sprint-SP-220.md) · **Date Requested:** 2026-11-17 (user: *"plan S7 Scrivener import"*)

**What:** `scrivi_import_manuscript` reads a Scrivener `.scriv` project (or its zipped backup) into the same Range and notes as
S6a, its chapters and scenes from the binder and each document's RTF through a new RTF reader; File ▸ Import ▸ Scrivener
Project… on the Mac, iPad and Linux. Rulings and ACs: the Sprint.

**Where:** `ScriviCore/src/manuscript/ManuscriptRtf.cpp` (+ `.hpp`, the RTF reader), `ManuscriptScrivener.cpp` (+ `.hpp`, the binder
walk, the folder and zip openers), `ManuscriptDocx` (`Segment` / `inlineMarkdown` shared), `ManuscriptImport` (`appendUtf8` /
`fromWindows1252` shared), the seed in `RangeOperations.cpp` and `FragmentPaster.cpp` (`firstPieceNamesTarget`),
`scrivi_import_manuscript` (`scrivi_c_api.cpp`, `scrivi.h`); Apple `presentImportManuscript(_:scrivener:)`, `scrivenerTypes`,
`ArchivePicker.scrivener`, `tellNotes` (`Scrivi/App/ProjectArchiveFlow.swift`); Linux `importManuscript(_, scrivener)`, the menu
item, `importNotes` (`platforms/linux/src/ScriviWindow.cpp`).
