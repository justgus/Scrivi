# Verified Task — T-0673

**Sprint:** [SP-212] · **Epic:** [EP-054] · **Platform:** `[ScriviCore]`
✅ **USER-VERIFIED 2026-11-12** (user, 2026-11-12: *"For AC5 I ran the same project, which has two authors. a-f passed."*)
**Archived:** 2026-11-12.

---

## ✅ T-0673 — `[ScriviCore]` EP-054 AC9: the byline on every export

**Epic:** [EP-054] (AC9) · **Status:** ✅ **Verified 2026-11-12** (user) · **Sprint:** [SP-212] (✅ Closed) →
[`../../Sprints/Closed/Sprint-SP-212.md`](../../Sprints/Closed/Sprint-SP-212.md) · **Date Requested:** 2026-11-12 (user: *"Next is EP-054 AC9."*)

**What:** `scrivi_export_manuscript` reads the project's ordered, named authors (`AuthorStore::read`) and every export
carries them: "by A, B and C" under the title and subtitle in Markdown, HTML and `.docx` (an Author style), and the authors
in HTML `<meta name="author">` and `.docx` `dc:creator` — never a retired or unnamed author ([Authorship design] Q6,
[EP-058] AC4). The rest is SP-212's Q1–Q4 (✅ ruled as recommended).

**Built (2026-11-12):** `ExportInput.authors` (`ScriviCore/src/manuscript/ManuscriptExport.hpp`); `scrivi_export_manuscript`
reads them (`scrivi_c_api.cpp`); `byline` / `creators`, the Author style and `dc:creator` (`ManuscriptExport.cpp`).
`ManuscriptExportCApiTests`: 3 new cases, the fixture's author unnamed so the existing goldens prove AC3. macOS ctest 837/837.
Linux ctest 842/842 (scrivi-vm); the same `.docx` SHA-256 from the Mac and Linux, whole manuscript and one chapter (SP-212 AC4).
**Verified (2026-11-12):** the user's live passes on the Mac and Linux, read in Pages and LibreOffice (SP-212 AC5).
