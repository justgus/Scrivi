# Verified Task — T-0682

**Sprint:** [SP-219] · **Epic:** [EP-054] · **Platform:** `[Cross]`
✅ **USER-VERIFIED 2026-11-17** (user, 2026-11-17: *"I ran all the Mac, iPad, and Linux Live Pass tests and they all passed."*)
**Archived:** 2026-11-17.

---

## ✅ T-0682 — `[Cross]` EP-054 S6b: import a manuscript from a `.docx`

**Epic:** [EP-054] (AC6, AC7, AC8) · **Status:** ✅ **Verified 2026-11-17** (user) · **Sprint:** [SP-219] (✅ Closed) →
[`../../Sprints/Closed/Sprint-SP-219.md`](../../Sprints/Closed/Sprint-SP-219.md) · **Date Requested:** 2026-11-15 (user: *"close SP-218 then plan S6b."*)

**What:** `scrivi_import_manuscript` reads a Word document (Word, Pages, Google Docs, Scrivi's own export) into the same Range and
notes as S6a, by way of Markdown; File ▸ Import ▸ Manuscript… offers `.docx` on the Mac, iPad and Linux. Rulings and ACs: the Sprint.

**Where:** `ScriviCore/src/manuscript/ManuscriptDocx.cpp` (+ `.hpp`), `scrivi_import_manuscript` (`scrivi_c_api.cpp`, `scrivi.h`),
`ManuscriptImport.cpp` (`reportMarkup`); Apple `manuscriptTypes` and `tellNotes` (`Scrivi/App/ProjectArchiveFlow.swift`); Linux
`importManuscript` filter and `importNotes` (`platforms/linux/src/ScriviWindow.cpp`). Evidence: 11 `[docx]` cases; ctest 866/866;
interop 351/351; scrivi-vm ctest 871/871; Linux smokes; CI green; the live passes on the Mac, iPad and Linux (2026-11-17).
