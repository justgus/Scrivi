# Verified Task — T-0675

**Sprint:** [SP-214] · **Epic:** [EP-054] · **Platform:** `[Cross]`
✅ **USER-VERIFIED 2026-11-13** (user, 2026-11-13: *"Linux steps 1-6 pass. 2148 pages for the full manuscript, 31 pages for Chapter 2."*)
**Archived:** 2026-11-13.

---

## ✅ T-0675 — `[Cross]` EP-054 S5b: PDF export

**Epic:** [EP-054] (S5b; AC5, AC9) · **Status:** ✅ **Verified 2026-11-13** (user) · **Sprint:** [SP-214] (✅ Closed 2026-11-13) →
[`../../Sprints/Closed/Sprint-SP-214.md`](../../Sprints/Closed/Sprint-SP-214.md) · **Date Requested:** 2026-11-12 (user: *"plan EP-054 S5b."*)

**What:** Export Manuscript… ▸ **PDF** on the Mac, iPad and Linux — the whole manuscript or the caret's chapter, in the writer's
typeface (embedded), on the default paper, with a title page and byline, each chapter on a new page, scene breaks, emphasis and
page numbers. The core writes a print page (`scrivi_export_manuscript`, `document: "print"`); WebKit paginates it on Apple,
`QTextDocument` + `QPdfWriter` on Linux. Rulings and ACs: the Sprint.

**Built (2026-11-12):** `exportPrint` (`ScriviCore/src/manuscript/ManuscriptExport.cpp:735`); `document: "print"`
(`scrivi_c_api.cpp:1362`, `scrivi.h`); `Scrivi/App/ManuscriptPDF.swift` (WebKit; Mac and iPad);
`ProjectArchiveFlow.swift:140`; `ScriviEngine.exportManuscript(… printPage:)` (+ stub); Linux
`platforms/linux/src/ManuscriptPdf.hpp`, `EditorShell::exportManuscript`, `settings_smoke` §5. Evidence: macOS ctest
842/842, interop 2/2, iOS build; Linux ctest 847/847, settings and bridge smokes green; `pdffonts` / `pdftotext` on a real
project. **Linux fix (2026-11-13, the first Linux pass failed):** text drawn white on a dark desktop, and ~80% of a long manuscript never laid out — `ManuscriptPdf.hpp` sets the page size after the heading fix and draws with black text; `settings_smoke` §5 guards both. **Verified (2026-11-13):** the user's live passes — Preview (Mac), Files (iPad), Papers (Linux: 2148 pages whole, 31 for Chapter 2), whole manuscript and one chapter (SP-214 AC2, AC5).
